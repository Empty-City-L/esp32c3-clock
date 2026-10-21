#include "photo.h"

#include <Preferences.h>
#include <SPIFFS.h>
#include <string.h>

namespace {

constexpr uint32_t kMagic = 0x50484333UL;  // "PHC3"
constexpr uint16_t kVersion = 1;
constexpr const char *kAnimPath = "/photo.anim";

struct __attribute__((packed)) AnimHeader {
  char magic[4];
  uint8_t version;
  uint8_t frames;
  uint16_t delayMs;
  uint32_t reserved;
};
static_assert(sizeof(AnimHeader) == PHOTO_HEADER_BYTES, "animation header");

struct PhotoConfig {
  uint32_t magic;
  uint16_t version;
  uint8_t pageSec;
};

Preferences prefs;
PhotoConfig gCfg;
PhotoStyle gStyle;
uint8_t gFrames = 0;
uint16_t gDelay = 0;
bool gValid = false;

void defaults(PhotoConfig &c) {
  memset(&c, 0, sizeof(c));
  c.magic = kMagic;
  c.version = kVersion;
  c.pageSec = 15;
}

void sanitize(PhotoConfig &c) {
  if (c.pageSec < 5 || c.pageSec > 120) c.pageSec = 15;
}

void defaultsStyle(PhotoStyle &s) {
  memset(&s, 0, sizeof(s));
  s.timeOn = true;
  s.timeStyle = 0;
  s.timeColor = 0xF2F7FF;
  s.colonStyle = 1;
  s.gradOn = true;
  s.gradColor = 0x000000;
  s.gradOpa = 85;
  s.gradHeight = 72;
}

void sanitizeStyle(PhotoStyle &s) {
  if (s.timeStyle > 2) s.timeStyle = 0;
  if (s.colonStyle > 3) s.colonStyle = 1;
  if (s.gradOpa > 100) s.gradOpa = 100;
  if (s.gradHeight != 48 && s.gradHeight != 56 && s.gradHeight != 72) {
    s.gradHeight = 56;
  }
  s.timeColor &= 0xFFFFFF;
  s.gradColor &= 0xFFFFFF;
}

bool saveConfig() {
  return prefs.putBytes("cfg", &gCfg, sizeof(gCfg)) == sizeof(gCfg);
}

bool headerOk(const AnimHeader &h) {
  return memcmp(h.magic, "ANIM", 4) == 0 && h.version == 1 && h.frames >= 1 &&
         h.frames <= PHOTO_MAX_FRAMES && h.delayMs <= 5000;
}

void refresh() {
  gFrames = 0;
  gDelay = 0;
  gValid = false;

  File f = SPIFFS.open(kAnimPath, "r");
  if (!f) return;
  AnimHeader h;
  if (f.read(reinterpret_cast<uint8_t *>(&h), sizeof(h)) != sizeof(h) || !headerOk(h)) {
    f.close();
    return;
  }
  const size_t expected = PHOTO_HEADER_BYTES + (size_t)h.frames * PHOTO_FRAME_BYTES;
  if (f.size() != expected) {
    f.close();
    return;
  }
  f.close();
  gFrames = h.frames;
  gDelay = h.delayMs < 20 ? 100 : h.delayMs;
  gValid = true;
}

void migrateLegacy() {
  // 旧单图：/photo0.raw -> /photo.anim（1 帧）
  if (!SPIFFS.exists(kAnimPath) && SPIFFS.exists("/photo0.raw")) {
    File raw = SPIFFS.open("/photo0.raw", "r");
    if (raw && raw.size() == PHOTO_FRAME_BYTES) {
      File out = SPIFFS.open(kAnimPath, "w");
      if (out) {
        AnimHeader h = {};
        memcpy(h.magic, "ANIM", 4);
        h.version = 1;
        h.frames = 1;
        h.delayMs = 0;
        out.write(reinterpret_cast<const uint8_t *>(&h), sizeof(h));
        uint8_t buf[512];
        while (raw.available()) {
          const size_t n = raw.read(buf, sizeof(buf));
          if (n == 0) break;
          out.write(buf, n);
        }
        out.close();
      }
    }
    if (raw) raw.close();
    SPIFFS.remove("/photo0.raw");
  }

  // 清理更早版本的 JPEG 文件
  for (uint8_t i = 0; i < 8; ++i) {
    char legacy[20];
    snprintf(legacy, sizeof(legacy), "/photo%u.jpg", (unsigned)i);
    if (SPIFFS.exists(legacy)) SPIFFS.remove(legacy);
  }
}

}  // namespace

void photoBegin() {
  prefs.begin("photo", false);
  PhotoConfig c;
  size_t n = prefs.getBytes("cfg", &c, sizeof(c));
  if (n != sizeof(c) || c.magic != kMagic || c.version != kVersion) {
    defaults(gCfg);
    saveConfig();
  } else {
    gCfg = c;
  }
  sanitize(gCfg);

  defaultsStyle(gStyle);
  gStyle.timeOn = prefs.getBool("t_on", gStyle.timeOn);
  gStyle.timeStyle = prefs.getUChar("t_style", gStyle.timeStyle);
  gStyle.timeColor = prefs.getUInt("t_color", gStyle.timeColor);
  gStyle.colonStyle = prefs.getUChar("colon", gStyle.colonStyle);
  gStyle.gradOn = prefs.getBool("g_on", gStyle.gradOn);
  gStyle.gradColor = prefs.getUInt("g_color", gStyle.gradColor);
  gStyle.gradOpa = prefs.getUChar("g_opa", gStyle.gradOpa);
  gStyle.gradHeight = prefs.getUChar("g_h", gStyle.gradHeight);
  sanitizeStyle(gStyle);

  if (!SPIFFS.begin(true)) {
    Serial.println("[photo] SPIFFS mount failed");
  } else {
    Serial.printf("[photo] SPIFFS %u/%u KB\n", (unsigned)(SPIFFS.usedBytes() / 1024),
                  (unsigned)(SPIFFS.totalBytes() / 1024));
  }
  migrateLegacy();
  refresh();
  Serial.printf("[photo] %u frames, %u ms, page=%us\n", gFrames, gDelay, gCfg.pageSec);
}

void photoRefresh() { refresh(); }

uint8_t photoCount() { return gValid ? 1 : 0; }
uint8_t photoFrameCount() { return gFrames; }
uint16_t photoDelayMs() { return gDelay; }
bool photoExists() { return gValid; }

bool photoReadFrame(uint8_t index, uint16_t *dst, size_t pixels) {
  if (!gValid || !dst || index >= gFrames || pixels != PHOTO_W * PHOTO_H) return false;
  File f = SPIFFS.open(kAnimPath, "r");
  if (!f) return false;
  const size_t offset = PHOTO_HEADER_BYTES + (size_t)index * PHOTO_FRAME_BYTES;
  if (!f.seek(offset)) {
    f.close();
    return false;
  }
  const bool ok = f.read(reinterpret_cast<uint8_t *>(dst), PHOTO_FRAME_BYTES) == PHOTO_FRAME_BYTES;
  f.close();
  return ok;
}

bool photoValidatePath(const char *path) {
  if (!path) return false;
  File f = SPIFFS.open(path, "r");
  if (!f) return false;
  AnimHeader h;
  bool ok = f.read(reinterpret_cast<uint8_t *>(&h), sizeof(h)) == sizeof(h) && headerOk(h);
  if (ok) {
    const size_t expected = PHOTO_HEADER_BYTES + (size_t)h.frames * PHOTO_FRAME_BYTES;
    ok = f.size() == expected;
  }
  f.close();
  return ok;
}

bool photoValidate() {
  if (!photoValidatePath(kAnimPath)) SPIFFS.remove(kAnimPath);
  refresh();
  return gValid;
}

bool photoDelete() {
  const bool ok = SPIFFS.remove(kAnimPath);
  refresh();
  return ok || !gValid;
}

size_t photoSize() {
  File f = SPIFFS.open(kAnimPath, "r");
  if (!f) return 0;
  const size_t n = f.size();
  f.close();
  return n;
}

const PhotoStyle &photoStyle() { return gStyle; }

bool photoSetStyle(const PhotoStyle &style) {
  gStyle = style;
  sanitizeStyle(gStyle);
  bool ok = true;
  ok &= prefs.putBool("t_on", gStyle.timeOn) == 1;
  ok &= prefs.putUChar("t_style", gStyle.timeStyle) == 1;
  ok &= prefs.putUInt("t_color", gStyle.timeColor) == 4;
  ok &= prefs.putUChar("colon", gStyle.colonStyle) == 1;
  ok &= prefs.putBool("g_on", gStyle.gradOn) == 1;
  ok &= prefs.putUInt("g_color", gStyle.gradColor) == 4;
  ok &= prefs.putUChar("g_opa", gStyle.gradOpa) == 1;
  ok &= prefs.putUChar("g_h", gStyle.gradHeight) == 1;
  return ok;
}

uint8_t photoPageSec() { return gCfg.pageSec; }

bool photoSetPageSec(uint8_t sec) {
  if (sec < 5) sec = 5;
  if (sec > 120) sec = 120;
  gCfg.pageSec = sec;
  return saveConfig();
}

void photoReset() {
  defaults(gCfg);
  saveConfig();
  defaultsStyle(gStyle);
  photoSetStyle(gStyle);
  refresh();
}

size_t photoTotalBytes() { return SPIFFS.totalBytes(); }
size_t photoUsedBytes() { return SPIFFS.usedBytes(); }

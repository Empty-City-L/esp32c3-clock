/**
 * web_config.cpp — HTTP 配网 / 设置接口
 *
 * 路由：
 *   GET  /                 配置页面
 *   GET  /api/config       读取配置（不返回密码）
 *   POST /api/config       保存配置并重启
 *   GET  /api/status       设备状态
 *   GET  /api/scan         扫描附近 WiFi
 *   GET  /api/brightness   实时预览背光（不写 NVS）
 *   POST /api/restart      重启
 *   POST /api/factory      恢复出厂并重启
 */
#include "web_config.h"

#include <DNSServer.h>
#include <SPIFFS.h>
#include <WebServer.h>
#include <WiFi.h>
#include <esp_heap_caps.h>
#include <string.h>
#include <time.h>

#include "board_config.h"
#include "pages.h"
#include "photo.h"
#include "settings.h"
#include "weather.h"
#include "web_ui.h"

extern const uint8_t *appScreenBuffer(size_t *len);
extern const char *appPhotoDebug();
extern const char *appPageDebug();
extern bool appForcePhoto();

namespace {

WebServer server(80);
DNSServer dns;

bool g_started = false;
bool g_apMode = false;
bool g_dnsStarted = false;
bool g_rebootPending = false;
uint32_t g_rebootAt = 0;
char g_apSsid[33] = "";
BrightnessHandler g_brightnessHandler = nullptr;

File g_uploadFile;
int g_uploadSlot = -1;
size_t g_uploadBytes = 0;
bool g_uploadError = false;
String g_uploadMessage;

/* ------------------------- 小工具 ------------------------- */

String jsonEscape(const char *s) {
  String out;
  if (!s) return out;
  for (const unsigned char *p = (const unsigned char *)s; *p; ++p) {
    switch (*p) {
      case '"':  out += "\\\""; break;
      case '\\': out += "\\\\"; break;
      case '\b': out += "\\b"; break;
      case '\f': out += "\\f"; break;
      case '\n': out += "\\n"; break;
      case '\r': out += "\\r"; break;
      case '\t': out += "\\t"; break;
      default:
        if (*p < 0x20) {
          char buf[8];
          snprintf(buf, sizeof(buf), "\\u%04X", *p);
          out += buf;
        } else {
          out += (char)*p;
        }
    }
  }
  return out;
}

void sendJson(int code, const String &body) {
  server.sendHeader("Cache-Control", "no-store");
  server.send(code, "application/json; charset=utf-8", body);
}

void sendError(int code, const char *message) {
  String j = "{\"ok\":false,\"error\":\"";
  j += jsonEscape(message);
  j += "\"}";
  sendJson(code, j);
}

void sendOk(const char *message) {
  String j = "{\"ok\":true";
  if (message && message[0]) {
    j += ",\"message\":\"";
    j += jsonEscape(message);
    j += "\"";
  }
  j += "}";
  sendJson(200, j);
}

bool argBool(const char *name) {
  if (!server.hasArg(name)) return false;
  String v = server.arg(name);
  return v == "1" || v == "true" || v == "on" || v == "yes";
}

uint32_t colorArg(const char *name, uint32_t fallback) {
  String v = server.arg(name);
  v.trim();
  if (v.startsWith("#")) v = v.substring(1);
  if (v.length() == 3) {
    v = String(v[0]) + v[0] + v[1] + v[1] + v[2] + v[2];
  }
  if (v.length() != 6) return fallback;
  char *end = nullptr;
  uint32_t c = strtoul(v.c_str(), &end, 16);
  if (!end || *end) return fallback;
  return c & 0xFFFFFF;
}

void rebootLater(uint32_t ms) {
  g_rebootPending = true;
  g_rebootAt = millis() + ms;
}

const char *weekdayCn(int wd) {
  static const char *names[] = {"周日", "周一", "周二", "周三", "周四", "周五", "周六"};
  if (wd < 0 || wd > 6) wd = 0;
  return names[wd];
}

String weatherJson() {
  const WeatherData &w = weatherData();
  String j = "{";
  j += "\"valid\":";
  j += w.valid ? "true" : "false";
  j += ",\"updating\":";
  j += w.updating ? "true" : "false";
  j += ",\"city\":\"" + jsonEscape(w.city) + "\",";
  j += "\"text\":\"" + jsonEscape(w.text) + "\",";
  j += "\"temp\":" + String((int)w.temp) + ",";
  j += "\"high\":" + String((int)w.high) + ",";
  j += "\"low\":" + String((int)w.low) + ",";
  j += "\"humidity\":" + String((unsigned)w.humidity) + ",";
  j += "\"wind\":\"" + jsonEscape(w.wind) + "\",";
  j += "\"age\":" + String((unsigned long)weatherAgeSec()) + ",";
  j += "\"error\":\"" + jsonEscape(w.error) + "\"";
  j += "}";
  return j;
}

/* ------------------------- 页面 ------------------------- */

void handleRoot() {
  server.sendHeader("Cache-Control", "no-store");
  server.sendHeader("X-Content-Type-Options", "nosniff");
  server.send_P(200, "text/html; charset=utf-8", INDEX_HTML);
}

void handleFavicon() {
  server.send(204, "text/plain", "");
}

/* ------------------------- 配置 ------------------------- */

void handleConfigGet() {
  String j = "{";
  j += "\"ssid\":\"" + jsonEscape(gSettings.wifiSsid) + "\",";
  j += "\"pass_set\":";
  j += gSettings.wifiPass[0] ? "true" : "false";
  j += ",\"open_network\":";
  j += (gSettings.wifiSsid[0] != '\0' && gSettings.wifiPass[0] == '\0') ? "true" : "false";
  j += ",\"tz\":\"" + jsonEscape(gSettings.tz) + "\",";
  j += "\"ntp1\":\"" + jsonEscape(gSettings.ntp1) + "\",";
  j += "\"ntp2\":\"" + jsonEscape(gSettings.ntp2) + "\",";
  j += "\"brightness\":" + String((unsigned)gSettings.brightness) + ",";
  j += "\"twelve_hour\":";
  j += (gSettings.flags & FLAG_12H) ? "true" : "false";
  j += ",\"show_seconds\":";
  j += (gSettings.flags & FLAG_SHOW_SECONDS) ? "true" : "false";
  j += ",\"invert\":";
  j += (gSettings.flags & FLAG_INVERT) ? "true" : "false";
  j += ",\"color_order\":\"";
  j += (gSettings.flags & FLAG_BGR) ? "bgr" : "rgb";
  j += "\",\"rotation\":" + String((unsigned)gSettings.rotation) + ",";
  j += "\"offset_x\":" + String((unsigned)gSettings.offsetX) + ",";
  j += "\"offset_y\":" + String((unsigned)gSettings.offsetY) + ",";
  j += "\"refresh_mode\":" + String((unsigned)gSettings.refresh) + ",";

  const WeatherConfig &wc = weatherConfig();
  j += "\"weather_enabled\":";
  j += wc.enabled ? "true" : "false";
  j += ",\"weather_key_set\":";
  j += weatherKeySet() ? "true" : "false";
  j += ",\"weather_location\":\"" + jsonEscape(wc.location) + "\",";
  j += "\"weather_update_min\":" + String((unsigned)wc.updateMin) + ",";
  j += "\"weather_page_sec\":" + String((unsigned)wc.pageSec) + ",";
  j += "\"weather_layout\":" + String((unsigned)weatherLayout()) + ",";
  j += "\"photo_page_sec\":" + String((unsigned)photoPageSec()) + ",";
  const PhotoStyle &ps = photoStyle();
  char tcol[10];
  char gcol[10];
  snprintf(tcol, sizeof(tcol), "#%06X", (unsigned)ps.timeColor);
  snprintf(gcol, sizeof(gcol), "#%06X", (unsigned)ps.gradColor);
  j += "\"photo_time_on\":";
  j += ps.timeOn ? "true" : "false";
  j += ",\"photo_time_style\":" + String((unsigned)ps.timeStyle) + ",";
  j += "\"photo_time_color\":\"" + String(tcol) + "\",";
  j += "\"photo_colon_style\":" + String((unsigned)ps.colonStyle) + ",";
  j += "\"photo_grad_on\":";
  j += ps.gradOn ? "true" : "false";
  j += ",\"photo_grad_color\":\"" + String(gcol) + "\",";
  j += "\"photo_grad_opa\":" + String((unsigned)ps.gradOpa) + ",";
  j += "\"photo_grad_height\":" + String((unsigned)ps.gradHeight) + ",";
  j += "\"photo_count\":" + String((unsigned)photoCount()) + ",";
  j += "\"photo_max\":" + String((unsigned)PHOTO_MAX) + ",";
  j += "\"photo_frames\":" + String((unsigned)photoFrameCount()) + ",";
  j += "\"photo_delay\":" + String((unsigned)photoDelayMs()) + ",";
  j += "\"photo_used\":" + String((unsigned long)photoUsedBytes()) + ",";
  j += "\"photo_total\":" + String((unsigned long)photoTotalBytes()) + ",";
  const PageConfig &pc = pagesConfig();
  j += "\"page_show_clock\":";
  j += (pc.visibleMask & (1 << PAGE_ID_CLOCK)) ? "true" : "false";
  j += ",\"page_show_weather\":";
  j += (pc.visibleMask & (1 << PAGE_ID_WEATHER)) ? "true" : "false";
  j += ",\"page_show_forecast\":";
  j += (pc.visibleMask & (1 << PAGE_ID_FORECAST)) ? "true" : "false";
  j += ",\"page_show_photo\":";
  j += (pc.visibleMask & (1 << PAGE_ID_PHOTO)) ? "true" : "false";
  j += ",\"page_order_clock\":" + String((unsigned)pc.order[PAGE_ID_CLOCK]) + ",";
  j += "\"page_order_weather\":" + String((unsigned)pc.order[PAGE_ID_WEATHER]) + ",";
  j += "\"page_order_forecast\":" + String((unsigned)pc.order[PAGE_ID_FORECAST]) + ",";
  j += "\"page_order_photo\":" + String((unsigned)pc.order[PAGE_ID_PHOTO]) + ",";
  j += "\"resident_page\":" + String((unsigned)pc.resident) + ",";
  j += "\"version\":\"" APP_VERSION "\"";
  j += ",\"ap_ssid\":\"" + jsonEscape(g_apSsid) + "\"";
  j += ",\"mdns\":\"" MDNS_HOST ".local\"";
  j += "}";
  sendJson(200, j);
}

static bool validHostToken(const String &s) {
  if (!s.length() || s.length() > 63) return false;
  for (size_t i = 0; i < s.length(); ++i) {
    char c = s[i];
    if (c == ' ' || c == '\t' || c == '\r' || c == '\n') return false;
  }
  return true;
}

void handleConfigPost() {
  Settings s = gSettings;

  String ssid = server.arg("ssid");
  ssid.trim();
  if (ssid.length() == 0) {
    sendError(400, "请填写 Wi-Fi 名称");
    return;
  }
  if (ssid.length() > 32) {
    sendError(400, "Wi-Fi 名称不能超过 32 字节");
    return;
  }

  const bool openNet = argBool("open");
  const bool ssidChanged = ssid != String(s.wifiSsid);
  String pass = server.arg("pass");

  if (!openNet) {
    if (pass.length() == 0 && (ssidChanged || s.wifiPass[0] == '\0')) {
      sendError(400, "请填写 Wi-Fi 密码，或勾选“开放网络”");
      return;
    }
    if (pass.length() > 0 && pass.length() < 8) {
      sendError(400, "WPA 密码至少 8 位；开放网络请勾选对应选项");
      return;
    }
    if (pass.length() > 63) {
      sendError(400, "Wi-Fi 密码不能超过 63 字节");
      return;
    }
  }

  String tz = server.arg("tz");
  tz.trim();
  if (tz.length() == 0) tz = TZ_DEFAULT;
  if (!validHostToken(tz)) {
    sendError(400, "时区字符串格式不正确");
    return;
  }

  String ntp1 = server.arg("ntp1");
  String ntp2 = server.arg("ntp2");
  ntp1.trim();
  ntp2.trim();
  if (ntp1.length() == 0) ntp1 = NTP_DEFAULT_1;
  if (ntp2.length() == 0) ntp2 = NTP_DEFAULT_2;
  if (!validHostToken(ntp1) || !validHostToken(ntp2)) {
    sendError(400, "NTP 服务器地址格式不正确");
    return;
  }

  int brightness = server.arg("brightness").toInt();
  brightness = constrain(brightness, 5, 100);

  int rotation = server.arg("rotation").toInt();
  if (rotation != 1 && rotation != 3) rotation = 1;

  int offsetX = constrain(server.arg("offset_x").toInt(), 0, 8);
  int offsetY = constrain(server.arg("offset_y").toInt(), 0, 8);
  const uint8_t refreshMode = (uint8_t)constrain(server.arg("refresh_mode").toInt(), 0, 2);

  /* 先校验天气字段，再统一写 NVS，避免校验失败时只保存了一半配置。 */
  WeatherConfig wc = weatherConfig();
  wc.enabled = argBool("weather_enabled");

  String wkey = server.arg("weather_key");
  wkey.trim();
  if (wkey.length() > sizeof(wc.apiKey) - 1) {
    sendError(400, "心知天气 Key 太长");
    return;
  }
  if (wkey.length() > 0) {
    memset(wc.apiKey, 0, sizeof(wc.apiKey));
    strncpy(wc.apiKey, wkey.c_str(), sizeof(wc.apiKey) - 1);
  }

  String wloc = server.arg("weather_location");
  wloc.trim();
  if (wloc.length() == 0) wloc = "beijing";
  if (wloc.length() > sizeof(wc.location) - 1) {
    sendError(400, "城市名太长（最多 31 字节）");
    return;
  }
  memset(wc.location, 0, sizeof(wc.location));
  strncpy(wc.location, wloc.c_str(), sizeof(wc.location) - 1);

  wc.updateMin = (uint8_t)constrain(server.arg("weather_update").toInt(), 5, 180);
  wc.pageSec = (uint8_t)constrain(server.arg("weather_page").toInt(), 5, 60);
  const uint8_t wlayout = (uint8_t)constrain(server.arg("weather_layout").toInt(), 0, 1);

  settingsSetString(s.wifiSsid, sizeof(s.wifiSsid), ssid.c_str());
  if (openNet) {
    s.wifiPass[0] = '\0';
  } else if (pass.length() > 0) {
    settingsSetString(s.wifiPass, sizeof(s.wifiPass), pass.c_str());
  }
  settingsSetString(s.tz, sizeof(s.tz), tz.c_str());
  settingsSetString(s.ntp1, sizeof(s.ntp1), ntp1.c_str());
  settingsSetString(s.ntp2, sizeof(s.ntp2), ntp2.c_str());
  s.brightness = (uint8_t)brightness;
  s.rotation = (uint8_t)rotation;
  s.offsetX = (uint8_t)offsetX;
  s.offsetY = (uint8_t)offsetY;
  s.refresh = refreshMode;

  s.flags = 0;
  if (argBool("twelve_hour")) s.flags |= FLAG_12H;
  if (argBool("show_seconds")) s.flags |= FLAG_SHOW_SECONDS;
  if (argBool("invert")) s.flags |= FLAG_INVERT;
  if (server.arg("color_order") == "bgr") s.flags |= FLAG_BGR;

  if (!settingsSave(s)) {
    sendError(500, "写入 NVS 失败，请重试");
    return;
  }
  gSettings = s;

  /* 天气配置单独存 NVS，不影响上面的 WiFi/时钟字段。 */
  if (!weatherSetConfig(wc)) {
    sendError(500, "天气配置写入 NVS 失败，请重试");
    return;
  }
  if (!weatherSetLayout(wlayout)) {
    sendError(500, "3 天天气布局写入 NVS 失败，请重试");
    return;
  }

  const int photoSec = constrain(server.arg("photo_page_sec").toInt(), 5, 120);
  if (!photoSetPageSec((uint8_t)photoSec)) {
    sendError(500, "照片页配置写入 NVS 失败，请重试");
    return;
  }

  PhotoStyle ps = photoStyle();
  ps.timeOn = argBool("photo_time_on");
  ps.timeStyle = (uint8_t)constrain(server.arg("photo_time_style").toInt(), 0, 2);
  ps.timeColor = colorArg("photo_time_color", ps.timeColor);
  ps.colonStyle = (uint8_t)constrain(server.arg("photo_colon_style").toInt(), 0, 3);
  ps.gradOn = argBool("photo_grad_on");
  ps.gradColor = colorArg("photo_grad_color", ps.gradColor);
  ps.gradOpa = (uint8_t)constrain(server.arg("photo_grad_opa").toInt(), 0, 100);
  const int gh = server.arg("photo_grad_height").toInt();
  ps.gradHeight = (gh == 48 || gh == 56 || gh == 72) ? (uint8_t)gh : 56;
  if (!photoSetStyle(ps)) {
    sendError(500, "照片页时间样式写入 NVS 失败，请重试");
    return;
  }

  PageConfig pc = pagesConfig();
  pc.visibleMask = 0;
  if (argBool("page_show_clock")) pc.visibleMask |= 1 << PAGE_ID_CLOCK;
  if (argBool("page_show_weather")) pc.visibleMask |= 1 << PAGE_ID_WEATHER;
  if (argBool("page_show_forecast")) pc.visibleMask |= 1 << PAGE_ID_FORECAST;
  if (argBool("page_show_photo")) pc.visibleMask |= 1 << PAGE_ID_PHOTO;
  pc.order[PAGE_ID_CLOCK] =
      (uint8_t)constrain(server.arg("page_order_clock").toInt(), 0, 3);
  pc.order[PAGE_ID_WEATHER] =
      (uint8_t)constrain(server.arg("page_order_weather").toInt(), 0, 3);
  pc.order[PAGE_ID_FORECAST] =
      (uint8_t)constrain(server.arg("page_order_forecast").toInt(), 0, 3);
  pc.order[PAGE_ID_PHOTO] =
      (uint8_t)constrain(server.arg("page_order_photo").toInt(), 0, 3);
  pc.resident = (uint8_t)constrain(server.arg("resident_page").toInt(), 0, 4);
  if (!pagesSetConfig(pc)) {
    sendError(500, "页面配置写入 NVS 失败，请重试");
    return;
  }

  if (g_brightnessHandler) g_brightnessHandler(s.brightness);

  Serial.printf("[web] config saved: ssid='%s' tz='%s' bright=%u\n",
                s.wifiSsid, s.tz, s.brightness);

  String j = "{\"ok\":true,\"reboot\":true,\"message\":\"保存成功，设备正在重启\"}";
  sendJson(200, j);
  rebootLater(1500);
}

/* ------------------------- 状态 / 扫描 ------------------------- */

void handleStatus() {
  const bool wifi = WiFi.status() == WL_CONNECTED;
  const time_t now = time(nullptr);
  const bool timeValid = now > 1700000000;

  char timeBuf[16] = "--:--:--";
  char dateBuf[40] = "等待对时";
  if (timeValid) {
    struct tm t;
    localtime_r(&now, &t);
    strftime(timeBuf, sizeof(timeBuf), "%H:%M:%S", &t);
    snprintf(dateBuf, sizeof(dateBuf), "%d月%d日 %s", t.tm_mon + 1, t.tm_mday,
             weekdayCn(t.tm_wday));
  }

  IPAddress ip = wifi ? WiFi.localIP() : WiFi.softAPIP();

  String j = "{";
  j += "\"mode\":\"";
  j += g_apMode ? "ap" : "sta";
  j += "\",\"wifi_connected\":";
  j += wifi ? "true" : "false";
  j += ",\"ssid\":\"" + jsonEscape(wifi ? WiFi.SSID().c_str() : "") + "\",";
  j += "\"ip\":\"" + ip.toString() + "\",";
  j += "\"ap_ssid\":\"" + jsonEscape(g_apSsid) + "\",";
  j += "\"rssi\":" + String(wifi ? WiFi.RSSI() : 0) + ",";
  j += "\"uptime\":" + String(millis() / 1000) + ",";
  j += "\"heap\":" + String(ESP.getFreeHeap()) + ",";
  j += "\"time\":\"" + String(timeBuf) + "\",";
  j += "\"date\":\"" + jsonEscape(dateBuf) + "\",";
  j += "\"time_valid\":";
  j += timeValid ? "true" : "false";
  j += ",\"version\":\"" APP_VERSION "\",";
  j += "\"chip\":\"" + String(ESP.getChipModel()) + "\",";
  j += "\"mac\":\"" + WiFi.macAddress() + "\",";
  j += "\"mdns\":\"" MDNS_HOST ".local\",";
  j += "\"photo_count\":" + String((unsigned)photoCount()) + ",";
  j += "\"photo_frames\":" + String((unsigned)photoFrameCount()) + ",";
  j += "\"photo_delay\":" + String((unsigned)photoDelayMs()) + ",";
  j += "\"page_debug\":\"" + jsonEscape(appPageDebug()) + "\",";
  j += "\"photo_debug\":\"" + jsonEscape(appPhotoDebug()) + "\",";
  j += "\"photo_dma_free\":" + String((unsigned long)heap_caps_get_free_size(MALLOC_CAP_DMA)) + ",";
  j += "\"photo_dma_largest\":" + String((unsigned long)heap_caps_get_largest_free_block(MALLOC_CAP_DMA)) + ",";
  j += "\"weather\":" + weatherJson();
  j += "}";
  sendJson(200, j);
}

void handleScan() {
  // 阻塞式扫描约 2~4 秒；只会在用户主动点击“扫描”时发生。
  int n = WiFi.scanNetworks(false, true);
  String j = "{\"networks\":[";
  int count = 0;
  for (int i = 0; i < n; ++i) {
    String ssid = WiFi.SSID(i);
    if (ssid.length() == 0) continue;
    if (count++) j += ",";
    j += "{\"ssid\":\"" + jsonEscape(ssid.c_str()) + "\",";
    j += "\"rssi\":" + String(WiFi.RSSI(i)) + ",";
    j += "\"enc\":";
    j += (WiFi.encryptionType(i) != WIFI_AUTH_OPEN) ? "true" : "false";
    j += ",\"channel\":" + String((int)WiFi.channel(i)) + "}";
    if (count >= 30) break;
  }
  j += "]}";
  WiFi.scanDelete();
  sendJson(200, j);
}

void handleBrightness() {
  int v = server.arg("value").toInt();
  v = constrain(v, 5, 100);
  if (g_brightnessHandler) g_brightnessHandler((uint8_t)v);
  String j = "{\"ok\":true,\"brightness\":" + String(v) + "}";
  sendJson(200, j);
}

void handleWeather() { sendJson(200, weatherJson()); }

void handleWeatherRefresh() {
  if (!weatherConfig().enabled || !weatherKeySet()) {
    sendError(400, "请先启用天气并填写心知天气 Key");
    return;
  }
  weatherRequestUpdate();
  sendOk("正在更新天气，请稍候");
}

/* ------------------------- 相册 ------------------------- */

void handlePhotoUploadChunk() {
  HTTPUpload &upload = server.upload();

  if (upload.status == UPLOAD_FILE_START) {
    g_uploadError = false;
    g_uploadBytes = 0;
    g_uploadMessage = "";
    g_uploadSlot = 0;

    // 12 字节头 + 最多 16 帧 × 40 KB，留一点余量。
    if (upload.totalSize > 700000) {
      g_uploadError = true;
      g_uploadMessage = "动图文件超过 700 KB";
      g_uploadSlot = -1;
      return;
    }
    if (SPIFFS.exists("/photo.tmp")) SPIFFS.remove("/photo.tmp");
    g_uploadFile = SPIFFS.open("/photo.tmp", "w");
    if (!g_uploadFile) {
      g_uploadError = true;
      g_uploadMessage = "SPIFFS 写入失败";
      g_uploadSlot = -1;
    }
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (g_uploadError || !g_uploadFile) return;
    if (g_uploadBytes + upload.currentSize > 700000) {
      g_uploadError = true;
      g_uploadMessage = "动图文件超过 700 KB";
      return;
    }
    const size_t wrote = g_uploadFile.write(upload.buf, upload.currentSize);
    if (wrote != upload.currentSize) {
      g_uploadError = true;
      g_uploadMessage = "写入不完整";
      return;
    }
    g_uploadBytes += upload.currentSize;
  } else if (upload.status == UPLOAD_FILE_END) {
    if (g_uploadFile) g_uploadFile.close();
    if (g_uploadError || g_uploadSlot < 0) {
      SPIFFS.remove("/photo.tmp");
      g_uploadSlot = -1;
      return;
    }

    if (!photoValidatePath("/photo.tmp")) {
      SPIFFS.remove("/photo.tmp");
      g_uploadError = true;
      g_uploadMessage = "动图格式无效（需 1–16 帧 RGB565）";
      g_uploadSlot = -1;
      return;
    }

    if (SPIFFS.exists("/photo.anim")) SPIFFS.remove("/photo.anim");
    if (!SPIFFS.rename("/photo.tmp", "/photo.anim") || !photoValidate()) {
      SPIFFS.remove("/photo.tmp");
      g_uploadError = true;
      g_uploadMessage = "照片保存失败";
      g_uploadSlot = -1;
      return;
    }
    photoRefresh();
  } else if (upload.status == UPLOAD_FILE_ABORTED) {
    if (g_uploadFile) g_uploadFile.close();
    SPIFFS.remove("/photo.tmp");
    g_uploadSlot = -1;
    g_uploadError = true;
    g_uploadMessage = "上传中断";
  }
}

void handlePhotoUploadComplete() {
  if (g_uploadError || g_uploadSlot < 0) {
    sendError(409, g_uploadMessage.length() ? g_uploadMessage.c_str() : "上传失败");
    g_uploadSlot = -1;
    g_uploadError = false;
    return;
  }

  String j = "{\"ok\":true,\"slot\":0";
  j += ",\"size\":" + String((unsigned long)g_uploadBytes);
  j += ",\"count\":" + String((unsigned)photoCount());
  j += ",\"frames\":" + String((unsigned)photoFrameCount());
  j += ",\"delay\":" + String((unsigned)photoDelayMs()) + "}";
  sendJson(200, j);
  g_uploadSlot = -1;
  g_uploadError = false;
}

void handlePhotos() {
  String j = "{\"max\":1";
  j += ",\"count\":" + String((unsigned)photoCount());
  j += ",\"frames\":" + String((unsigned)photoFrameCount());
  j += ",\"delay\":" + String((unsigned)photoDelayMs());
  j += ",\"used\":" + String((unsigned long)photoUsedBytes());
  j += ",\"total\":" + String((unsigned long)photoTotalBytes());
  j += ",\"photos\":[";
  if (photoCount()) {
    j += "{\"slot\":0,\"size\":" + String((unsigned long)photoSize()) + ",\"frames\":" +
         String((unsigned)photoFrameCount()) + ",\"delay\":" +
         String((unsigned)photoDelayMs()) + "}";
  }
  j += "]}";
  sendJson(200, j);
}

void handlePhotoGet() {
  File f = SPIFFS.open("/photo.anim", "r");
  if (!f) {
    sendError(404, "照片不存在");
    return;
  }
  server.sendHeader("Cache-Control", "private,max-age=86400");
  server.streamFile(f, "application/octet-stream");
  f.close();
}

void handlePhotoDelete() {
  if (!photoDelete()) {
    sendError(404, "照片不存在");
    return;
  }
  sendOk("照片已删除");
}

void handlePhotoShow() {
  if (!appForcePhoto()) {
    sendError(500, "照片显示失败");
    return;
  }
  sendOk("已切到照片页");
}

/* 临时调试：抓当前 LVGL 帧缓冲。 */
void handleScreenshot() {
  size_t len = 0;
  const uint8_t *buf = appScreenBuffer(&len);
  if (!buf || len == 0) {
    sendError(503, "帧缓冲尚未就绪");
    return;
  }
  server.sendHeader("Cache-Control", "no-store");
  server.sendHeader("X-Screen-W", String(SCREEN_W));
  server.sendHeader("X-Screen-H", String(SCREEN_H));
  server.sendHeader("X-Rotation", String((unsigned)gSettings.rotation));
  server.setContentLength(len);
  server.send(200, "application/octet-stream", "");
  server.sendContent(reinterpret_cast<const char *>(buf), len);
}

/* ------------------------- 系统 ------------------------- */

void handleRestart() {
  sendOk("设备正在重启");
  rebootLater(700);
}

void handleFactoryReset() {
  settingsReset();
  weatherReset();
  pagesReset();
  photoDelete();
  photoReset();
  sendOk("已恢复出厂设置，正在重启");
  rebootLater(900);
}

void handleNotFound() {
  if (server.method() == HTTP_GET) {
    String uri = server.uri();
    if (uri == "/favicon.ico") {
      handleFavicon();
      return;
    }
    if (g_apMode && !uri.startsWith("/api/")) {
      // Captive portal：把所有域名请求拉回配置页
      server.sendHeader("Location", "http://192.168.4.1/", true);
      server.send(302, "text/plain", "");
      return;
    }
  }
  if (server.uri().startsWith("/api/")) {
    sendError(404, "接口不存在");
  } else {
    server.send(404, "text/plain; charset=utf-8", "Not found");
  }
}

}  // namespace

void webSetBrightnessHandler(BrightnessHandler handler) {
  g_brightnessHandler = handler;
}

void webBegin(bool apMode, const char *apSsid) {
  if (apSsid) {
    strncpy(g_apSsid, apSsid, sizeof(g_apSsid) - 1);
    g_apSsid[sizeof(g_apSsid) - 1] = '\0';
  }

  if (!g_started) {
    server.on("/", HTTP_GET, handleRoot);
    server.on("/favicon.ico", HTTP_GET, handleFavicon);
    server.on("/api/config", HTTP_GET, handleConfigGet);
    server.on("/api/config", HTTP_POST, handleConfigPost);
    server.on("/api/status", HTTP_GET, handleStatus);
    server.on("/api/scan", HTTP_GET, handleScan);
    server.on("/api/brightness", HTTP_GET, handleBrightness);
    server.on("/api/weather", HTTP_GET, handleWeather);
    server.on("/api/weather/refresh", HTTP_POST, handleWeatherRefresh);
    server.on("/api/screenshot.raw", HTTP_GET, handleScreenshot);
    server.on("/api/photos", HTTP_GET, handlePhotos);
    server.on("/api/photo", HTTP_GET, handlePhotoGet);
    server.on("/api/photo/upload", HTTP_POST, handlePhotoUploadComplete,
              handlePhotoUploadChunk);
    server.on("/api/photo/delete", HTTP_POST, handlePhotoDelete);
    server.on("/api/photo/show", HTTP_POST, handlePhotoShow);
    server.on("/api/restart", HTTP_POST, handleRestart);
    server.on("/api/factory", HTTP_POST, handleFactoryReset);
    server.onNotFound(handleNotFound);

    server.begin();
    g_started = true;
    Serial.println("[web] HTTP server started on port 80");
  }

  webSetApMode(apMode);
}

void webSetApMode(bool apMode) {
  g_apMode = apMode;
  if (apMode) {
    if (!g_dnsStarted) {
      dns.start(53, "*", WiFi.softAPIP());
      g_dnsStarted = true;
      Serial.printf("[web] captive portal DNS at %s\n", WiFi.softAPIP().toString().c_str());
    }
  } else if (g_dnsStarted) {
    dns.stop();
    g_dnsStarted = false;
    Serial.println("[web] captive portal DNS stopped");
  }
}

bool webIsApMode() { return g_apMode; }
const char *webApSsid() { return g_apSsid; }

void webLoop() {
  server.handleClient();
  if (g_dnsStarted) dns.processNextRequest();

  if (g_rebootPending && (int32_t)(millis() - g_rebootAt) >= 0) {
    Serial.println("[web] reboot now");
    delay(80);
    ESP.restart();
  }
}

#include "pages.h"

#include <Preferences.h>

namespace {

Preferences prefs;
PageConfig gCfg;

void defaults(PageConfig &c) {
  for (uint8_t i = 0; i < PAGE_COUNT; ++i) c.order[i] = i;
  c.visibleMask = 0x0F;
  c.resident = 0;
}

void sanitize(PageConfig &c) {
  bool used[PAGE_COUNT] = {false, false, false, false};
  for (uint8_t i = 0; i < PAGE_COUNT; ++i) {
    if (c.order[i] >= PAGE_COUNT || used[c.order[i]]) {
      c.order[i] = 0xFF;
    } else {
      used[c.order[i]] = true;
    }
  }
  for (uint8_t i = 0; i < PAGE_COUNT; ++i) {
    if (c.order[i] != 0xFF) continue;
    for (uint8_t pos = 0; pos < PAGE_COUNT; ++pos) {
      if (!used[pos]) {
        c.order[i] = pos;
        used[pos] = true;
        break;
      }
    }
  }
  if ((c.visibleMask & 0x0F) == 0) c.visibleMask = 1 << PAGE_ID_CLOCK;
  c.visibleMask &= 0x0F;
  if (c.resident > 4) c.resident = 0;
}

}  // namespace

void pagesBegin() {
  prefs.begin("pages", false);
  PageConfig c;
  defaults(c);
  uint8_t ord[PAGE_COUNT];
  if (prefs.getBytes("ord", ord, sizeof(ord)) == sizeof(ord)) {
    for (uint8_t i = 0; i < PAGE_COUNT; ++i) c.order[i] = ord[i];
  }
  c.visibleMask = prefs.getUChar("vis", 0x0F);
  c.resident = prefs.getUChar("res", 0);
  sanitize(c);
  gCfg = c;
  Serial.printf("[pages] order=%u,%u,%u,%u vis=0x%X resident=%u\n",
                gCfg.order[0], gCfg.order[1], gCfg.order[2], gCfg.order[3],
                gCfg.visibleMask, gCfg.resident);
}

const PageConfig &pagesConfig() { return gCfg; }

bool pagesSetConfig(const PageConfig &cfg) {
  gCfg = cfg;
  sanitize(gCfg);
  bool ok = prefs.putBytes("ord", gCfg.order, sizeof(gCfg.order)) == sizeof(gCfg.order);
  ok &= prefs.putUChar("vis", gCfg.visibleMask) == 1;
  ok &= prefs.putUChar("res", gCfg.resident) == 1;
  return ok;
}

void pagesReset() {
  defaults(gCfg);
  pagesSetConfig(gCfg);
}

bool pageVisible(uint8_t id) {
  if (id >= PAGE_COUNT) return false;
  return (gCfg.visibleMask & (1 << id)) != 0;
}

uint8_t pageOrder(uint8_t id) {
  if (id >= PAGE_COUNT) return 0;
  return gCfg.order[id];
}

uint8_t residentPage() { return gCfg.resident; }

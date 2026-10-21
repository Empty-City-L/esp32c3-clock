#include "settings.h"

#include <Preferences.h>
#include <string.h>

#include "board_config.h"

Settings gSettings;
static Preferences prefs;

void settingsSetString(char *dst, size_t dstSize, const char *src) {
  if (!dst || dstSize == 0) return;
  if (!src) src = "";
  strncpy(dst, src, dstSize - 1);
  dst[dstSize - 1] = '\0';
}

void settingsDefaults(Settings &s) {
  memset(&s, 0, sizeof(s));
  s.magic = CONFIG_MAGIC;
  s.version = CONFIG_VERSION;
  s.brightness = 85;
  s.flags = FLAG_SHOW_SECONDS;
  if (!LCD_RGB_ORDER) s.flags |= FLAG_BGR;  // LovyanGFX: false = BGR
  if (LCD_INVERT) s.flags |= FLAG_INVERT;
  s.rotation = (SCREEN_ROTATION == 3) ? 3 : 1;
  s.offsetX = LCD_OFFSET_X;
  s.offsetY = LCD_OFFSET_Y;
  settingsSetString(s.tz, sizeof(s.tz), TZ_DEFAULT);
  settingsSetString(s.ntp1, sizeof(s.ntp1), NTP_DEFAULT_1);
  settingsSetString(s.ntp2, sizeof(s.ntp2), NTP_DEFAULT_2);
}

static void sanitize(Settings &s) {
  s.wifiSsid[sizeof(s.wifiSsid) - 1] = '\0';
  s.wifiPass[sizeof(s.wifiPass) - 1] = '\0';
  s.tz[sizeof(s.tz) - 1] = '\0';
  s.ntp1[sizeof(s.ntp1) - 1] = '\0';
  s.ntp2[sizeof(s.ntp2) - 1] = '\0';

  if (s.brightness < 5 || s.brightness > 100) s.brightness = 85;
  if (s.rotation != 1 && s.rotation != 3) s.rotation = 1;
  if (s.offsetX > 8) s.offsetX = LCD_OFFSET_X;
  if (s.offsetY > 8) s.offsetY = LCD_OFFSET_Y;
  if (s.refresh > 2) s.refresh = 0;
  s.flags &= (FLAG_12H | FLAG_SHOW_SECONDS | FLAG_INVERT | FLAG_BGR);
  if (s.tz[0] == '\0') settingsSetString(s.tz, sizeof(s.tz), TZ_DEFAULT);
  if (s.ntp1[0] == '\0') settingsSetString(s.ntp1, sizeof(s.ntp1), NTP_DEFAULT_1);
  if (s.ntp2[0] == '\0') settingsSetString(s.ntp2, sizeof(s.ntp2), NTP_DEFAULT_2);
}

void settingsBegin() {
  prefs.begin("clockc3", false);

  Settings s;
  size_t n = prefs.getBytes("cfg", &s, sizeof(s));
  if (n != sizeof(s) || s.magic != CONFIG_MAGIC || s.version != CONFIG_VERSION) {
    settingsDefaults(gSettings);
    settingsSave(gSettings);
    Serial.println("[cfg] defaults written");
  } else {
    gSettings = s;
  }
  sanitize(gSettings);

  Serial.printf("[cfg] brightness=%u rotation=%u offset=%u/%u flags=0x%02X refresh=%u\n",
                gSettings.brightness, gSettings.rotation, gSettings.offsetX,
                gSettings.offsetY, gSettings.flags, gSettings.refresh);
  Serial.printf("[cfg] wifi='%s' tz='%s' ntp='%s'\n",
                gSettings.wifiSsid[0] ? gSettings.wifiSsid : "(unset)",
                gSettings.tz, gSettings.ntp1);
}

bool settingsSave(const Settings &s) {
  return prefs.putBytes("cfg", &s, sizeof(s)) == sizeof(s);
}

void settingsReset() {
  prefs.clear();
  Settings s;
  settingsDefaults(s);
  settingsSave(s);
  prefs.end();
  Serial.println("[cfg] factory reset");
}

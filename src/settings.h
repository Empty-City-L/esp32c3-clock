/**
 * settings.h — 掉电保存的用户配置（NVS / Preferences）
 */
#pragma once

#include <Arduino.h>

#define CONFIG_MAGIC   0x4333434CUL  // "C3CL"
#define CONFIG_VERSION 1

/* 显示 / 时间选项位 */
enum : uint8_t {
  FLAG_12H          = 1 << 0,  // 12 小时制
  FLAG_SHOW_SECONDS = 1 << 1,  // 显示秒环
  FLAG_INVERT       = 1 << 2,  // 屏幕反色
  FLAG_BGR          = 1 << 3,  // BGR 颜色顺序（大多数 ST7735S）
};

struct Settings {
  uint32_t magic;
  uint16_t version;
  uint8_t  brightness;   // 5..100
  uint8_t  flags;
  uint8_t  rotation;     // 1 / 3（横屏 / 横屏翻转）
  uint8_t  offsetX;      // 0..8
  uint8_t  offsetY;      // 0..8
  uint8_t  refresh;      // 0=标准 1=流畅 2=极速
  char     wifiSsid[33];
  char     wifiPass[65];
  char     tz[64];
  char     ntp1[64];
  char     ntp2[64];
};

extern Settings gSettings;

void settingsBegin();
bool settingsSave(const Settings &s);
void settingsReset();
void settingsDefaults(Settings &s);
void settingsSetString(char *dst, size_t dstSize, const char *src);

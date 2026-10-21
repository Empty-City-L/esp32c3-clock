/**
 * weather.h — 心知天气（Seniverse）v3 数据模块
 *
 * 配置保存在独立 NVS 命名空间 "weather"，不会影响现有 WiFi/时钟配置。
 */
#pragma once

#include <Arduino.h>

enum WeatherIcon : uint8_t {
  WX_SUN = 0,
  WX_PARTLY,
  WX_CLOUD,
  WX_RAIN,
  WX_THUNDER,
  WX_SNOW,
  WX_FOG,
  WX_UNKNOWN,
};

struct WeatherConfig {
  uint32_t magic;
  uint16_t version;
  bool enabled;
  uint8_t updateMin;   // 5..180 分钟
  uint8_t pageSec;     // 5..60 秒
  char apiKey[48];
  char location[32];   // 心知 location：beijing / 城市 ID / 中文城市名
};

struct ForecastDay {
  bool valid;
  int8_t high;
  int8_t low;
  WeatherIcon icon;
  char text[32];
};

struct WeatherData {
  bool valid;
  bool updating;
  char city[32];
  char text[32];
  int8_t temp;
  int8_t high;
  int8_t low;
  uint8_t humidity;
  char wind[24];
  WeatherIcon icon;
  ForecastDay forecast[3];
  uint8_t forecastDays;
  uint32_t updatedMs;
  char error[48];
};

void weatherBegin();

/* 后台任务可能正在更新数据，以下接口返回加锁后的快照。 */
WeatherConfig weatherConfig();
WeatherData weatherData();

bool weatherSetConfig(const WeatherConfig &cfg);
bool weatherKeySet();
void weatherReset();

/* 3 天天气布局：0 = A 独立预报页；1 = B 合并到天气页 */
uint8_t weatherLayout();
bool weatherSetLayout(uint8_t layout);

void weatherLoop();
void weatherRequestUpdate();

/* 距离上次成功更新的秒数；未成功过返回 0 */
uint32_t weatherAgeSec();

/* 天气代码 -> 图标 / 固定中文文案 */
WeatherIcon weatherIconFromCode(int code);
const char *weatherIconSymbol(WeatherIcon icon);
const char *weatherTextFromCode(int code, const char *fallback);

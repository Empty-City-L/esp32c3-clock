#include "weather.h"

#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <Preferences.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <atomic>
#include <ctype.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <string.h>

#include "board_config.h"

namespace {

constexpr uint32_t kMagic = 0x57584333UL;  // "WXC3"
constexpr uint16_t kVersion = 1;

Preferences prefs;
WeatherConfig gCfg;
WeatherData gData;
SemaphoreHandle_t gDataLock = nullptr;
SemaphoreHandle_t gFetchSignal = nullptr;
std::atomic<bool> gFetching{false};
std::atomic<uint32_t> gGeneration{0};
uint8_t gLayout = 0;
std::atomic<bool> gForce{false};
std::atomic<uint32_t> gNextFetch{0};

void lockData() {
  if (gDataLock) xSemaphoreTake(gDataLock, portMAX_DELAY);
}

void unlockData() {
  if (gDataLock) xSemaphoreGive(gDataLock);
}

void copyStr(char *dst, size_t n, const char *src) {
  if (!dst || n == 0) return;
  if (!src) src = "";
  strncpy(dst, src, n - 1);
  dst[n - 1] = '\0';
}

void setError(const char *msg) {
  lockData();
  copyStr(gData.error, sizeof(gData.error), msg);
  gData.updating = false;
  unlockData();
}

String urlEncode(const char *s) {
  String out;
  if (!s) return out;
  for (const unsigned char *p = (const unsigned char *)s; *p; ++p) {
    if (isalnum(*p) || *p == '-' || *p == '_' || *p == '.' || *p == '~') {
      out += (char)*p;
    } else {
      char buf[5];
      snprintf(buf, sizeof(buf), "%%%02X", *p);
      out += buf;
    }
  }
  return out;
}

bool httpGet(const String &url, String &payload, String &err) {
  WiFiClient client;
  HTTPClient http;
  http.setTimeout(6000);
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  http.setUserAgent("ESP32-C3-Clock/1.0");
  if (!http.begin(client, url)) {
    err = "连接失败";
    return false;
  }
  http.addHeader("Accept-Encoding", "identity");
  const int code = http.GET();
  if (code != HTTP_CODE_OK) {
    // 心知用 400/401/403/429 表示城市、Key 或额度问题；给页面可读提示。
    if (code == 400) err = "城市或参数错误";
    else if (code == 401 || code == 403) err = "Key 无效、无权限或额度用尽";
    else if (code == 429) err = "请求过于频繁，稍后再试";
    else err = String("HTTP ") + code;
    http.end();
    return false;
  }
  payload = http.getString();
  http.end();
  return true;
}

WeatherIcon iconFromText(const char *text) {
  if (!text || !text[0]) return WX_UNKNOWN;
  if (strstr(text, "雷")) return WX_THUNDER;
  if (strstr(text, "冰雹") || strstr(text, "雪")) return WX_SNOW;
  if (strstr(text, "雨")) return WX_RAIN;
  if (strstr(text, "雾") || strstr(text, "霾") || strstr(text, "沙") ||
      strstr(text, "尘")) {
    return WX_FOG;
  }
  if (strstr(text, "多云")) return WX_PARTLY;
  if (strstr(text, "阴")) return WX_CLOUD;
  if (strstr(text, "晴")) return WX_SUN;
  return WX_UNKNOWN;
}

bool parseNow(const String &payload, WeatherData &d, const WeatherConfig &cfg,
              String &err) {
  DynamicJsonDocument doc(4096);
  DeserializationError e = deserializeJson(doc, payload);
  if (e) {
    err = String("JSON ") + e.c_str();
    return false;
  }
  JsonObject r = doc["results"][0];
  if (r.isNull()) {
    err = "API 无结果";
    return false;
  }

  JsonObject loc = r["location"];
  JsonObject now = r["now"];
  const char *name = loc["name"] | "";
  const char *apiText = now["text"] | "";
  const int code = atoi(now["code"] | "99");

  copyStr(d.city, sizeof(d.city), name[0] ? name : cfg.location);
  d.temp = (int8_t)constrain(atoi(now["temperature"] | "0"), -60, 60);
  d.humidity = (uint8_t)constrain(atoi(now["humidity"] | "0"), 0, 100);
  // 优先显示心知返回的 now.text。不同版本/城市代码表可能不一致，
  // 代码映射只作为 text 缺失时的兜底，避免“多云”被代码表误显示成雷阵雨。
  copyStr(d.text, sizeof(d.text),
          apiText[0] ? apiText : weatherTextFromCode(code, "未知"));
  const WeatherIcon textIcon = iconFromText(d.text);
  d.icon = (textIcon == WX_UNKNOWN) ? weatherIconFromCode(code) : textIcon;

  char wind[24];
  const char *dir = now["wind_direction"] | "";
  const char *scale = now["wind_scale"] | "";
  if (dir[0]) {
    if (scale[0]) snprintf(wind, sizeof(wind), "%s风 %s级", dir, scale);
    else snprintf(wind, sizeof(wind), "%s风", dir);
  } else {
    copyStr(wind, sizeof(wind), "风力 --");
  }
  copyStr(d.wind, sizeof(d.wind), wind);
  return true;
}

void parseDaily(const String &payload, WeatherData &d) {
  DynamicJsonDocument doc(6144);
  if (deserializeJson(doc, payload)) return;
  JsonArray arr = doc["results"][0]["daily"];
  if (arr.isNull()) return;

  d.forecastDays = 0;
  for (uint8_t i = 0; i < 3 && i < arr.size(); ++i) {
    JsonObject day = arr[i];
    if (day.isNull()) continue;

    ForecastDay &f = d.forecast[i];
    f.valid = true;
    f.high = (int8_t)constrain(atoi(day["high"] | "0"), -60, 60);
    f.low = (int8_t)constrain(atoi(day["low"] | "0"), -60, 60);
    const int code = atoi(day["code_day"] | "99");
    const char *text = day["text_day"] | "";
    copyStr(f.text, sizeof(f.text),
            text[0] ? text : weatherTextFromCode(code, "未知"));
    const WeatherIcon ti = iconFromText(f.text);
    f.icon = (ti == WX_UNKNOWN) ? weatherIconFromCode(code) : ti;

    if (i == 0) {
      d.high = f.high;
      d.low = f.low;
      if (d.humidity == 0) {
        d.humidity = (uint8_t)constrain(atoi(day["humidity"] | "0"), 0, 100);
      }
      const char *dir = day["wind_direction"] | "";
      const char *scale = day["wind_scale"] | "";
      if (dir[0]) {
        char wind[24];
        if (scale[0]) snprintf(wind, sizeof(wind), "%s风 %s级", dir, scale);
        else snprintf(wind, sizeof(wind), "%s风", dir);
        copyStr(d.wind, sizeof(d.wind), wind);
      }
    }
    d.forecastDays = (uint8_t)(i + 1);
  }
}

void commitData(const WeatherData &d) {
  lockData();
  gData = d;
  unlockData();
}

void fetchWeather() {
  WeatherConfig cfg;
  WeatherData d;
  lockData();
  cfg = gCfg;
  d = gData;
  // 立即把“更新中”暴露给界面；最终结果再整体提交。
  gData.updating = true;
  gData.error[0] = '\0';
  unlockData();

  const uint32_t generation = gGeneration.load();
  d.updating = true;
  d.error[0] = '\0';

  const String location = urlEncode(cfg.location);
  const String key = urlEncode(cfg.apiKey);
  const String base = "http://api.seniverse.com/v3/weather/";
  String body;
  String err;

  const String nowUrl = base + "now.json?key=" + key + "&location=" + location +
                        "&language=zh-Hans&unit=c";
  if (!httpGet(nowUrl, body, err)) {
    copyStr(d.error, sizeof(d.error), err.c_str());
    d.updating = false;
    if (generation == gGeneration.load()) commitData(d);
    Serial.printf("[weather] now failed: %s\n", d.error);
    return;
  }
  if (!parseNow(body, d, cfg, err)) {
    copyStr(d.error, sizeof(d.error), err.c_str());
    d.updating = false;
    if (generation == gGeneration.load()) commitData(d);
    Serial.printf("[weather] now parse: %s\n", d.error);
    return;
  }

  d.forecastDays = 0;
  memset(d.forecast, 0, sizeof(d.forecast));

  const String dailyUrl = base + "daily.json?key=" + key + "&location=" + location +
                          "&language=zh-Hans&unit=c&start=0&days=3";
  if (httpGet(dailyUrl, body, err)) {
    parseDaily(body, d);
  } else {
    // 实况成功、预报失败时仍显示实况；最高/最低退化为当前温度。
    d.high = d.temp;
    d.low = d.temp;
    Serial.printf("[weather] daily failed: %s\n", err.c_str());
  }

  d.valid = true;
  d.updating = false;
  d.updatedMs = millis();
  d.error[0] = '\0';
  if (generation != gGeneration.load()) {
    Serial.println("[weather] config changed, discard stale result");
    return;
  }
  commitData(d);
  Serial.printf("[weather] ok: %s %dC %s, high=%d low=%d hum=%u\n", d.city,
                d.temp, d.text, d.high, d.low, d.humidity);
}

void fetchTask(void *) {
  for (;;) {
    if (xSemaphoreTake(gFetchSignal, portMAX_DELAY) == pdTRUE) {
      fetchWeather();
      gFetching.store(false);
    }
  }
}

void sanitize(WeatherConfig &c) {
  if (c.updateMin < 5 || c.updateMin > 180) c.updateMin = 15;
  if (c.pageSec < 5 || c.pageSec > 60) c.pageSec = 10;
  c.apiKey[sizeof(c.apiKey) - 1] = '\0';
  c.location[sizeof(c.location) - 1] = '\0';
  if (c.location[0] == '\0') copyStr(c.location, sizeof(c.location), "beijing");
}

void defaults(WeatherConfig &c) {
  memset(&c, 0, sizeof(c));
  c.magic = kMagic;
  c.version = kVersion;
  c.enabled = false;
  c.updateMin = 15;
  c.pageSec = 10;
  copyStr(c.location, sizeof(c.location), "beijing");
}

bool saveCfg(const WeatherConfig &cfg) {
  return prefs.putBytes("cfg", &cfg, sizeof(cfg)) == sizeof(cfg);
}

}  // namespace

WeatherIcon weatherIconFromCode(int code) {
  if (code == 0) return WX_SUN;
  if (code == 1) return WX_PARTLY;
  if (code == 2) return WX_CLOUD;
  if (code == 4) return WX_THUNDER;
  if (code == 5 || code == 6 || (code >= 13 && code <= 17) ||
      (code >= 26 && code <= 28)) {
    return WX_SNOW;
  }
  if ((code >= 3 && code <= 12) || code == 19 || (code >= 21 && code <= 25)) {
    return WX_RAIN;
  }
  if (code == 18 || (code >= 29 && code <= 38)) return WX_FOG;
  return WX_UNKNOWN;
}

const char *weatherIconSymbol(WeatherIcon icon) {
  switch (icon) {
    case WX_SUN: return "☀";
    case WX_PARTLY: return "⛅";
    case WX_CLOUD: return "☁";
    case WX_RAIN: return "☔";
    case WX_THUNDER: return "⚡";
    case WX_SNOW: return "❄";
    case WX_FOG: return "≋";
    default: return "?";
  }
}

const char *weatherTextFromCode(int code, const char *fallback) {
  switch (code) {
    case 0: return "晴";
    case 1: return "多云";
    case 2: return "阴";
    case 3: return "阵雨";
    case 4: return "雷阵雨";
    case 5: return "冰雹";
    case 6: return "雨夹雪";
    case 7: return "小雨";
    case 8: return "中雨";
    case 9: return "大雨";
    case 10: return "暴雨";
    case 11: return "大暴雨";
    case 12: return "特大暴雨";
    case 13: return "阵雪";
    case 14: return "小雪";
    case 15: return "中雪";
    case 16: return "大雪";
    case 17: return "暴雪";
    case 18: return "雾";
    case 19: return "冻雨";
    case 20: return "沙尘暴";
    case 21: return "小到中雨";
    case 22: return "中到大雨";
    case 23: return "大到暴雨";
    case 24: return "暴雨到大暴雨";
    case 25: return "大暴雨到特大暴雨";
    case 26: return "小到中雪";
    case 27: return "中到大雪";
    case 28: return "大到暴雪";
    case 29: return "浮尘";
    case 30: return "扬沙";
    case 31: return "强沙尘暴";
    case 32: return "浓雾";
    case 33: return "强浓雾";
    case 34: return "中度霾";
    case 35: return "重度霾";
    case 36: return "严重霾";
    case 37: return "大雾";
    case 38: return "特强浓雾";
    default: return (fallback && fallback[0]) ? fallback : "未知";
  }
}

void weatherBegin() {
  gDataLock = xSemaphoreCreateMutex();
  gFetchSignal = xSemaphoreCreateBinary();

  lockData();
  memset(&gData, 0, sizeof(gData));
  gData.text[0] = '-';
  copyStr(gData.error, sizeof(gData.error), "未配置天气");
  unlockData();

  prefs.begin("weather", false);
  WeatherConfig c;
  size_t n = prefs.getBytes("cfg", &c, sizeof(c));
  if (n != sizeof(c) || c.magic != kMagic || c.version != kVersion) {
    defaults(gCfg);
    saveCfg(gCfg);
    Serial.println("[weather] defaults written");
  } else {
    gCfg = c;
  }
  sanitize(gCfg);
  gLayout = prefs.getUChar("layout", 0);
  if (gLayout > 1) gLayout = 0;
  Serial.printf("[weather] enabled=%d location='%s' key=%s update=%umin page=%us layout=%u\n",
                gCfg.enabled, gCfg.location, gCfg.apiKey[0] ? "set" : "empty",
                gCfg.updateMin, gCfg.pageSec, gLayout);

  if (gDataLock && gFetchSignal) {
    xTaskCreate(fetchTask, "weather", 8192, nullptr, 1, nullptr);
  }
}

WeatherConfig weatherConfig() {
  lockData();
  WeatherConfig c = gCfg;
  unlockData();
  return c;
}

WeatherData weatherData() {
  lockData();
  WeatherData d = gData;
  unlockData();
  return d;
}

bool weatherKeySet() {
  lockData();
  const bool set = gCfg.apiKey[0] != '\0';
  unlockData();
  return set;
}

uint8_t weatherLayout() { return gLayout; }

bool weatherSetLayout(uint8_t layout) {
  gLayout = layout > 1 ? 0 : layout;
  return prefs.putUChar("layout", gLayout) == 1;
}

bool weatherSetConfig(const WeatherConfig &cfg) {
  WeatherConfig c = cfg;
  c.magic = kMagic;
  c.version = kVersion;
  sanitize(c);

  lockData();
  const bool identityChanged =
      strcmp(c.apiKey, gCfg.apiKey) != 0 || strcmp(c.location, gCfg.location) != 0;
  gCfg = c;
  if (identityChanged) {
    gData.valid = false;
    gData.updating = false;
    copyStr(gData.error, sizeof(gData.error), "等待更新");
  }
  if (!gCfg.enabled) {
    gData.updating = false;
    copyStr(gData.error, sizeof(gData.error), "天气已关闭");
  }
  unlockData();

  // 配置变更后，后台任务正在进行的旧请求结果作废。
  gGeneration.fetch_add(1);
  gNextFetch.store(0);

  const bool ok = saveCfg(c);
  if (c.enabled && c.apiKey[0]) weatherRequestUpdate();
  return ok;
}

void weatherRequestUpdate() {
  gForce.store(true);
  gNextFetch.store(0);
}

void weatherReset() {
  WeatherConfig c;
  defaults(c);
  saveCfg(c);

  lockData();
  gCfg = c;
  memset(&gData, 0, sizeof(gData));
  gData.text[0] = '-';
  copyStr(gData.error, sizeof(gData.error), "未配置天气");
  unlockData();

  gGeneration.fetch_add(1);
  gForce.store(false);
  gNextFetch.store(0);
  gLayout = 0;
  prefs.putUChar("layout", 0);
}

uint32_t weatherAgeSec() {
  lockData();
  const bool valid = gData.valid;
  const uint32_t updatedMs = gData.updatedMs;
  unlockData();
  if (!valid) return 0;
  return (millis() - updatedMs) / 1000;
}

void weatherLoop() {
  const WeatherConfig cfg = weatherConfig();
  if (!cfg.enabled) return;
  if (cfg.apiKey[0] == '\0') {
    setError("未配置心知 Key");
    return;
  }
  if (WiFi.status() != WL_CONNECTED) {
    setError("WiFi 未连接");
    return;
  }
  if (gFetching.load()) return;

  const uint32_t now = millis();
  const uint32_t next = gNextFetch.load();
  if (!gForce.load() && next != 0 && (int32_t)(now - next) < 0) return;

  gForce.store(false);
  gNextFetch.store(now + (uint32_t)cfg.updateMin * 60000UL);
  gFetching.store(true);
  // 信号量已有待处理请求时，任务同样会使用最新配置发起请求。
  xSemaphoreGive(gFetchSignal);
}

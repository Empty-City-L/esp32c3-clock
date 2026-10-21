/**
 * ESP32-C3 + 1.8" ST7735S (160x128 横屏) + LVGL 8 桌面时钟
 *
 * 上电流程：
 *   1. 有已保存的 Wi-Fi -> 连接 -> NTP 对时 -> 显示时钟
 *   2. 没有 Wi-Fi 或 20 秒连不上 -> 开启配网热点 ESP32-Clock-XXXX
 *      手机连接热点后访问 http://192.168.4.1 配置
 *   3. 配网/设置页保存后自动重启
 *
 * 断网后时间继续走（ESP32 内部 RTC），底部状态变黄并自动重连；
 * 掉线超过 30 秒会重新开启配网热点，方便修改 Wi-Fi。
 */
#include <Arduino.h>
#include <ESPmDNS.h>
#include <WiFi.h>
#include <esp_heap_caps.h>
#include <esp_mac.h>
#include <lvgl.h>
#include <stdlib.h>
#include <time.h>

#include "board_config.h"
#include "pages.h"
#include "photo.h"
#include "settings.h"
#include "ui.h"
#include "weather.h"
#include "web_config.h"

#define LGFX_USE_V1
#include <LovyanGFX.hpp>

/* ============================ 屏幕驱动 ============================ */

/* 刷新档位：0=标准 10MHz/30ms，1=流畅 20MHz/20ms，2=极速 40MHz/16ms */
static uint32_t lcdSpiHz() {
  if (gSettings.refresh == 2) return 40000000UL;
  if (gSettings.refresh == 1) return 20000000UL;
  return LCD_SPI_HZ;
}

static uint32_t lvglRefrMs() {
  if (gSettings.refresh == 2) return 16;
  if (gSettings.refresh == 1) return 20;
  return 30;
}

class LGFX : public lgfx::LGFX_Device {
  lgfx::Panel_ST7735S _panel;
  lgfx::Bus_SPI _bus;

 public:
  void configure() {
    {
      auto cfg = _bus.config();
      cfg.spi_host = SPI2_HOST;      // ESP32-C3 只有 SPI2
      cfg.spi_mode = 0;
      cfg.freq_write = lcdSpiHz();
      cfg.freq_read = 16000000;
      cfg.spi_3wire = true;          // 屏幕没有 MISO
      cfg.use_lock = true;
      cfg.dma_channel = SPI_DMA_CH_AUTO;
      cfg.pin_sclk = PIN_LCD_SCLK;
      cfg.pin_mosi = PIN_LCD_MOSI;
      cfg.pin_miso = -1;
      cfg.pin_dc = PIN_LCD_DC;
      _bus.config(cfg);
      _panel.setBus(&_bus);
    }
    {
      auto cfg = _panel.config();
      cfg.pin_cs = PIN_LCD_CS;
      cfg.pin_rst = PIN_LCD_RST;
      cfg.pin_busy = -1;
      cfg.panel_width = LCD_PANEL_W;
      cfg.panel_height = LCD_PANEL_H;
      cfg.memory_width = LCD_MEMORY_W;
      cfg.memory_height = LCD_MEMORY_H;
      cfg.offset_x = gSettings.offsetX;
      cfg.offset_y = gSettings.offsetY;
      cfg.offset_rotation = 0;
      cfg.readable = false;
      cfg.invert = (gSettings.flags & FLAG_INVERT) != 0;
      // LovyanGFX 的 rgb_order: true = RGB, false = BGR。
      // 我们的选项叫 FLAG_BGR，所以这里要取反。
      cfg.rgb_order = (gSettings.flags & FLAG_BGR) == 0;
      cfg.dlen_16bit = false;
      cfg.bus_shared = true;
      _panel.config(cfg);
    }
    setPanel(&_panel);
  }

  /**
   * 覆盖 LovyanGFX 的 MADCTL。
   *
   * 实机面板探测结果：rotation 1 + MADCTL 0xA0（MY|MV）时，
   * 白框正、四角颜色和 TOP/BOTTOM 都正确。LovyanGFX 默认的
   * 0x64（MX|MV|MH）在这块屏上会剪切/镜像。
   */
  void forceMadctl() {
    // 面板固定使用 rotation 1 + 0xA0；翻转在 flush 里做软件 180° 旋转。
    uint8_t mad = 0xA0;  // MY|MV
    if (gSettings.flags & FLAG_BGR) mad |= 0x08;
    writeCommand(0x36);  // MADCTL
    writeData(mad);
  }
};

static LGFX lcd;

/* ============================ 背光 PWM ============================ */
static void applyBrightness(uint8_t percent) {
  percent = constrain(percent, (uint8_t)5, (uint8_t)100);
  const uint32_t duty = (uint32_t)percent * 255U / 100U;
  ledcWrite(LCD_BL_CHANNEL, duty);
}

static void initBacklight() {
  ledcSetup(LCD_BL_CHANNEL, LCD_BL_FREQ, LCD_BL_BITS);
  ledcAttachPin(PIN_LCD_BL, LCD_BL_CHANNEL);
  applyBrightness(gSettings.brightness);
}

/* ============================ LVGL 缓冲 ============================ */
/* ST7735S 对局部刷新区域可能比较敏感；使用整屏缓冲 + 整屏刷新，
   避免局部窗口错位/残影。160x128x2 = 40 KB。 */
static lv_color_t *draw_buf1 = nullptr;
static lv_disp_draw_buf_t draw_buf;
static lv_disp_drv_t disp_drv;

/* 给 /api/screenshot.raw 用的当前帧缓冲（RGB565，160x128）。 */
const uint8_t *appScreenBuffer(size_t *len) {
  if (!draw_buf1) {
    if (len) *len = 0;
    return nullptr;
  }
  if (len) *len = (size_t)SCREEN_W * SCREEN_H * 2;
  return reinterpret_cast<const uint8_t *>(draw_buf1);
}

static void disp_flush(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_p) {
  (void)area;
  (void)color_p;

  /* full_refresh = 1 时，LVGL 会把整帧画进 draw_buf1。
     - 逐行设置 160x1 窗口发送，避免 ST7735S 长块写入行错位。
     - 方向 3 时在软件里做 180° 旋转，不依赖面板的 MX/MY 组合。 */
  static uint16_t flip_row[SCREEN_W];
  const bool flip = (gSettings.rotation == 3);
  uint16_t *src = reinterpret_cast<uint16_t *>(draw_buf1);

  lcd.startWrite();
  for (uint32_t y = 0; y < SCREEN_H; ++y) {
    const uint16_t *row = src + (flip ? (SCREEN_H - 1 - y) * SCREEN_W : y * SCREEN_W);
    if (flip) {
      for (uint32_t x = 0; x < SCREEN_W; ++x) {
        flip_row[x] = row[SCREEN_W - 1 - x];
      }
      row = flip_row;
    }
    lcd.setWindow(0, y, SCREEN_W - 1, y);
    lcd.writePixels(reinterpret_cast<const lgfx::rgb565_t *>(row), SCREEN_W);
  }
  lcd.endWrite();

  lv_disp_flush_ready(drv);
}

/* ============================ 网络状态 ============================ */
enum NetState { NET_CONNECTING, NET_SYNCING, NET_RUNNING, NET_AP_ONLY };

static NetState state = NET_CONNECTING;
static bool ap_active = false;

enum UiPage { PAGE_CLOCK, PAGE_WEATHER, PAGE_FORECAST, PAGE_PHOTO };

static UiPage page = PAGE_CLOCK;
static uint32_t page_since = 0;
static uint32_t last_weather_update = 0;
static uint8_t photo_frame = 0;
static uint32_t photo_next_ms = 0;
static uint32_t photo_force_until = 0;
static uint16_t *photo_buf = nullptr;
static char photo_debug[48] = "idle";
static char page_debug[16] = "clock";

const char *appPhotoDebug() { return photo_debug; }
const char *appPageDebug() { return page_debug; }
static bool web_started = false;
static bool mdns_started = false;
static bool warn_shown = false;
static bool was_online = false;
static uint8_t ntp_retries = 0;
static uint32_t state_since = 0;
static uint32_t last_retry = 0;
static uint32_t last_resync = 0;
static uint32_t sta_connected_at = 0;
static uint32_t sta_lost_at = 0;
static char ap_ssid[33] = "";

static bool wifi_configured() { return gSettings.wifiSsid[0] != '\0'; }
static bool time_ok(time_t t) { return t > 1700000000; }  // 2023-11 之后才算对时成功
static bool sta_connected() { return WiFi.status() == WL_CONNECTED; }

/* 即使还没连 WiFi，也要先应用时区：ESP32 的 RTC 可能在复位后保留 UTC 时间。 */
static void applyTimezone() {
  setenv("TZ", gSettings.tz, 1);
  tzset();
}

static void startSntp() {
  configTzTime(gSettings.tz, gSettings.ntp1, gSettings.ntp2, NTP_DEFAULT_3);
  Serial.printf("[ntp] tz=%s server=%s / %s\n", gSettings.tz, gSettings.ntp1, gSettings.ntp2);
}

static void buildApSsid() {
  uint8_t mac[6] = {0};
  // 在 WiFi 初始化前也能读到出厂 MAC，避免热点名变成 0000。
  if (esp_read_mac(mac, ESP_MAC_WIFI_STA) != ESP_OK) {
    WiFi.macAddress(mac);
  }
  snprintf(ap_ssid, sizeof(ap_ssid), AP_SSID_PREFIX "%02X%02X", mac[4], mac[5]);
}

static void startWeb() {
  if (!web_started) {
    webBegin(ap_active, ap_ssid);
    web_started = true;
  } else {
    webSetApMode(ap_active);
  }
}

static void startMdns() {
  if (mdns_started || !sta_connected()) return;
  if (MDNS.begin(MDNS_HOST)) {
    MDNS.addService("http", "tcp", 80);
    mdns_started = true;
    Serial.printf("[mdns] http://%s.local/\n", MDNS_HOST);
  } else {
    Serial.println("[mdns] begin failed");
  }
}

static void startAp(bool showSplash = true) {
  if (ap_active) return;

  Serial.printf("[wifi] starting config AP: %s\n", ap_ssid);
  WiFi.mode(wifi_configured() ? WIFI_AP_STA : WIFI_AP);
  WiFi.softAP(ap_ssid);
  delay(150);

  ap_active = true;
  startWeb();
  if (showSplash) {
    ui::splash("配网模式", ap_ssid, "浏览器访问", "192.168.4.1");
  }
  Serial.printf("[wifi] AP IP: %s\n", WiFi.softAPIP().toString().c_str());
}

static void stopAp() {
  if (!ap_active) return;
  WiFi.softAPdisconnect(true);
  if (sta_connected()) WiFi.mode(WIFI_STA);
  ap_active = false;
  webSetApMode(false);
  Serial.println("[wifi] config AP stopped");
}

static void onStaConnected() {
  const uint32_t now = millis();
  Serial.printf("[wifi] connected, IP: %s, RSSI: %d\n",
                WiFi.localIP().toString().c_str(), WiFi.RSSI());

  last_retry = now;
  sta_connected_at = now;
  was_online = true;
  ntp_retries = 0;
  startMdns();
  startWeb();
  startSntp();
  state_since = now;
  last_resync = now;

  if (time_ok(time(nullptr))) {
    state = NET_RUNNING;
    page = PAGE_CLOCK;
    page_since = now;
    photo_frame = 0;
    ui::show_clock();
    ui::set_link(ui::Link::Online);
  } else {
    state = NET_SYNCING;
    ui::splash("正在同步时间", "NTP 对时中");
  }
}

static void startSta() {
  WiFi.mode(WIFI_STA);
  WiFi.setHostname(MDNS_HOST);
  WiFi.setSleep(false);
  WiFi.setAutoReconnect(true);
  WiFi.begin(gSettings.wifiSsid, gSettings.wifiPass);

  state = NET_CONNECTING;
  state_since = millis();
  last_retry = millis();
  warn_shown = false;

  Serial.printf("[wifi] connecting to '%s' ...\n", gSettings.wifiSsid);
  ui::splash("正在连接 WiFi", gSettings.wifiSsid, "请稍候");
}

static void networkLoop(uint32_t now) {
  switch (state) {
    case NET_CONNECTING: {
      if (sta_connected()) {
        onStaConnected();
        break;
      }
      if (!warn_shown && now - state_since > 12000) {
        warn_shown = true;
        ui::splash("连接失败", "正在重试", "请检查密码或信号");
      }
      if (now - state_since > WIFI_CONNECT_TIMEOUT_MS && !ap_active) {
        startAp();  // 保留 STA 重连，同时开热点方便改配置
      }
      if (now - last_retry > WIFI_RETRY_MS) {
        last_retry = now;
        Serial.println("[wifi] retry...");
        WiFi.reconnect();
      }
      break;
    }

    case NET_SYNCING: {
      if (!sta_connected()) {
        // 对时途中掉线：回到连接流程，超时后会开配置热点。
        Serial.println("[wifi] lost while syncing");
        state = NET_CONNECTING;
        state_since = now;
        last_retry = now;
        warn_shown = false;
        ui::splash("正在连接 WiFi", gSettings.wifiSsid, "请稍候");
        break;
      }
      if (time_ok(time(nullptr))) {
        Serial.println("[ntp] synced");
        state = NET_RUNNING;
        last_resync = now;
        page = PAGE_CLOCK;
        page_since = now;
        photo_frame = 0;
        ui::show_clock();
        ui::set_link(ui::Link::Online);
      } else if (now - state_since > 20000) {
        state_since = now;
        Serial.println("[ntp] retry...");
        startSntp();
        if (++ntp_retries >= 2) {
          ui::splash("同步失败", "正在重试", "请检查 NTP 服务器");
        }
      }
      break;
    }

    case NET_RUNNING: {
      if (sta_connected()) {
        if (!was_online) {  // 刚掉线又恢复：立刻补一次对时
          was_online = true;
          startSntp();
          last_resync = now;
        }
        ui::set_link(ui::Link::Online);
        if (now - last_resync > 6UL * 3600UL * 1000UL) {
          last_resync = now;
          startSntp();
        }
        if (ap_active && now - sta_connected_at > AP_KEEP_AFTER_STA_MS &&
            WiFi.softAPgetStationNum() == 0) {
          stopAp();  // 还有手机连着热点时先不关，避免配网页突然断线
        }
      } else {
        if (was_online) {
          was_online = false;
          sta_lost_at = now;
          ui::set_link(ui::Link::Offline);
          Serial.println("[wifi] lost, using RTC");
        }
        if (now - last_retry > WIFI_RETRY_MS) {
          last_retry = now;
          Serial.println("[wifi] reconnect...");
          WiFi.reconnect();
        }
        if (!ap_active && now - sta_lost_at > 30000) {
          startAp(false);  // 掉线太久：开热点，但保持时钟画面
        }
      }
      break;
    }

    case NET_AP_ONLY:
      // 没有保存过 Wi-Fi，等待用户在配置页填写；保存后设备会重启。
      break;
  }
}

static bool canWeatherPage() {
  const WeatherConfig &wc = weatherConfig();
  const WeatherData &wd = weatherData();
  return wc.enabled && wc.apiKey[0] && wd.valid;
}

static bool canForecastPage() {
  return canWeatherPage() && weatherLayout() == 0 &&
         weatherData().forecastDays >= 3;
}

static bool canPhotoPage() { return photoCount() > 0; }

static bool showPhotoFrame(uint8_t index) {
  if (!photo_buf) {
    photo_buf = static_cast<uint16_t *>(
        heap_caps_malloc((size_t)SCREEN_W * SCREEN_H * 2, MALLOC_CAP_DMA));
    if (!photo_buf) {
      strncpy(photo_debug, "buffer alloc fail", sizeof(photo_debug) - 1);
      Serial.println("[photo] buffer allocation failed");
      return false;
    }
    Serial.printf("[photo] buffer allocated, free heap: %u B\n", (unsigned)ESP.getFreeHeap());
  }

  if (!photoReadFrame(index, photo_buf, (size_t)SCREEN_W * SCREEN_H)) {
    strncpy(photo_debug, "read fail", sizeof(photo_debug) - 1);
    Serial.printf("[photo] read failed: frame %u\n", index);
    return false;
  }
  strncpy(photo_debug, "shown", sizeof(photo_debug) - 1);
  ui::show_photo(photo_buf);
  return true;
}

static uint8_t pageIdOf(UiPage p) {
  switch (p) {
    case PAGE_WEATHER: return PAGE_ID_WEATHER;
    case PAGE_FORECAST: return PAGE_ID_FORECAST;
    case PAGE_PHOTO: return PAGE_ID_PHOTO;
    case PAGE_CLOCK:
    default: return PAGE_ID_CLOCK;
  }
}

/* 数据是否可用；不管网页上的显示开关。 */
static bool pageUsable(UiPage p) {
  switch (p) {
    case PAGE_WEATHER: return canWeatherPage();
    case PAGE_FORECAST: return canForecastPage();
    case PAGE_PHOTO: return canPhotoPage();
    case PAGE_CLOCK:
    default: return true;
  }
}

static size_t buildPageList(UiPage out[PAGE_COUNT]) {
  size_t n = 0;
  for (uint8_t pos = 0; pos < PAGE_COUNT; ++pos) {
    for (uint8_t id = 0; id < PAGE_COUNT; ++id) {
      if (pageOrder(id) != pos) continue;
      if (!pageVisible(id)) continue;
      UiPage p = (UiPage)id;
      if (!pageUsable(p)) continue;
      out[n++] = p;
    }
  }
  // 全隐藏或全不可用时，至少保留时钟，避免黑屏。
  if (n == 0) out[n++] = PAGE_CLOCK;
  return n;
}

static UiPage nextPage(UiPage current) {
  UiPage list[PAGE_COUNT];
  const size_t n = buildPageList(list);
  for (size_t i = 0; i < n; ++i) {
    if (list[i] == current) return list[(i + 1) % n];
  }
  return list[0];
}

static bool residentUiPage(UiPage &out) {
  const uint8_t r = residentPage();
  UiPage p = PAGE_CLOCK;
  switch (r) {
    case 1: p = PAGE_CLOCK; break;
    case 2: p = PAGE_WEATHER; break;
    case 3: p = PAGE_FORECAST; break;
    case 4: p = PAGE_PHOTO; break;
    default: return false;
  }
  if (!pageUsable(p)) return false;
  out = p;
  return true;
}

static void enterPage(UiPage p) {
  switch (p) {
    case PAGE_WEATHER:
      strncpy(page_debug, weatherLayout() == 1 ? "weather-b" : "weather-a",
              sizeof(page_debug) - 1);
      ui::show_weather(weatherData());
      break;
    case PAGE_FORECAST:
      strncpy(page_debug, "forecast", sizeof(page_debug) - 1);
      ui::show_forecast(weatherData());
      break;
    case PAGE_PHOTO: {
      strncpy(page_debug, "photo", sizeof(page_debug) - 1);
      photo_frame = 0;
      if (!showPhotoFrame(0)) {
        page = PAGE_CLOCK;
        ui::show_clock();
        break;
      }
      photo_next_ms = millis() + (photoDelayMs() < 80 ? 80 : photoDelayMs());
      break;
    }
    case PAGE_CLOCK:
    default:
      strncpy(page_debug, "clock", sizeof(page_debug) - 1);
      ui::show_clock();
      break;
  }
}

bool appForcePhoto() {
  if (!showPhotoFrame(0)) return false;
  page = PAGE_PHOTO;
  page_since = millis();
  photo_force_until = millis() + 20000UL;
  photo_frame = 0;
  photo_next_ms = millis() + (photoDelayMs() < 80 ? 80 : photoDelayMs());
  strncpy(page_debug, "photo-force", sizeof(page_debug) - 1);
  return true;
}

/* 按网页配置的显示/顺序轮播；常驻页优先。 */
static void pageLoop(uint32_t now) {
  if (state != NET_RUNNING) return;

  const WeatherConfig &wc = weatherConfig();
  const WeatherData &wd = weatherData();

  if (wd.updatedMs != last_weather_update) {
    last_weather_update = wd.updatedMs;
    ui::set_weather(wd);
  }

  // 网页触发“预览照片”后短暂锁定照片页，避免轮播立刻切走。
  if (photo_force_until != 0 && page == PAGE_PHOTO &&
      (int32_t)(now - photo_force_until) < 0) {
    page_since = now;
    return;
  }

  UiPage resident = PAGE_CLOCK;
  if (residentUiPage(resident)) {
    if (page != resident) {
      page = resident;
      page_since = now;
      enterPage(page);
    }
    return;
  }

  UiPage list[PAGE_COUNT];
  const size_t n = buildPageList(list);
  bool found = false;
  for (size_t i = 0; i < n; ++i) {
    if (list[i] == page) {
      found = true;
      break;
    }
  }
  if (!found) {
    page = list[0];
    page_since = now;
    enterPage(page);
    return;
  }

  const uint32_t dwell = (page == PAGE_PHOTO) ? (uint32_t)photoPageSec() * 1000UL
                                              : (uint32_t)wc.pageSec * 1000UL;
  // Web 回调可能在本轮 loop 采样 now 之后更新 page_since；无符号相减会溢出。
  if ((int32_t)(now - page_since) < 0) return;
  if (now - page_since < dwell) return;

  const UiPage next = nextPage(page);
  if (next == page) {
    page_since = now;
    return;
  }

  page = next;
  page_since = now;
  enterPage(page);
}

/* 动图帧播放：只在照片页生效。 */
static void photoTick(uint32_t now) {
  if (state != NET_RUNNING || page != PAGE_PHOTO || !photo_buf) return;
  const uint8_t count = photoFrameCount();
  if (count <= 1) return;
  if ((int32_t)(now - photo_next_ms) < 0) return;

  const uint8_t next = (uint8_t)((photo_frame + 1) % count);
  if (!showPhotoFrame(next)) {
    page = PAGE_CLOCK;
    ui::show_clock();
    return;
  }
  photo_frame = next;
  uint16_t delay_ms = photoDelayMs();
  if (delay_ms < 80) delay_ms = 80;
  photo_next_ms = now + delay_ms;
}

/* ============================ UI dump ============================ */
#ifdef UI_DUMP
static void dumpUi() {
  // UI_DUMP 环境下 draw_buf1 就是整屏缓冲，直接复用。
  lv_disp_draw_buf_init(&draw_buf, draw_buf1, nullptr, SCREEN_W * SCREEN_H);
  disp_drv.draw_buf = &draw_buf;
  disp_drv.full_refresh = 1;

  struct tm t = {};
  t.tm_year = 2026 - 1900;
  t.tm_mon = 9;
  t.tm_mday = 18;
  t.tm_hour = 23;
  t.tm_min = 25;
  t.tm_sec = 38;
  t.tm_wday = 6;

  ui::show_clock();
  ui::set_link(ui::Link::Online);
  ui::set_clock(t);
  lv_refr_now(nullptr);

  Serial.flush();
  Serial.end();
  delay(50);
  Serial.begin(921600);
  delay(100);

  const char magic[8] = {'U', 'I', 'D', 'U', 'M', 'P', '1', 0};
  uint16_t w = SCREEN_W, h = SCREEN_H;
  Serial.write(reinterpret_cast<const uint8_t *>(magic), sizeof(magic));
  Serial.write(reinterpret_cast<const uint8_t *>(&w), sizeof(w));
  Serial.write(reinterpret_cast<const uint8_t *>(&h), sizeof(h));
  Serial.write(reinterpret_cast<const uint8_t *>(draw_buf1), SCREEN_W * SCREEN_H * 2);
  Serial.flush();

  for (;;) delay(1000);
}
#endif

/* ============================ setup ============================ */
void setup() {
  Serial.begin(115200);
  delay(250);
  Serial.println();
  Serial.println("=== ESP32-C3 ST7735S clock boot ===");
  Serial.printf("chip: %s rev %d, flash: %u KB, heap: %u B\n", ESP.getChipModel(),
                ESP.getChipRevision(), (unsigned)(ESP.getFlashChipSize() / 1024),
                (unsigned)ESP.getFreeHeap());

  settingsBegin();
  weatherBegin();
  photoBegin();
  pagesBegin();
  applyTimezone();

  /* 屏幕 */
  lcd.configure();
  lcd.setColorDepth(16);
  if (!lcd.init()) {
    Serial.println("[lcd] init failed");
  }
  /* 启动时先按竖屏、再按横屏各清一次全屏，
     清掉面板边缘可能残留的 GRAM 像素。 */
  lcd.setRotation(0);
  lcd.fillScreen(0x0000);
  lcd.setRotation(1);  // 这块面板固定横屏 rotation 1
  lcd.forceMadctl();   // 实机确认：rotation 1 必须用 0xA0
  lcd.fillScreen(0x0000);
  initBacklight();

  /* LVGL */
  lv_init();

  const size_t pixels = (size_t)SCREEN_W * SCREEN_H;
  draw_buf1 = static_cast<lv_color_t *>(heap_caps_malloc(pixels * sizeof(lv_color_t), MALLOC_CAP_DMA));
  if (!draw_buf1) {
    Serial.println("FATAL: full-screen draw buffer allocation failed");
    while (true) delay(1000);
  }
  lv_disp_draw_buf_init(&draw_buf, draw_buf1, nullptr, pixels);
  Serial.printf("[lvgl] full buffer: %u B, free heap: %u B\n",
                (unsigned)(pixels * sizeof(lv_color_t)), (unsigned)ESP.getFreeHeap());

  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = SCREEN_W;
  disp_drv.ver_res = SCREEN_H;
  disp_drv.flush_cb = disp_flush;
  disp_drv.draw_buf = &draw_buf;
  disp_drv.full_refresh = 1;  // 每次都整屏刷新，避免 ST7735S 局部窗口错位
  lv_disp_drv_register(&disp_drv);

  lv_timer_t *refr = _lv_disp_get_refr_timer(NULL);
  if (refr) lv_timer_set_period(refr, lvglRefrMs());

  ui::init();
#ifdef UI_DUMP
  dumpUi();
#endif

  webSetBrightnessHandler(applyBrightness);
  buildApSsid();

  if (wifi_configured()) {
    startSta();
  } else {
    state = NET_AP_ONLY;
    startAp();
  }
}

/* ============================ loop ============================ */
static uint32_t last_tick = 0;
static int last_sec = -1;

void loop() {
  const uint32_t now = millis();

  /* LVGL 心跳 + 刷新 */
  lv_tick_inc(now - last_tick);
  last_tick = now;
  lv_timer_handler();

  /* Web / DNS / 延迟重启 */
  webLoop();

  /* Wi-Fi + NTP 状态机 */
  networkLoop(now);

  /* 心知天气：NTP 完成后再拉取，避免阻塞对时（可能阻塞数秒） */
  if (state == NET_RUNNING) weatherLoop();

  /* 时钟页 / 天气页 / 照片页轮播 */
  pageLoop(now);
  photoTick(now);
  ui::tick();

  /* 每秒刷新时间 */
  const time_t t = time(nullptr);
  if (time_ok(t)) {
    struct tm tmv;
    localtime_r(&t, &tmv);
    if (tmv.tm_sec != last_sec) {
      last_sec = tmv.tm_sec;
      if (state == NET_RUNNING) {
        ui::set_clock(tmv);
      }
    }
  }

  delay(2);
}

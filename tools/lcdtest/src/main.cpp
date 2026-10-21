/**
 * 纯色测试固件（ESP32-C3 + ST7735S 160x128）
 *
 * 黑 -> 红 -> 绿 -> 蓝 -> 白，每个颜色 3 秒，循环。
 * 用来判断固定白点/坏点/边缘 GRAM 残留。
 *
 * 面板参数与主工程一致：
 *   rotation 1, MADCTL 0xA0, RGB, offset 2/1, SPI 10 MHz
 */
#include <Arduino.h>

#define LGFX_USE_V1
#include <LovyanGFX.hpp>

static constexpr int PIN_SCLK = 4;
static constexpr int PIN_MOSI = 6;
static constexpr int PIN_CS = 7;
static constexpr int PIN_DC = 2;
static constexpr int PIN_RST = 3;
static constexpr int PIN_BL = 5;

class LGFX : public lgfx::LGFX_Device {
  lgfx::Panel_ST7735S panel_;
  lgfx::Bus_SPI bus_;

 public:
  LGFX() {
    {
      auto cfg = bus_.config();
      cfg.spi_host = SPI2_HOST;
      cfg.spi_mode = 0;
      cfg.freq_write = 10000000;
      cfg.freq_read = 8000000;
      cfg.spi_3wire = true;
      cfg.use_lock = true;
      cfg.dma_channel = SPI_DMA_CH_AUTO;
      cfg.pin_sclk = PIN_SCLK;
      cfg.pin_mosi = PIN_MOSI;
      cfg.pin_miso = -1;
      cfg.pin_dc = PIN_DC;
      bus_.config(cfg);
      panel_.setBus(&bus_);
    }
    {
      auto cfg = panel_.config();
      cfg.pin_cs = PIN_CS;
      cfg.pin_rst = PIN_RST;
      cfg.pin_busy = -1;
      cfg.panel_width = 128;
      cfg.panel_height = 160;
      cfg.memory_width = 132;
      cfg.memory_height = 162;
      cfg.offset_x = 2;
      cfg.offset_y = 1;
      cfg.offset_rotation = 0;
      cfg.readable = false;
      cfg.invert = false;
      cfg.rgb_order = true;  // RGB
      cfg.dlen_16bit = false;
      cfg.bus_shared = true;
      panel_.config(cfg);
    }
    setPanel(&panel_);
  }
};

static LGFX lcd;

static void forceMadctl(uint8_t mad) {
  lcd.startWrite();
  lcd.writeCommand(0x36);  // MADCTL
  lcd.writeData(mad);
  lcd.endWrite();
}

static void backlightOn() {
  ledcSetup(0, 5000, 8);
  ledcAttachPin(PIN_BL, 0);
  ledcWrite(0, 255);
}

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println();
  Serial.println("=== ST7735S solid color test ===");

  backlightOn();
  lcd.setColorDepth(16);
  lcd.init();
  lcd.setRotation(1);
  forceMadctl(0xA0);
  lcd.fillScreen(TFT_BLACK);
}

void loop() {
  static uint32_t last = 0;
  static uint8_t idx = 0;
  if (millis() - last < 3000) {
    delay(20);
    return;
  }
  last = millis();

  static const uint16_t colors[] = {TFT_BLACK, TFT_RED, TFT_GREEN, TFT_BLUE, TFT_WHITE};
  static const char *names[] = {"BLACK", "RED", "GREEN", "BLUE", "WHITE"};

  lcd.fillScreen(colors[idx]);
  Serial.printf("fillScreen: %s\n", names[idx]);
  idx = (idx + 1) % 5;
}

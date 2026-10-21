/**
 * ST7735S 面板自检 / 寻址探测固件（独立于时钟主程序）
 *
 * 先显示 6 秒竖屏基准图，然后每 6 秒切换一组横屏 MADCTL。
 * 屏幕中央显示 #序号 和十六进制 MADCTL，方便肉眼确定哪组是正的。
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
      cfg.freq_write = 10000000;  // 先降到 10 MHz，排除线材/时序问题
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
      cfg.rgb_order = true;  // 这块屏实测用 RGB
      cfg.dlen_16bit = false;
      cfg.bus_shared = true;
      panel_.config(cfg);
    }
    setPanel(&panel_);
  }
};

static LGFX lcd;

static void backlightOn() {
  ledcSetup(0, 5000, 8);
  ledcAttachPin(PIN_BL, 0);
  ledcWrite(0, 255);
}

static void forceMadctl(uint8_t mad) {
  lcd.startWrite();
  lcd.writeCommand(0x36);
  lcd.writeData(mad);
  lcd.endWrite();
}

static void cornerLabels() {
  lcd.setTextColor(TFT_WHITE, TFT_BLACK);
  lcd.setTextDatum(textdatum_t::top_left);
  lcd.drawString("TL", 32, 8, &fonts::Font2);
  lcd.setTextDatum(textdatum_t::top_right);
  lcd.drawString("TR", 128, 8, &fonts::Font2);
  lcd.setTextDatum(textdatum_t::bottom_left);
  lcd.drawString("BL", 32, 120, &fonts::Font2);
  lcd.setTextDatum(textdatum_t::bottom_right);
  lcd.drawString("BR", 128, 120, &fonts::Font2);
}

static void drawPortrait() {
  lcd.setRotation(0);
  forceMadctl(0x00);
  lcd.fillScreen(TFT_BLACK);

  for (int i = 0; i < 3; ++i) lcd.drawRect(i, i, 128 - 2 * i, 160 - 2 * i, TFT_WHITE);

  lcd.fillRect(4, 4, 24, 24, TFT_RED);
  lcd.fillRect(100, 4, 24, 24, TFT_GREEN);
  lcd.fillRect(4, 132, 24, 24, TFT_BLUE);
  lcd.fillRect(100, 132, 24, 24, TFT_YELLOW);

  lcd.drawFastVLine(64, 0, 160, TFT_DARKGREY);
  lcd.drawFastHLine(0, 80, 128, TFT_DARKGREY);

  lcd.setTextDatum(textdatum_t::middle_center);
  lcd.setTextColor(TFT_WHITE, TFT_BLACK);
  lcd.drawString("PORTRAIT", 64, 62, &fonts::Font4);
  lcd.drawString("128x160", 64, 94, &fonts::Font2);
  Serial.println("[probe] PORTRAIT rot0 MAD 0x00");
}

static void drawLandscape(uint8_t idx, uint8_t mad) {
  lcd.setRotation(1);
  forceMadctl(mad);
  lcd.fillScreen(TFT_BLACK);

  for (int i = 0; i < 3; ++i) lcd.drawRect(i, i, 160 - 2 * i, 128 - 2 * i, TFT_WHITE);

  lcd.fillRect(4, 4, 24, 24, TFT_RED);
  lcd.fillRect(132, 4, 24, 24, TFT_GREEN);
  lcd.fillRect(4, 100, 24, 24, TFT_BLUE);
  lcd.fillRect(132, 100, 24, 24, TFT_YELLOW);

  lcd.drawFastVLine(80, 0, 128, TFT_DARKGREY);
  lcd.drawFastHLine(0, 64, 160, TFT_DARKGREY);

  cornerLabels();

  lcd.setTextDatum(textdatum_t::middle_center);
  lcd.setTextColor(TFT_WHITE, TFT_BLACK);
  char buf[20];
  snprintf(buf, sizeof(buf), "#%u 0x%02X", idx, mad);
  lcd.drawString(buf, 80, 48, &fonts::Font4);
  lcd.drawString("TOP", 80, 24, &fonts::Font2);
  lcd.drawString("BOTTOM", 80, 104, &fonts::Font2);

  Serial.printf("[probe] #%u landscape rot1 MAD 0x%02X\n", idx, mad);
}

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println();
  Serial.println("=== ST7735S panel probe ===");

  backlightOn();
  lcd.setColorDepth(16);
  lcd.init();
  lcd.fillScreen(TFT_BLACK);

  drawPortrait();
  delay(6000);

  static const uint8_t mads[] = {0x60, 0x64, 0x20, 0x24, 0xA0, 0xA4, 0xE0, 0xE4};
  for (uint8_t i = 0; i < sizeof(mads); ++i) {
    drawLandscape(i + 1, mads[i]);
    delay(6000);
  }

  // 循环，方便反复观察
  for (;;) {
    drawPortrait();
    delay(6000);
    for (uint8_t i = 0; i < sizeof(mads); ++i) {
      drawLandscape(i + 1, mads[i]);
      delay(6000);
    }
  }
}

void loop() {}

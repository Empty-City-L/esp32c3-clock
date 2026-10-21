/**
 * board_config.h — ESP32-C3 + 1.8" ST7735S 硬件配置
 *
 * 屏幕：1.8 寸 ST7735S，128 x 160，SPI，RGB565
 * 主控：ESP32-C3（RISC-V，160 MHz，4 MB Flash）
 *
 * 接线（与用户实际走线一致）：
 *   SCL/CLK -> GPIO4
 *   SDA/MOSI-> GPIO6
 *   CS      -> GPIO7
 *   RES/RST -> GPIO3
 *   DC      -> GPIO2
 *   BLK     -> GPIO5（PWM 调光）
 *
 * 注意：
 *   - ESP32-C3 的 GPIO4/5/6/7 也是 JTAG 引脚，接屏幕没有影响，
 *     但插着 JTAG 调试器时不要同时驱动这些脚。
 *   - GPIO2 是 C3 的启动 strapping 脚，屏幕上电时 DC 悬空一般没问题；
 *     若遇到无法启动，可在 DC 与 3V3 之间加 10k 上拉。
 */
#pragma once

/* ---------------- 屏幕 ---------------- */
#define SCREEN_W 160          // 横屏宽度
#define SCREEN_H 128          // 横屏高度
#define SCREEN_ROTATION 1     // 横屏；当前调试值 1，方向不对可在 Web 里切换

/* ---------------- 引脚 ---------------- */
#define PIN_LCD_SCLK  4       // SCL
#define PIN_LCD_MOSI  6       // SDA
#define PIN_LCD_CS    7       // CS
#define PIN_LCD_RST   3       // RES
#define PIN_LCD_DC    2       // DC
#define PIN_LCD_BL    5       // BLK / 背光

#define LCD_BL_CHANNEL 0      // LEDC 通道
#define LCD_BL_FREQ    5000   // 5 kHz，无频闪
#define LCD_BL_BITS    8

/* ---------------- ST7735S 参数 ---------------- */
#define LCD_SPI_HZ     10000000
#define LCD_PANEL_W    128
#define LCD_PANEL_H    160
#define LCD_MEMORY_W   132    // ST7735S 内部 GRAM
#define LCD_MEMORY_H   162
#define LCD_OFFSET_X   2      // 128 宽面板在 132 宽 GRAM 中的偏移
#define LCD_OFFSET_Y   1      // 160 高面板在 162 高 GRAM 中的偏移
#define LCD_INVERT     false  // 颜色像底片时改为 true（Web 里也能改）
#define LCD_RGB_ORDER  true   // LovyanGFX: true = RGB（这块屏实测用 RGB）

/* ---------------- 应用 ---------------- */
#define APP_NAME       "ESP32-C3 桌面时钟"
#define APP_VERSION    "1.0.0"

#define AP_SSID_PREFIX "ESP32-Clock-"
#define MDNS_HOST      "esp32clock"

#define WIFI_CONNECT_TIMEOUT_MS  (20UL * 1000UL)
#define WIFI_RETRY_MS            (10UL * 1000UL)
#define AP_KEEP_AFTER_STA_MS     (60UL * 1000UL)

/* ---------------- 默认时间 ---------------- */
#define TZ_DEFAULT     "CST-8"
#define NTP_DEFAULT_1  "ntp.aliyun.com"
#define NTP_DEFAULT_2  "cn.pool.ntp.org"
#define NTP_DEFAULT_3  "pool.ntp.org"

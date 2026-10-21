/**
 * @file lv_conf.h
 * LVGL 8.3.x configuration for the ESP32-C3 clock.
 *
 * 只列出与默认值不同的配置，其余由 lv_conf_internal.h 补齐。
 * 目标是：Flash 占用小、RAM 交给 ESP32 堆管理、只保留界面用到的控件。
 */
#ifndef LV_CONF_H
#define LV_CONF_H

#include <stdint.h>

/*====================
   COLOR SETTINGS
 *====================*/
#define LV_COLOR_DEPTH 16
#define LV_COLOR_16_SWAP 0

/* 背景使用纯色，避免 RGB565 渐变出现色阶/斜纹 */
#define LV_DITHER_GRADIENT 0

/*=========================
   MEMORY SETTINGS
 *=========================*/
/* 不使用 LVGL 的 64 kB 静态内存池，改用 ESP32 内部堆，
   把 DRAM 留给 WiFi 协议栈和 DMA 绘制缓冲。 */
#define LV_MEM_CUSTOM 1
#define LV_MEM_CUSTOM_INCLUDE <esp_heap_caps.h>
#define LV_MEM_CUSTOM_ALLOC(size) heap_caps_malloc((size), MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT)
#define LV_MEM_CUSTOM_FREE(ptr) heap_caps_free(ptr)
#define LV_MEM_CUSTOM_REALLOC(ptr, size) heap_caps_realloc((ptr), (size), MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT)
#define LV_MEM_BUF_MAX_NUM 16

/*====================
   HAL SETTINGS
 *====================*/
#define LV_DISP_DEF_REFR_PERIOD 30
#define LV_INDEV_DEF_READ_PERIOD 30

/* Tick 由 main.cpp 中的 lv_tick_inc() 提供 */
#define LV_TICK_CUSTOM 0

/*=======================
   FEATURE CONFIGURATION
 *=======================*/
#define LV_USE_LOG 0
#define LV_USE_ASSERT_NULL 1
#define LV_USE_ASSERT_MALLOC 1

#define LV_USE_THEME_DEFAULT 1
#define LV_THEME_DEFAULT_DARK 1

#define LV_USE_FLEX 1
#define LV_USE_GRID 0

#define LV_USE_SPINNER 1
#define LV_USE_BAR 1
#define LV_USE_LABEL 1
#define LV_LABEL_TEXT_SELECTION 0
#define LV_LABEL_LONG_TXT_HINT 1

#define LV_USE_ANIMIMG 0
#define LV_USE_ARC 1
#define LV_USE_LINE 1
#define LV_USE_IMG 1   /* lv_canvas.h 依赖 lv_img_t，必须保留 */

#define LV_USE_SNAPSHOT 0
#define LV_USE_MONKEY 0
#define LV_USE_GRIDNAV 0
#define LV_USE_FRAGMENT 0

/*==================
 *   FONT USAGE
 *==================*/
#define LV_FONT_MONTSERRAT_14 1
#define LV_FONT_MONTSERRAT_20 0
#define LV_FONT_MONTSERRAT_48 0

/* 中文界面使用 src/fonts 下用 lv_font_conv 生成的字体 */
#define LV_FONT_SIMSUN_16_CJK 0

#define LV_FONT_DEFAULT &lv_font_montserrat_14

#define LV_USE_FONT_COMPRESSED 0
#define LV_USE_FONT_SUBPX 0

#endif /* LV_CONF_H */

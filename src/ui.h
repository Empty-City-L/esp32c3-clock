/**
 * ui.h — 160 x 128 横屏 LVGL 界面
 */
#pragma once

#include <lvgl.h>
#include <time.h>

#include "weather.h"

namespace ui {

enum class Link {
  Online,   // 已对时
  Offline,  // 断网，靠 RTC 走时
};

/* 创建全部界面对象（lv_init 之后调用一次） */
void init();

/* 启动 / 配网提示页。line2 / line3 / foot 可传 nullptr */
void splash(const char *title, const char *line2 = nullptr, const char *line3 = nullptr,
            const char *foot = nullptr);

/* 切换到时钟页（带淡入） */
void show_clock();

/* 切换到天气页；按 weatherLayout() 自动选 A 当前页或 B 合并页 */
void show_weather(const WeatherData &w);

/* 切换到 A 方案的独立 3 天预报页 */
void show_forecast(const WeatherData &w);

/* 只刷新天气/预报页内容，不切页 */
void set_weather(const WeatherData &w);

/* 切换到照片页；pixels 是 160x128 RGB565，生命周期由调用方持有 */
void show_photo(const uint16_t *pixels);

/* 照片页只显示时间 */
void set_photo_time(const char *time);

/* 每秒刷新：星期、日期、时间、秒环 */
void set_clock(const struct tm &t);

/* 高频小步刷新秒环，放在主循环里调用 */
void tick();

/* 底部状态 */
void set_link(Link link);

/* 是否显示秒环（来自设置） */
void set_seconds_visible(bool visible);

}  // namespace ui

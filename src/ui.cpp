/**
 * ui.cpp — 160 x 128 横屏时钟界面
 *
 * 画面结构（横屏 160 x 128）：
 *
 *   周六                      10月18日
 *   ────────────────────────────────
 *
 *      23:25                ╭───╮
 *                           │38 │   ← 秒环
 *                           ╰───╯
 *
 *   ● 已同步                    NTP
 *   ────────────────────────────────
 */
#include "ui.h"

#include <stdio.h>
#include <string.h>

#include "board_config.h"
#include "photo.h"
#include "settings.h"

LV_FONT_DECLARE(font_time_44);
LV_FONT_DECLARE(font_sec_16);
LV_FONT_DECLARE(font_cn_14);
LV_FONT_DECLARE(font_cn_12);
LV_FONT_DECLARE(font_weather_28);
LV_FONT_DECLARE(font_weather_18);
LV_FONT_DECLARE(font_din_30);
LV_FONT_DECLARE(font_din_22);
LV_IMG_DECLARE(photo_grad_48);
LV_IMG_DECLARE(photo_grad_56);
LV_IMG_DECLARE(photo_grad_72);

namespace ui {
void set_forecast(const WeatherData &w);
namespace {

/* ------------------------- 配色 ------------------------- */
constexpr uint32_t COL_BG      = 0x070C15;
constexpr uint32_t COL_TEXT    = 0xF2F7FF;
constexpr uint32_t COL_ACCENT  = 0x5EEAD4;
constexpr uint32_t COL_MUTED   = 0x8FA0BC;
constexpr uint32_t COL_DIM     = 0x62748E;
constexpr uint32_t COL_HAIR    = 0x1A2740;
constexpr uint32_t COL_GREEN   = 0x34D399;
constexpr uint32_t COL_AMBER   = 0xFBBF24;
constexpr uint32_t COL_BLUE    = 0x60A5FA;

lv_obj_t *splash_box = nullptr;
lv_obj_t *splash_spin = nullptr;
lv_obj_t *splash_top = nullptr;
lv_obj_t *splash_title = nullptr;
lv_obj_t *splash_line2 = nullptr;
lv_obj_t *splash_line3 = nullptr;
lv_obj_t *splash_foot = nullptr;

lv_obj_t *clock_box = nullptr;
lv_obj_t *lbl_week = nullptr;
lv_obj_t *lbl_date = nullptr;
lv_obj_t *lbl_ampm = nullptr;
lv_obj_t *lbl_time = nullptr;
lv_obj_t *sec_arc = nullptr;
lv_obj_t *lbl_sec = nullptr;
lv_obj_t *dash = nullptr;
lv_obj_t *dot = nullptr;
lv_obj_t *lbl_status = nullptr;
lv_obj_t *lbl_proto = nullptr;

/* 天气页 */
lv_obj_t *weather_box = nullptr;
lv_obj_t *w_city = nullptr;
lv_obj_t *w_date = nullptr;
lv_obj_t *w_icon = nullptr;
lv_obj_t *w_temp = nullptr;
lv_obj_t *w_text = nullptr;
lv_obj_t *w_hilo = nullptr;
lv_obj_t *w_hum = nullptr;
lv_obj_t *w_wind = nullptr;
lv_obj_t *w_dot = nullptr;
lv_obj_t *w_status = nullptr;
lv_obj_t *w_age = nullptr;
lv_obj_t *w_msg = nullptr;

/* A：独立 3 天预报页 */
lv_obj_t *forecast_box = nullptr;
lv_obj_t *f_city = nullptr;
lv_obj_t *f_date = nullptr;
lv_obj_t *f_day[3] = {nullptr, nullptr, nullptr};
lv_obj_t *f_icon[3] = {nullptr, nullptr, nullptr};
lv_obj_t *f_text[3] = {nullptr, nullptr, nullptr};
lv_obj_t *f_hi[3] = {nullptr, nullptr, nullptr};
lv_obj_t *f_lo[3] = {nullptr, nullptr, nullptr};

/* B：当前天气 + 3 天合并页 */
lv_obj_t *compact_box = nullptr;
lv_obj_t *c_city = nullptr;
lv_obj_t *c_date = nullptr;
lv_obj_t *c_icon = nullptr;
lv_obj_t *c_temp = nullptr;
lv_obj_t *c_text = nullptr;
lv_obj_t *c_hilo = nullptr;
lv_obj_t *c_hum = nullptr;
lv_obj_t *c_day[3] = {nullptr, nullptr, nullptr};
lv_obj_t *c_icon_day[3] = {nullptr, nullptr, nullptr};
lv_obj_t *c_hilo_day[3] = {nullptr, nullptr, nullptr};

/* 照片页 */
lv_obj_t *photo_box = nullptr;
lv_obj_t *photo_img = nullptr;
lv_obj_t *photo_shade = nullptr;
lv_obj_t *photo_time_row = nullptr;
lv_obj_t *photo_hour = nullptr;
lv_obj_t *photo_min = nullptr;
lv_obj_t *photo_colon_orig = nullptr;
lv_obj_t *photo_colon_custom = nullptr;
lv_obj_t *photo_colon_dot1 = nullptr;
lv_obj_t *photo_colon_dot2 = nullptr;
lv_obj_t *photo_colon_spacer = nullptr;
lv_img_dsc_t photo_dsc;
bool photo_visible = false;

char last_time[8] = "";
char last_date[24] = "";
char last_week[8] = "";
char last_ampm[4] = "";
Link last_link = Link::Online;
bool clock_shown = false;
bool clock_visible = false;
bool weather_visible = false;
bool forecast_visible = false;
bool seconds_visible = true;

/* 秒环插值：把每秒的整数跳变铺成帧动画 */
uint32_t sec_tick_base_ms = 0;
uint32_t sec_tick_last_ms = 0;
uint8_t sec_tick_base_sec = 0;
bool sec_tick_active = false;

/* 一个“无样式”的透明容器 */
lv_obj_t *plain_container(lv_obj_t *parent, int w, int h) {
  lv_obj_t *obj = lv_obj_create(parent);
  lv_obj_remove_style_all(obj);
  lv_obj_set_size(obj, w, h);
  lv_obj_set_pos(obj, 0, 0);
  lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
  return obj;
}

lv_obj_t *make_label(lv_obj_t *parent, const lv_font_t *font, uint32_t color,
                     const char *text) {
  lv_obj_t *lbl = lv_label_create(parent);
  lv_obj_set_style_text_font(lbl, font, 0);
  lv_obj_set_style_text_color(lbl, lv_color_hex(color), 0);
  lv_obj_set_style_text_letter_space(lbl, 0, 0);
  lv_label_set_text(lbl, text);
  return lbl;
}

/* 细分隔线 */
lv_obj_t *hairline(lv_obj_t *parent, int x, int y, int w) {
  lv_obj_t *line = lv_obj_create(parent);
  lv_obj_remove_style_all(line);
  lv_obj_set_size(line, w, 1);
  lv_obj_set_pos(line, x, y);
  lv_obj_set_style_bg_color(line, lv_color_hex(COL_HAIR), 0);
  lv_obj_set_style_bg_opa(line, LV_OPA_COVER, 0);
  return line;
}

/* 页面切换：上移 10px + 淡入 */
lv_coord_t slide_dx = 0;
lv_coord_t slide_dy = 0;

void slide_anim_cb(void *obj, int32_t v) {
  lv_obj_t *o = static_cast<lv_obj_t *>(obj);
  lv_obj_set_style_opa(o, (lv_opa_t)v, 0);
  const int32_t k = 255 - v;
  lv_obj_set_pos(o, (lv_coord_t)((int32_t)slide_dx * k / 255),
                 (lv_coord_t)((int32_t)slide_dy * k / 255));
}

void slide_in(lv_obj_t *obj, uint32_t ms) {
  lv_anim_del(obj, slide_anim_cb);
  slide_dx = 0;
  slide_dy = 10;
  lv_obj_set_pos(obj, slide_dx, slide_dy);
  lv_obj_set_style_opa(obj, LV_OPA_TRANSP, 0);

  lv_anim_t a;
  lv_anim_init(&a);
  lv_anim_set_var(&a, obj);
  lv_anim_set_values(&a, 0, 255);
  lv_anim_set_time(&a, ms);
  lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
  lv_anim_set_exec_cb(&a, slide_anim_cb);
  lv_anim_start(&a);
}

void build_splash(lv_obj_t *scr) {
  splash_box = plain_container(scr, SCREEN_W, SCREEN_H);

  splash_top = make_label(splash_box, &font_cn_12, COL_DIM, "ESP32-C3");
  lv_obj_align(splash_top, LV_ALIGN_TOP_MID, 0, 4);

  splash_spin = lv_spinner_create(splash_box, 1400, 70);
  lv_obj_set_size(splash_spin, 30, 30);
  lv_obj_align(splash_spin, LV_ALIGN_TOP_MID, 0, 22);
  lv_obj_set_style_pad_all(splash_spin, 0, 0);
  lv_obj_set_style_arc_width(splash_spin, 2, LV_PART_MAIN);
  lv_obj_set_style_arc_width(splash_spin, 2, LV_PART_INDICATOR);
  lv_obj_set_style_arc_color(splash_spin, lv_color_hex(COL_HAIR), LV_PART_MAIN);
  lv_obj_set_style_arc_color(splash_spin, lv_color_hex(COL_ACCENT), LV_PART_INDICATOR);
  lv_obj_set_style_bg_opa(splash_spin, LV_OPA_TRANSP, LV_PART_MAIN);

  splash_title = make_label(splash_box, &font_cn_14, COL_TEXT, "");
  lv_obj_set_width(splash_title, 148);
  lv_obj_set_style_text_align(splash_title, LV_TEXT_ALIGN_CENTER, 0);
  lv_label_set_long_mode(splash_title, LV_LABEL_LONG_DOT);
  lv_obj_align(splash_title, LV_ALIGN_TOP_MID, 0, 54);

  splash_line2 = make_label(splash_box, &font_cn_12, COL_MUTED, "");
  lv_obj_set_width(splash_line2, 148);
  lv_obj_set_style_text_align(splash_line2, LV_TEXT_ALIGN_CENTER, 0);
  lv_label_set_long_mode(splash_line2, LV_LABEL_LONG_DOT);
  lv_obj_align(splash_line2, LV_ALIGN_TOP_MID, 0, 74);

  splash_line3 = make_label(splash_box, &font_cn_12, COL_ACCENT, "");
  lv_obj_set_width(splash_line3, 148);
  lv_obj_set_style_text_align(splash_line3, LV_TEXT_ALIGN_CENTER, 0);
  lv_label_set_long_mode(splash_line3, LV_LABEL_LONG_DOT);
  lv_obj_align(splash_line3, LV_ALIGN_TOP_MID, 0, 92);

  splash_foot = make_label(splash_box, &font_cn_12, COL_DIM, "ST7735S 160x128");
  lv_obj_set_width(splash_foot, 148);
  lv_obj_set_style_text_align(splash_foot, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_align(splash_foot, LV_ALIGN_BOTTOM_MID, 0, -4);
}

void build_clock(lv_obj_t *scr) {
  clock_box = plain_container(scr, SCREEN_W, SCREEN_H);

  /* 顶部：星期 + 日期 */
  lbl_week = make_label(clock_box, &font_cn_12, COL_ACCENT, "周六");
  lv_obj_set_pos(lbl_week, 9, 6);

  lbl_ampm = make_label(clock_box, &font_cn_12, COL_MUTED, "PM");
  lv_obj_set_pos(lbl_ampm, 40, 6);
  lv_obj_add_flag(lbl_ampm, LV_OBJ_FLAG_HIDDEN);

  lbl_date = make_label(clock_box, &font_cn_12, COL_MUTED, "10月18日");
  lv_obj_align(lbl_date, LV_ALIGN_TOP_RIGHT, -9, 6);

  hairline(clock_box, 9, 24, SCREEN_W - 18);

  /* 大号时间：固定宽度 + 居中，切换 1:05 / 11:05 时不会左右跳 */
  lbl_time = make_label(clock_box, &font_time_44, COL_TEXT, "23:25");
  lv_obj_set_width(lbl_time, 114);
  lv_obj_set_style_text_align(lbl_time, LV_TEXT_ALIGN_CENTER, 0);
  lv_label_set_long_mode(lbl_time, LV_LABEL_LONG_CLIP);
  lv_obj_align(lbl_time, LV_ALIGN_TOP_LEFT, 2, 40);

  /* 右侧秒环：270° 圆弧 + 中心数字 */
  sec_arc = lv_arc_create(clock_box);
  lv_obj_set_size(sec_arc, 36, 36);
  lv_obj_align(sec_arc, LV_ALIGN_TOP_RIGHT, -4, 41);
  lv_obj_set_style_pad_all(sec_arc, 0, 0);
  lv_obj_set_style_bg_opa(sec_arc, LV_OPA_TRANSP, LV_PART_MAIN);
  lv_obj_set_style_border_width(sec_arc, 0, LV_PART_MAIN);
  lv_obj_set_style_arc_width(sec_arc, 3, LV_PART_MAIN);
  lv_obj_set_style_arc_width(sec_arc, 3, LV_PART_INDICATOR);
  lv_obj_set_style_arc_color(sec_arc, lv_color_hex(COL_HAIR), LV_PART_MAIN);
  lv_obj_set_style_arc_color(sec_arc, lv_color_hex(COL_ACCENT), LV_PART_INDICATOR);
  lv_obj_set_style_opa(sec_arc, LV_OPA_TRANSP, LV_PART_KNOB);
  /* LVGL 8 arc 量程是 int16_t，用 0..3000（每步 20ms）实现平滑走秒 */
  lv_arc_set_range(sec_arc, 0, 3000);
  lv_arc_set_bg_angles(sec_arc, 135, 45);
  lv_arc_set_value(sec_arc, 0);

  lbl_sec = make_label(clock_box, &font_sec_16, COL_ACCENT, "38");
  lv_obj_align_to(lbl_sec, sec_arc, LV_ALIGN_CENTER, 0, 0);

  /* 不显示秒时，用一小段强调线填补右侧留白 */
  dash = lv_obj_create(clock_box);
  lv_obj_remove_style_all(dash);
  lv_obj_set_size(dash, 22, 2);
  lv_obj_align_to(dash, sec_arc, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_radius(dash, LV_RADIUS_CIRCLE, 0);
  lv_obj_set_style_bg_color(dash, lv_color_hex(COL_ACCENT), 0);
  lv_obj_set_style_bg_opa(dash, LV_OPA_COVER, 0);
  lv_obj_add_flag(dash, LV_OBJ_FLAG_HIDDEN);

  /* 底部状态 */
  dot = lv_obj_create(clock_box);
  lv_obj_remove_style_all(dot);
  lv_obj_set_size(dot, 5, 5);
  lv_obj_set_pos(dot, 9, 104);
  lv_obj_set_style_radius(dot, LV_RADIUS_CIRCLE, 0);
  lv_obj_set_style_bg_color(dot, lv_color_hex(COL_GREEN), 0);
  lv_obj_set_style_bg_opa(dot, LV_OPA_COVER, 0);

  lbl_status = make_label(clock_box, &font_cn_12, COL_MUTED, "已同步");
  lv_obj_set_pos(lbl_status, 19, 100);

  lbl_proto = make_label(clock_box, &font_cn_12, COL_DIM, "NTP");
  lv_obj_align(lbl_proto, LV_ALIGN_TOP_RIGHT, -9, 100);

  hairline(clock_box, 9, 120, SCREEN_W - 18);

  lv_obj_add_flag(clock_box, LV_OBJ_FLAG_HIDDEN);
}

void build_weather(lv_obj_t *scr) {
  weather_box = plain_container(scr, SCREEN_W, SCREEN_H);

  w_city = make_label(weather_box, &font_cn_12, COL_ACCENT, "北京");
  lv_obj_set_width(w_city, 76);
  lv_label_set_long_mode(w_city, LV_LABEL_LONG_DOT);
  lv_obj_set_pos(w_city, 9, 5);

  w_date = make_label(weather_box, &font_cn_12, COL_MUTED, "10月19日");
  lv_obj_align(w_date, LV_ALIGN_TOP_RIGHT, -9, 5);

  hairline(weather_box, 9, 23, SCREEN_W - 18);

  w_icon = make_label(weather_box, &font_weather_28, COL_ACCENT, "☀");
  lv_obj_set_width(w_icon, 32);
  lv_obj_set_style_text_align(w_icon, LV_TEXT_ALIGN_CENTER, 0);
  lv_label_set_long_mode(w_icon, LV_LABEL_LONG_CLIP);
  lv_obj_set_pos(w_icon, 8, 29);

  w_temp = make_label(weather_box, &font_time_44, COL_TEXT, "26°");
  lv_obj_set_width(w_temp, 100);
  lv_label_set_long_mode(w_temp, LV_LABEL_LONG_CLIP);
  lv_obj_set_pos(w_temp, 52, 33);

  w_text = make_label(weather_box, &font_cn_14, COL_ACCENT, "多云");
  lv_obj_set_pos(w_text, 10, 64);

  w_hilo = make_label(weather_box, &font_cn_12, COL_MUTED, "18° / 27°");
  lv_obj_set_pos(w_hilo, 54, 70);

  w_hum = make_label(weather_box, &font_cn_12, COL_MUTED, "湿度 45%");
  lv_obj_set_pos(w_hum, 10, 88);

  w_wind = make_label(weather_box, &font_cn_12, COL_DIM, "东南风 2级");
  lv_obj_set_width(w_wind, 76);
  lv_obj_set_pos(w_wind, 75, 88);

  w_dot = lv_obj_create(weather_box);
  lv_obj_remove_style_all(w_dot);
  lv_obj_set_size(w_dot, 5, 5);
  lv_obj_set_pos(w_dot, 9, 105);
  lv_obj_set_style_radius(w_dot, LV_RADIUS_CIRCLE, 0);
  lv_obj_set_style_bg_color(w_dot, lv_color_hex(COL_GREEN), 0);
  lv_obj_set_style_bg_opa(w_dot, LV_OPA_COVER, 0);

  w_status = make_label(weather_box, &font_cn_12, COL_MUTED, "天气已更新");
  lv_obj_set_pos(w_status, 19, 102);

  w_age = make_label(weather_box, &font_cn_12, COL_DIM, "刚刚");
  lv_obj_align(w_age, LV_ALIGN_TOP_RIGHT, -9, 102);

  w_msg = make_label(weather_box, &font_cn_14, COL_MUTED, "未配置天气");
  lv_obj_set_width(w_msg, 144);
  lv_obj_set_style_text_align(w_msg, LV_TEXT_ALIGN_CENTER, 0);
  lv_label_set_long_mode(w_msg, LV_LABEL_LONG_DOT);
  lv_obj_align(w_msg, LV_ALIGN_TOP_MID, 0, 52);
  lv_obj_add_flag(w_msg, LV_OBJ_FLAG_HIDDEN);

  lv_obj_add_flag(weather_box, LV_OBJ_FLAG_HIDDEN);
}

lv_obj_t *centered_label(lv_obj_t *parent, const lv_font_t *font, uint32_t color,
                         const char *text, int cx, int y, int w) {
  lv_obj_t *lbl = make_label(parent, font, color, text);
  lv_obj_set_width(lbl, w);
  lv_obj_set_style_text_align(lbl, LV_TEXT_ALIGN_CENTER, 0);
  lv_label_set_long_mode(lbl, LV_LABEL_LONG_CLIP);
  lv_obj_set_pos(lbl, cx - w / 2, y);
  return lbl;
}

void build_forecast(lv_obj_t *scr) {
  forecast_box = plain_container(scr, SCREEN_W, SCREEN_H);

  f_city = make_label(forecast_box, &font_cn_12, COL_ACCENT, "邵阳");
  lv_obj_set_width(f_city, 76);
  lv_label_set_long_mode(f_city, LV_LABEL_LONG_DOT);
  lv_obj_set_pos(f_city, 9, 5);

  f_date = make_label(forecast_box, &font_cn_12, COL_MUTED, "10月19日");
  lv_obj_align(f_date, LV_ALIGN_TOP_RIGHT, -9, 5);
  hairline(forecast_box, 9, 23, SCREEN_W - 18);

  const int cols[3] = {32, 80, 128};
  const char *days[3] = {"今天", "明天", "后天"};
  for (int i = 0; i < 3; ++i) {
    f_day[i] = centered_label(forecast_box, &font_cn_12,
                              i == 0 ? COL_ACCENT : COL_MUTED, days[i],
                              cols[i], 25, 46);
    f_icon[i] = centered_label(forecast_box, &font_weather_28,
                               i == 0 ? COL_ACCENT : COL_MUTED, "☀",
                               cols[i], 39, 46);
    f_text[i] = centered_label(forecast_box, &font_cn_12, COL_MUTED, "多云",
                               cols[i], 68, 46);
    f_hi[i] = centered_label(forecast_box, &font_din_22, COL_TEXT, "24°",
                             cols[i], 82, 46);
    f_lo[i] = centered_label(forecast_box, &font_cn_12, COL_DIM, "16°",
                             cols[i], 104, 46);
  }

  lv_obj_add_flag(forecast_box, LV_OBJ_FLAG_HIDDEN);
}

void build_compact(lv_obj_t *scr) {
  compact_box = plain_container(scr, SCREEN_W, SCREEN_H);

  c_city = make_label(compact_box, &font_cn_12, COL_ACCENT, "邵阳");
  lv_obj_set_width(c_city, 76);
  lv_label_set_long_mode(c_city, LV_LABEL_LONG_DOT);
  lv_obj_set_pos(c_city, 9, 4);

  c_date = make_label(compact_box, &font_cn_12, COL_MUTED, "10月19日");
  lv_obj_align(c_date, LV_ALIGN_TOP_RIGHT, -9, 4);
  hairline(compact_box, 9, 20, SCREEN_W - 18);

  c_icon = centered_label(compact_box, &font_weather_28, COL_ACCENT, "☀",
                          19, 25, 30);
  c_temp = make_label(compact_box, &font_din_30, COL_TEXT, "19°");
  lv_obj_set_width(c_temp, 64);
  lv_label_set_long_mode(c_temp, LV_LABEL_LONG_CLIP);
  lv_obj_set_pos(c_temp, 36, 26);

  c_text = make_label(compact_box, &font_cn_12, COL_ACCENT, "多云");
  lv_obj_set_pos(c_text, 38, 57);

  c_hilo = make_label(compact_box, &font_cn_12, COL_MUTED, "24° / 16°");
  lv_obj_set_pos(c_hilo, 94, 32);

  c_hum = make_label(compact_box, &font_cn_12, COL_DIM, "湿度 76%");
  lv_obj_set_pos(c_hum, 94, 50);

  hairline(compact_box, 9, 73, SCREEN_W - 18);

  const int cols[3] = {32, 80, 128};
  const char *days[3] = {"今天", "明天", "后天"};
  for (int i = 0; i < 3; ++i) {
    c_day[i] = centered_label(compact_box, &font_cn_12,
                              i == 0 ? COL_ACCENT : COL_MUTED, days[i],
                              cols[i], 78, 46);
    c_icon_day[i] = centered_label(compact_box, &font_weather_18,
                                   i == 0 ? COL_ACCENT : COL_MUTED, "☀",
                                   cols[i], 93, 46);
    c_hilo_day[i] = centered_label(compact_box, &font_cn_12, COL_MUTED, "24/16",
                                   cols[i], 113, 46);
  }

  lv_obj_add_flag(compact_box, LV_OBJ_FLAG_HIDDEN);
}

void apply_photo_style() {
  if (!photo_time_row) return;
  const PhotoStyle &s = photoStyle();

  if (!s.timeOn) {
    lv_obj_add_flag(photo_time_row, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(photo_shade, LV_OBJ_FLAG_HIDDEN);
    return;
  }
  lv_obj_clear_flag(photo_time_row, LV_OBJ_FLAG_HIDDEN);

  const lv_font_t *font = &font_time_44;
  int row_h = 52;
  int bottom = -2;
  if (s.timeStyle == 1) {
    font = &font_din_30;
    row_h = 40;
    bottom = -8;
  } else if (s.timeStyle == 2) {
    font = &font_din_22;
    row_h = 32;
    bottom = -12;
  }

  lv_obj_set_style_text_font(photo_hour, font, 0);
  lv_obj_set_style_text_font(photo_min, font, 0);
  lv_obj_set_style_text_font(photo_colon_orig, font, 0);
  lv_obj_set_style_text_color(photo_hour, lv_color_hex(s.timeColor), 0);
  lv_obj_set_style_text_color(photo_min, lv_color_hex(s.timeColor), 0);
  lv_obj_set_style_text_color(photo_colon_orig, lv_color_hex(s.timeColor), 0);

  lv_obj_set_height(photo_time_row, row_h);
  lv_obj_set_style_pad_left(photo_time_row, 9, 0);
  lv_obj_set_style_pad_right(photo_time_row, 9, 0);
  lv_obj_set_flex_align(photo_time_row,
                        s.timeStyle == 2 ? LV_FLEX_ALIGN_END : LV_FLEX_ALIGN_START,
                        LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_align(photo_time_row, LV_ALIGN_BOTTOM_LEFT, 0, bottom);

  const bool orig = s.colonStyle == 0;
  const bool custom = s.colonStyle == 1 || s.colonStyle == 2;
  const bool spacer = s.colonStyle == 3;
  if (orig) lv_obj_clear_flag(photo_colon_orig, LV_OBJ_FLAG_HIDDEN);
  else lv_obj_add_flag(photo_colon_orig, LV_OBJ_FLAG_HIDDEN);
  if (custom) lv_obj_clear_flag(photo_colon_custom, LV_OBJ_FLAG_HIDDEN);
  else lv_obj_add_flag(photo_colon_custom, LV_OBJ_FLAG_HIDDEN);
  if (spacer) lv_obj_clear_flag(photo_colon_spacer, LV_OBJ_FLAG_HIDDEN);
  else lv_obj_add_flag(photo_colon_spacer, LV_OBJ_FLAG_HIDDEN);

  const int cw = s.timeStyle == 0 ? 14 : 12;
  lv_obj_set_size(photo_colon_custom, cw, row_h);
  int dot = s.timeStyle == 0 ? 5 : (s.timeStyle == 1 ? 4 : 3);
  int dot_h = dot;
  int dw = dot;
  const bool thin = s.colonStyle == 2;
  if (thin) {
    dw = 2;
    dot_h = s.timeStyle == 0 ? 5 : 4;
  }
  const int cy1 = (row_h * (thin ? 36 : 40)) / 100;
  const int cy2 = (row_h * (thin ? 60 : 58)) / 100;
  lv_obj_set_size(photo_colon_dot1, dw, dot_h);
  lv_obj_set_size(photo_colon_dot2, dw, dot_h);
  lv_obj_set_pos(photo_colon_dot1, (cw - dw) / 2, cy1 - dot_h / 2);
  lv_obj_set_pos(photo_colon_dot2, (cw - dw) / 2, cy2 - dot_h / 2);
  const lv_coord_t radius = s.colonStyle == 2 ? 1 : LV_RADIUS_CIRCLE;
  lv_obj_set_style_radius(photo_colon_dot1, radius, 0);
  lv_obj_set_style_radius(photo_colon_dot2, radius, 0);
  lv_obj_set_style_bg_color(photo_colon_dot1, lv_color_hex(s.timeColor), 0);
  lv_obj_set_style_bg_color(photo_colon_dot2, lv_color_hex(s.timeColor), 0);

  if (s.gradOn) {
    const lv_img_dsc_t *src = &photo_grad_56;
    if (s.gradHeight == 48) src = &photo_grad_48;
    else if (s.gradHeight == 72) src = &photo_grad_72;
    lv_img_set_src(photo_shade, src);
    lv_obj_set_size(photo_shade, SCREEN_W, s.gradHeight);
    lv_obj_align(photo_shade, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_img_recolor(photo_shade, lv_color_hex(s.gradColor), 0);
    lv_obj_set_style_img_recolor_opa(photo_shade, LV_OPA_COVER, 0);
    lv_obj_set_style_opa(photo_shade, (lv_opa_t)((uint16_t)s.gradOpa * 255 / 100), 0);
    lv_obj_clear_flag(photo_shade, LV_OBJ_FLAG_HIDDEN);
  } else {
    lv_obj_add_flag(photo_shade, LV_OBJ_FLAG_HIDDEN);
  }
}

void build_photo(lv_obj_t *scr) {
  photo_box = plain_container(scr, SCREEN_W, SCREEN_H);

  photo_img = lv_img_create(photo_box);
  lv_obj_set_pos(photo_img, 0, 0);
  lv_obj_set_size(photo_img, SCREEN_W, SCREEN_H);
  lv_obj_clear_flag(photo_img, LV_OBJ_FLAG_SCROLLABLE);

  photo_shade = lv_img_create(photo_box);
  lv_obj_set_pos(photo_shade, 0, 0);
  lv_obj_add_flag(photo_shade, LV_OBJ_FLAG_HIDDEN);

  photo_time_row = plain_container(photo_box, SCREEN_W, 52);
  lv_obj_set_flex_flow(photo_time_row, LV_FLEX_FLOW_ROW);
  lv_obj_set_style_pad_column(photo_time_row, 0, 0);

  photo_hour = make_label(photo_time_row, &font_time_44, COL_TEXT, "13");
  photo_colon_orig = make_label(photo_time_row, &font_time_44, COL_TEXT, ":");
  photo_colon_custom = lv_obj_create(photo_time_row);
  lv_obj_remove_style_all(photo_colon_custom);
  lv_obj_clear_flag(photo_colon_custom, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_size(photo_colon_custom, 10, 52);

  photo_colon_dot1 = lv_obj_create(photo_colon_custom);
  lv_obj_remove_style_all(photo_colon_dot1);
  lv_obj_set_style_radius(photo_colon_dot1, LV_RADIUS_CIRCLE, 0);
  lv_obj_set_style_bg_opa(photo_colon_dot1, LV_OPA_COVER, 0);
  photo_colon_dot2 = lv_obj_create(photo_colon_custom);
  lv_obj_remove_style_all(photo_colon_dot2);
  lv_obj_set_style_radius(photo_colon_dot2, LV_RADIUS_CIRCLE, 0);
  lv_obj_set_style_bg_opa(photo_colon_dot2, LV_OPA_COVER, 0);

  photo_colon_spacer = lv_obj_create(photo_time_row);
  lv_obj_remove_style_all(photo_colon_spacer);
  lv_obj_set_size(photo_colon_spacer, 7, 1);
  lv_obj_add_flag(photo_colon_spacer, LV_OBJ_FLAG_HIDDEN);

  photo_min = make_label(photo_time_row, &font_time_44, COL_TEXT, "16");

  apply_photo_style();
  lv_obj_add_flag(photo_box, LV_OBJ_FLAG_HIDDEN);
}

}  // namespace

void init() {
  lv_obj_t *scr = lv_scr_act();
  lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_bg_color(scr, lv_color_hex(COL_BG), 0);
  lv_obj_set_style_bg_grad_dir(scr, LV_GRAD_DIR_NONE, 0);
  lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);
  lv_obj_set_style_pad_all(scr, 0, 0);
  lv_obj_set_style_border_width(scr, 0, 0);

  build_splash(scr);
  build_clock(scr);
  build_weather(scr);
  build_forecast(scr);
  build_compact(scr);
  build_photo(scr);
  // 不创建底部页码点：A/B/照片页都不显示圆点。

  seconds_visible = (gSettings.flags & FLAG_SHOW_SECONDS) != 0;
  set_seconds_visible(seconds_visible);
}

void splash(const char *title, const char *line2, const char *line3, const char *foot) {
  if (!splash_box) return;

  lv_label_set_text(splash_title, title ? title : "");
  lv_label_set_text(splash_line2, line2 ? line2 : "");
  lv_label_set_text(splash_line3, line3 ? line3 : "");
  lv_label_set_text(splash_foot, foot ? foot : "");

  if (clock_box) lv_obj_add_flag(clock_box, LV_OBJ_FLAG_HIDDEN);
  if (weather_box) lv_obj_add_flag(weather_box, LV_OBJ_FLAG_HIDDEN);
  if (compact_box) lv_obj_add_flag(compact_box, LV_OBJ_FLAG_HIDDEN);
  if (forecast_box) lv_obj_add_flag(forecast_box, LV_OBJ_FLAG_HIDDEN);
  if (photo_box) lv_obj_add_flag(photo_box, LV_OBJ_FLAG_HIDDEN);
  lv_obj_clear_flag(splash_box, LV_OBJ_FLAG_HIDDEN);
  clock_shown = false;
  clock_visible = false;
  weather_visible = false;
  forecast_visible = false;
  photo_visible = false;
}

void show_clock() {
  if (!clock_box) return;

  lv_obj_add_flag(splash_box, LV_OBJ_FLAG_HIDDEN);
  if (weather_box) lv_obj_add_flag(weather_box, LV_OBJ_FLAG_HIDDEN);
  if (compact_box) lv_obj_add_flag(compact_box, LV_OBJ_FLAG_HIDDEN);
  if (forecast_box) lv_obj_add_flag(forecast_box, LV_OBJ_FLAG_HIDDEN);
  if (photo_box) lv_obj_add_flag(photo_box, LV_OBJ_FLAG_HIDDEN);
  lv_obj_clear_flag(clock_box, LV_OBJ_FLAG_HIDDEN);

#ifdef UI_DUMP
  lv_obj_set_style_opa(clock_box, LV_OPA_COVER, 0);
#else
  if (!clock_visible) slide_in(clock_box, 300);
  sec_tick_active = false;

  /* 轮播回到时钟页时立刻对齐当前秒，避免秒环先显示旧值再跳回。 */
  const time_t now = time(nullptr);
  if (now > 1700000000) {
    struct tm tmv;
    localtime_r(&now, &tmv);
    set_clock(tmv);
  }
#endif
  clock_shown = true;
  clock_visible = true;
  weather_visible = false;
  forecast_visible = false;
  photo_visible = false;
}

void show_weather(const WeatherData &w) {
  if (!weather_box || !compact_box) return;

  const bool compact = weatherLayout() == 1;
  lv_obj_add_flag(splash_box, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(clock_box, LV_OBJ_FLAG_HIDDEN);
  if (photo_box) lv_obj_add_flag(photo_box, LV_OBJ_FLAG_HIDDEN);
  if (forecast_box) lv_obj_add_flag(forecast_box, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(weather_box, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(compact_box, LV_OBJ_FLAG_HIDDEN);

  lv_obj_t *box = compact ? compact_box : weather_box;
  lv_obj_clear_flag(box, LV_OBJ_FLAG_HIDDEN);
  set_weather(w);

#ifndef UI_DUMP
  if (!weather_visible) slide_in(box, 220);
#endif
  weather_visible = true;
  forecast_visible = false;
  clock_visible = false;
  clock_shown = false;
  photo_visible = false;
}

void show_forecast(const WeatherData &w) {
  if (!forecast_box) return;

  lv_obj_add_flag(splash_box, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(clock_box, LV_OBJ_FLAG_HIDDEN);
  if (weather_box) lv_obj_add_flag(weather_box, LV_OBJ_FLAG_HIDDEN);
  if (compact_box) lv_obj_add_flag(compact_box, LV_OBJ_FLAG_HIDDEN);
  if (photo_box) lv_obj_add_flag(photo_box, LV_OBJ_FLAG_HIDDEN);
  lv_obj_clear_flag(forecast_box, LV_OBJ_FLAG_HIDDEN);
  set_forecast(w);

#ifndef UI_DUMP
  if (!forecast_visible) slide_in(forecast_box, 220);
#endif
  forecast_visible = true;
  weather_visible = false;
  clock_visible = false;
  clock_shown = false;
  photo_visible = false;
}

void show_photo(const uint16_t *pixels) {
  if (!photo_box || !pixels) return;

  lv_obj_add_flag(splash_box, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(clock_box, LV_OBJ_FLAG_HIDDEN);
  if (weather_box) lv_obj_add_flag(weather_box, LV_OBJ_FLAG_HIDDEN);
  if (compact_box) lv_obj_add_flag(compact_box, LV_OBJ_FLAG_HIDDEN);
  if (forecast_box) lv_obj_add_flag(forecast_box, LV_OBJ_FLAG_HIDDEN);
  lv_obj_clear_flag(photo_box, LV_OBJ_FLAG_HIDDEN);

  photo_dsc.header.always_zero = 0;
  photo_dsc.header.w = SCREEN_W;
  photo_dsc.header.h = SCREEN_H;
  photo_dsc.header.cf = LV_IMG_CF_TRUE_COLOR;
  photo_dsc.data_size = SCREEN_W * SCREEN_H * 2;
  photo_dsc.data = reinterpret_cast<const uint8_t *>(pixels);
  lv_img_set_src(photo_img, &photo_dsc);

#ifndef UI_DUMP
  if (!photo_visible) slide_in(photo_box, 220);
#endif
  photo_visible = true;
  clock_visible = false;
  clock_shown = false;
  weather_visible = false;
  forecast_visible = false;
}

void set_photo_time(const char *time) {
  if (!photo_hour || !photo_min) return;
  char hh[4] = "--";
  char mm[4] = "--";
  if (time) {
    const char *sep = strchr(time, ':');
    if (sep && (sep - time) < (int)sizeof(hh)) {
      size_t n = (size_t)(sep - time);
      memcpy(hh, time, n);
      hh[n] = '\0';
      strncpy(mm, sep + 1, sizeof(mm) - 1);
      mm[sizeof(mm) - 1] = '\0';
    } else {
      strncpy(hh, time, sizeof(hh) - 1);
      hh[sizeof(hh) - 1] = '\0';
      mm[0] = '\0';
    }
  }
  lv_label_set_text(photo_hour, hh);
  lv_label_set_text(photo_min, mm);
}

void set_compact(const WeatherData &w) {
  if (!compact_box) return;

  char buf[32];
  lv_label_set_text(c_city, w.city[0] ? w.city : "--");
  lv_label_set_text(c_date, last_date);
  if (w.valid) {
    lv_label_set_text(c_icon, weatherIconSymbol(w.icon));
    snprintf(buf, sizeof(buf), "%d°", w.temp);
    lv_label_set_text(c_temp, buf);
    lv_label_set_text(c_text, w.text[0] ? w.text : "未知");
    snprintf(buf, sizeof(buf), "%d° / %d°", w.high, w.low);
    lv_label_set_text(c_hilo, buf);
    snprintf(buf, sizeof(buf), "湿度 %u%%", w.humidity);
    lv_label_set_text(c_hum, buf);
  } else {
    lv_label_set_text(c_icon, "☀");
    lv_label_set_text(c_temp, "--°");
    lv_label_set_text(c_text, "未知");
    lv_label_set_text(c_hilo, "--° / --°");
    lv_label_set_text(c_hum, "湿度 --");
  }

  for (int i = 0; i < 3; ++i) {
    if (i < w.forecastDays && w.forecast[i].valid) {
      lv_label_set_text(c_icon_day[i], weatherIconSymbol(w.forecast[i].icon));
      snprintf(buf, sizeof(buf), "%d/%d", w.forecast[i].high, w.forecast[i].low);
      lv_label_set_text(c_hilo_day[i], buf);
    } else {
      lv_label_set_text(c_icon_day[i], "≋");
      lv_label_set_text(c_hilo_day[i], "--/--");
    }
  }
}

void set_forecast(const WeatherData &w) {
  if (!forecast_box) return;

  char buf[32];
  lv_label_set_text(f_city, w.city[0] ? w.city : "--");
  lv_label_set_text(f_date, last_date);
  for (int i = 0; i < 3; ++i) {
    if (i < w.forecastDays && w.forecast[i].valid) {
      lv_label_set_text(f_icon[i], weatherIconSymbol(w.forecast[i].icon));
      lv_label_set_text(f_text[i], w.forecast[i].text[0] ? w.forecast[i].text : "未知");
      snprintf(buf, sizeof(buf), "%d°", w.forecast[i].high);
      lv_label_set_text(f_hi[i], buf);
      snprintf(buf, sizeof(buf), "%d°", w.forecast[i].low);
      lv_label_set_text(f_lo[i], buf);
    } else {
      lv_label_set_text(f_icon[i], "≋");
      lv_label_set_text(f_text[i], "--");
      lv_label_set_text(f_hi[i], "--°");
      lv_label_set_text(f_lo[i], "--°");
    }
  }
}

void set_weather(const WeatherData &w) {
  if (!weather_box) return;

  char buf[32];
  if (w.valid) {
    lv_obj_add_flag(w_msg, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(w_icon, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(w_temp, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(w_text, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(w_hilo, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(w_hum, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(w_wind, LV_OBJ_FLAG_HIDDEN);

    lv_label_set_text(w_city, w.city[0] ? w.city : "--");
    lv_label_set_text(w_icon, weatherIconSymbol(w.icon));
    snprintf(buf, sizeof(buf), "%d°", w.temp);
    lv_label_set_text(w_temp, buf);
    lv_label_set_text(w_text, w.text[0] ? w.text : "未知");
    snprintf(buf, sizeof(buf), "%d° / %d°", w.high, w.low);
    lv_label_set_text(w_hilo, buf);
    snprintf(buf, sizeof(buf), "湿度 %u%%", w.humidity);
    lv_label_set_text(w_hum, buf);
    lv_label_set_text(w_wind, w.wind[0] ? w.wind : "风力 --");
    lv_label_set_text(w_status, "天气已更新");
    lv_obj_set_style_bg_color(w_dot, lv_color_hex(COL_GREEN), 0);

    if (w.updating) {
      lv_label_set_text(w_age, "更新中");
    } else {
      const uint32_t age = weatherAgeSec();
      if (age < 60) lv_label_set_text(w_age, "刚刚");
      else if (age < 3600) {
        snprintf(buf, sizeof(buf), "%u分钟前", (unsigned)(age / 60));
        lv_label_set_text(w_age, buf);
      } else {
        snprintf(buf, sizeof(buf), "%u小时前", (unsigned)(age / 3600));
        lv_label_set_text(w_age, buf);
      }
    }
    lv_obj_align(w_age, LV_ALIGN_TOP_RIGHT, -9, 102);
  } else {
    lv_obj_add_flag(w_icon, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(w_temp, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(w_text, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(w_hilo, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(w_hum, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(w_wind, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(w_msg, LV_OBJ_FLAG_HIDDEN);

    lv_label_set_text(w_city, "天气");
    lv_label_set_text(w_msg, w.error[0] ? w.error : "未配置天气");
    lv_label_set_text(w_status, w.updating ? "更新中" : "未配置");
    lv_label_set_text(w_age, "");
    lv_obj_set_style_bg_color(w_dot, lv_color_hex(COL_AMBER), 0);
  }
  set_compact(w);
  set_forecast(w);
}

void set_clock(const struct tm &t) {
  static const char *week_cn[] = {"周日", "周一", "周二", "周三", "周四", "周五", "周六"};

  char ampm[4] = "";
  char time_buf[8];
  if (gSettings.flags & FLAG_12H) {
    int h = t.tm_hour % 12;
    if (h == 0) h = 12;
    snprintf(time_buf, sizeof(time_buf), "%d:%02d", h, t.tm_min);
    snprintf(ampm, sizeof(ampm), "%s", t.tm_hour < 12 ? "AM" : "PM");
  } else {
    snprintf(time_buf, sizeof(time_buf), "%02d:%02d", t.tm_hour, t.tm_min);
  }
  if (strcmp(time_buf, last_time) != 0) {
    strcpy(last_time, time_buf);
    lv_label_set_text(lbl_time, last_time);
    set_photo_time(last_time);
  }
  if (strcmp(ampm, last_ampm) != 0) {
    strcpy(last_ampm, ampm);
    if (ampm[0]) {
      lv_label_set_text(lbl_ampm, ampm);
      lv_obj_clear_flag(lbl_ampm, LV_OBJ_FLAG_HIDDEN);
    } else {
      lv_obj_add_flag(lbl_ampm, LV_OBJ_FLAG_HIDDEN);
    }
  }

  char date_buf[24];
  snprintf(date_buf, sizeof(date_buf), "%d月%d日", t.tm_mon + 1, t.tm_mday);
  if (strcmp(date_buf, last_date) != 0) {
    strcpy(last_date, date_buf);
    lv_label_set_text(lbl_date, last_date);
    if (w_date) lv_label_set_text(w_date, last_date);
    if (c_date) lv_label_set_text(c_date, last_date);
    if (f_date) lv_label_set_text(f_date, last_date);
  }

  int wd = t.tm_wday;
  if (wd < 0 || wd > 6) wd = 0;
  const char *week = week_cn[wd];
  if (strcmp(week, last_week) != 0) {
    strcpy(last_week, week);
    lv_label_set_text(lbl_week, last_week);
  }

  if (seconds_visible) {
    char sec_buf[4];
    snprintf(sec_buf, sizeof(sec_buf), "%02d", t.tm_sec);
    lv_label_set_text(lbl_sec, sec_buf);
    sec_tick_base_sec = (uint8_t)(t.tm_sec < 0 ? 0 : t.tm_sec);
    sec_tick_base_ms = millis();
    sec_tick_last_ms = sec_tick_base_ms;
    sec_tick_active = true;
    lv_arc_set_value(sec_arc, (int32_t)sec_tick_base_sec * 50);
  }
}

void tick() {
  if (!seconds_visible || !clock_visible || !sec_arc || !sec_tick_active) return;

  const uint32_t now = millis();
  if (now - sec_tick_last_ms < 12) return;
  sec_tick_last_ms = now;

  uint32_t elapsed = now - sec_tick_base_ms;
  if (elapsed >= 1000) elapsed = 999;
  lv_arc_set_value(sec_arc, (int32_t)sec_tick_base_sec * 50 + (int32_t)(elapsed / 20));
}

void set_link(Link link) {
  if (link == last_link) return;
  last_link = link;

  switch (link) {
    case Link::Online:
      lv_label_set_text(lbl_status, "已同步");
      lv_label_set_text(lbl_proto, "NTP");
      lv_obj_set_style_bg_color(dot, lv_color_hex(COL_GREEN), 0);
      break;
    case Link::Offline:
      lv_label_set_text(lbl_status, "离线走时");
      lv_label_set_text(lbl_proto, "RTC");
      lv_obj_set_style_bg_color(dot, lv_color_hex(COL_AMBER), 0);
      break;
  }
}

void set_seconds_visible(bool visible) {
  seconds_visible = visible;
  if (!visible) sec_tick_active = false;
  if (!sec_arc) return;

  if (visible) {
    lv_obj_clear_flag(sec_arc, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(lbl_sec, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(dash, LV_OBJ_FLAG_HIDDEN);
  } else {
    lv_obj_add_flag(sec_arc, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(lbl_sec, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(dash, LV_OBJ_FLAG_HIDDEN);
  }
}

}  // namespace ui

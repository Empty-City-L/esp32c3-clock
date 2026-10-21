/**
 * web_config.h — 内置 Web 配网 / 设置页
 */
#pragma once

#include <Arduino.h>

using BrightnessHandler = void (*)(uint8_t percent);

/* 启动 HTTP 服务（会自动注册一次路由） */
void webBegin(bool apMode, const char *apSsid);

/* 主循环调用：处理 HTTP / DNS / 延迟重启 */
void webLoop();

/* 配网热点是否启用（影响 DNS 劫持与状态显示） */
void webSetApMode(bool apMode);
bool webIsApMode();
const char *webApSsid();

/* 拖动亮度滑杆时的实时预览回调 */
void webSetBrightnessHandler(BrightnessHandler handler);

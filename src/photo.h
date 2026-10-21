/**
 * photo.h — SPIFFS 单文件动画（最多 16 帧，160x128 RGB565）
 *
 * 文件格式：
 *   "ANIM", version=1, frameCount, delayMs, reserved, frame0..frameN
 * 静态图片就是 frameCount=1。
 */
#pragma once

#include <Arduino.h>

static constexpr uint8_t PHOTO_MAX = 1;
static constexpr uint8_t PHOTO_MAX_FRAMES = 16;
static constexpr size_t PHOTO_W = 160;
static constexpr size_t PHOTO_H = 128;
static constexpr size_t PHOTO_FRAME_BYTES = PHOTO_W * PHOTO_H * 2;
static constexpr size_t PHOTO_HEADER_BYTES = 12;

void photoBegin();
void photoRefresh();

uint8_t photoCount();
uint8_t photoFrameCount();
uint16_t photoDelayMs();
bool photoReadFrame(uint8_t index, uint16_t *dst, size_t pixels);

bool photoExists();
bool photoDelete();
bool photoValidate();
bool photoValidatePath(const char *path);
size_t photoSize();

uint8_t photoPageSec();
bool photoSetPageSec(uint8_t sec);
void photoReset();

/* 照片页时间外观 */
struct PhotoStyle {
  bool timeOn;          // 照片页是否显示时间
  uint8_t timeStyle;    // 0=大号44 1=中号30 2=小号22
  uint32_t timeColor;   // 0xRRGGBB
  uint8_t colonStyle;   // 0=原版 1=紧凑圆点 2=细竖条 3=无冒号
  bool gradOn;
  uint32_t gradColor;   // 0xRRGGBB
  uint8_t gradOpa;      // 0..100
  uint8_t gradHeight;   // 48/56/72
};
const PhotoStyle &photoStyle();
bool photoSetStyle(const PhotoStyle &style);

size_t photoTotalBytes();
size_t photoUsedBytes();

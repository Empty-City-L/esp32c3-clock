/**
 * pages.h — 页面显示、顺序、常驻页配置
 *
 * 独立 NVS namespace "pages"，不修改 Settings/Weather/Photo 结构，
 * 避免已有配置因结构变化被重置。
 */
#pragma once

#include <Arduino.h>

static constexpr uint8_t PAGE_COUNT = 4;

enum PageId : uint8_t {
  PAGE_ID_CLOCK = 0,
  PAGE_ID_WEATHER = 1,
  PAGE_ID_FORECAST = 2,
  PAGE_ID_PHOTO = 3,
};

/* resident: 0=不常驻；1=时钟；2=天气；3=3天预报；4=动图 */
struct PageConfig {
  uint8_t order[PAGE_COUNT];  // order[id] = 0..3 位置
  uint8_t visibleMask;        // bit0 时钟, bit1 天气, bit2 预报, bit3 动图
  uint8_t resident;
};

void pagesBegin();
const PageConfig &pagesConfig();
bool pagesSetConfig(const PageConfig &cfg);
void pagesReset();

bool pageVisible(uint8_t id);
uint8_t pageOrder(uint8_t id);
uint8_t residentPage();  // 0=不常驻，1..4

inline bool pageVisibleId(PageId id) { return pageVisible((uint8_t)id); }

#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""时间冒号样式对比预览（设计稿）。"""
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

ROOT = Path(__file__).resolve().parent.parent
DOCS = ROOT / "docs"
DIN = r"C:\Windows\Fonts\DINNextLTPro-Bold.ttf"
DENG = r"C:\Windows\Fonts\Deng.ttf"

S = 4
W, ROW_H = 1800, 170
BG = (7, 12, 21)
TEXT = (242, 247, 255)
MUTED = (143, 160, 188)
TEAL = (94, 234, 212)
LINE = (26, 39, 64)

FONT = ImageFont.truetype(DIN, 44 * S)
LABEL = ImageFont.truetype(DENG, 20)
NOTE = ImageFont.truetype(DENG, 17)
TITLE = ImageFont.truetype(DENG, 24)


def time_metrics(d, x, y):
    bbox = d.textbbox((x, y), "13", font=FONT, anchor="lt")
    h = bbox[3] - bbox[1]
    return h


def draw_time(d, variant, x, y):
    w13 = d.textlength("13", font=FONT)
    w16 = d.textlength("16", font=FONT)
    h = time_metrics(d, x, y)

    if variant == "current":
        d.text((x, y), "13:16", font=FONT, fill=TEXT, anchor="lt")
        return

    if variant == "none":
        gap = 7 * S
        d.text((x, y), "13", font=FONT, fill=TEXT, anchor="lt")
        d.text((x + w13 + gap, y), "16", font=FONT, fill=TEXT, anchor="lt")
        return

    gap = 4 * S
    if variant == "tight":
        gap = 2 * S
        radius = 3.0 * S
    elif variant == "thin":
        gap = 4 * S
        radius = 0
    else:
        radius = 3.5 * S

    colon_w = 7 * S if variant != "thin" else 5 * S
    cx = x + w13 + gap + colon_w / 2
    cy1 = y + h * 0.36
    cy2 = y + h * 0.66

    d.text((x, y), "13", font=FONT, fill=TEXT, anchor="lt")
    if variant == "thin":
        bw = 2.6 * S
        bh = 5.2 * S
        for cy in (cy1, cy2):
            d.rounded_rectangle([cx - bw / 2, cy - bh / 2, cx + bw / 2, cy + bh / 2],
                                radius=bw / 2, fill=TEXT)
    else:
        for cy in (cy1, cy2):
            d.ellipse([cx - radius, cy - radius, cx + radius, cy + radius], fill=TEXT)

    d.text((x + w13 + 2 * gap + colon_w, y), "16", font=FONT, fill=TEXT, anchor="lt")


def main():
    DOCS.mkdir(exist_ok=True)
    rows = [
        ("current", "原版", "DIN 自带方点，粗且偏低"),
        ("round", "圆点冒号", "两颗小圆点，视觉更轻，推荐"),
        ("tight", "紧凑圆点", "圆点更小、更贴近数字"),
        ("thin", "细竖条", "两条细竖线，最克制"),
        ("none", "不要冒号", "只留白，靠间距区分时分"),
    ]
    gap_top = 70
    H = gap_top + len(rows) * ROW_H + 48
    im = Image.new("RGB", (W, H), BG)
    d = ImageDraw.Draw(im)

    d.text((40, 24), "时间冒号样式对比", font=TITLE, fill=TEXT)
    d.text((320, 30), "数字保持 DIN，仅替换中间的冒号", font=NOTE, fill=MUTED)

    for i, (key, name, desc) in enumerate(rows):
        y = gap_top + i * ROW_H
        rec = key == "round"
        if rec:
            d.rounded_rectangle([24, y + 8, W - 24, y + ROW_H - 8], radius=14,
                                fill=(12, 24, 34), outline=(42, 98, 94), width=2)
            d.rectangle([24, y + 28, 30, y + ROW_H - 28], fill=TEAL)
        else:
            d.rectangle([24, y + ROW_H - 1, W - 24, y + ROW_H], fill=LINE)

        d.text((48, y + 42), name, font=LABEL, fill=TEAL if rec else TEXT)
        d.text((48, y + 76), desc, font=NOTE, fill=MUTED)
        if rec:
            d.rounded_rectangle([48, y + 110, 124, y + 136], radius=12,
                                fill=(32, 72, 78))
            d.text((86, y + 123), "推荐", font=NOTE, fill=TEAL, anchor="mm")

        draw_time(d, key, 330, y + 14)
        # 实际 44px 大小参考
        d.text((1510, y + 58), "13:16", font=ImageFont.truetype(DIN, 44),
               fill=TEXT, anchor="lt")
        d.text((1510, y + 112), "实际大小", font=NOTE, fill=MUTED)

    d.text((40, H - 40), "圆点冒号用两个独立圆点替换冒号；颜色仍跟随时间颜色，照片页/时钟页都可共用。",
           font=NOTE, fill=(120, 137, 162))
    im.save(DOCS / "time-colon-preview.png")
    print("saved", DOCS / "time-colon-preview.png")


if __name__ == "__main__":
    main()

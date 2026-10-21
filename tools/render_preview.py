#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""按 LVGL 布局坐标绘制一张设计预览图（非实机截图）。

用法：
    python tools/render_preview.py

依赖 Pillow；输出 docs/preview.png。字体取 Windows 自带的
DIN Next LT Pro Bold 与等线（Deng.ttf）。
"""
from __future__ import annotations

import os
from pathlib import Path

from PIL import Image, ImageDraw, ImageFilter, ImageFont

ROOT = Path(__file__).resolve().parent.parent
OUT = ROOT / "docs" / "preview.png"

DIN = r"C:\Windows\Fonts\DINNextLTPro-Bold.ttf"
DENG = r"C:\Windows\Fonts\Deng.ttf"

W, H = 160, 128
S = 4  # 4x 超采样

BG = (7, 12, 21)
TEXT = (242, 247, 255)
ACCENT = (94, 234, 212)
MUTED = (143, 160, 188)
DIM = (98, 116, 142)
HAIR = (26, 39, 64)
GREEN = (52, 211, 153)


def font(path: str, size: int) -> ImageFont.FreeTypeFont:
    return ImageFont.truetype(path, size * S)


def sc(v: float) -> int:
    return int(round(v * S))


def draw_clock() -> Image.Image:
    im = Image.new("RGB", (W * S, H * S), BG)
    d = ImageDraw.Draw(im)

    f_week = font(DENG, 12)
    f_time = font(DIN, 44)
    f_sec = font(DIN, 16)
    f_status = font(DENG, 12)

    # 顶部：星期 + 日期
    d.text((sc(9), sc(5)), "周六", font=f_week, fill=ACCENT)
    date = "10月18日"
    dw = d.textlength(date, font=f_week)
    d.text((sc(W - 9) - dw, sc(5)), date, font=f_week, fill=MUTED)

    # 分隔线
    d.rectangle([sc(9), sc(24), sc(W - 9), sc(24) + S - 1], fill=HAIR)

    # 大号时间：与 LVGL 一样在 3..115 的 112px 框内居中
    text = "23:25"
    tw = d.textlength(text, font=f_time)
    bbox = f_time.getbbox(text)
    x = sc(2) + (sc(114) - tw) / 2
    y = sc(40) - bbox[1]
    d.text((x, y), text, font=f_time, fill=TEXT)

    # 秒环：270°，开口在下方
    box = [sc(120), sc(37), sc(156), sc(73)]
    d.arc(box, start=135, end=405, fill=HAIR, width=3 * S)
    end = 135 + 270 * 38 / 59
    d.arc(box, start=135, end=end, fill=ACCENT, width=3 * S)
    d.text((sc(138), sc(55)), "38", font=f_sec, fill=ACCENT, anchor="mm")

    # 底部状态
    cx, cy, r = sc(11.5), sc(106.5), sc(2.5)
    d.ellipse([cx - r, cy - r, cx + r, cy + r], fill=GREEN)
    d.text((sc(19), sc(99)), "已同步", font=f_status, fill=MUTED)
    proto = "NTP"
    pw = d.textlength(proto, font=f_status)
    d.text((sc(W - 9) - pw, sc(99)), proto, font=f_status, fill=DIM)

    d.rectangle([sc(9), sc(120), sc(W - 9), sc(120) + S - 1], fill=HAIR)
    return im


def make_canvas(screen: Image.Image) -> Image.Image:
    """给屏幕加一个很淡的深色边框和光晕，方便在 README 里展示。"""
    pad_x, pad_top, pad_bottom = 72, 60, 92
    cw = screen.width + pad_x * 2
    ch = screen.height + pad_top + pad_bottom
    canvas = Image.new("RGB", (cw, ch), (8, 13, 22))
    d = ImageDraw.Draw(canvas)

    # 顶部青色氛围光
    glow = Image.new("L", (cw, ch), 0)
    gd = ImageDraw.Draw(glow)
    gd.ellipse([cw * 0.48, -ch * 0.35, cw * 1.15, ch * 0.55], fill=48)
    glow = glow.filter(ImageFilter.GaussianBlur(70))
    canvas.paste(Image.new("RGB", (cw, ch), (10, 38, 44)), (0, 0), glow)

    # 屏幕阴影
    shadow = Image.new("RGBA", (cw, ch), (0, 0, 0, 0))
    sd = ImageDraw.Draw(shadow)
    sd.rounded_rectangle(
        [pad_x - 14, pad_top - 14, pad_x + screen.width + 14, pad_top + screen.height + 14],
        radius=26,
        fill=(0, 0, 0, 190),
    )
    shadow = shadow.filter(ImageFilter.GaussianBlur(24))
    canvas = Image.alpha_composite(canvas.convert("RGBA"), shadow).convert("RGB")

    # 屏幕边框
    d = ImageDraw.Draw(canvas)
    d.rounded_rectangle(
        [pad_x - 5, pad_top - 5, pad_x + screen.width + 5, pad_top + screen.height + 5],
        radius=12,
        fill=(18, 28, 44),
        outline=(42, 62, 90),
        width=2,
    )
    canvas.paste(screen, (pad_x, pad_top))

    # 底部说明
    caption = "ESP32-C3  ·  ST7735S  ·  160 × 128"
    cf = ImageFont.truetype(DENG, 23)
    tw = d.textlength(caption, font=cf)
    d.text(((cw - tw) / 2, ch - 52), caption, font=cf, fill=(110, 128, 152))
    return canvas


def main() -> None:
    if not os.path.exists(DIN) or not os.path.exists(DENG):
        raise SystemExit("需要 Windows 自带的 DIN Next LT Pro Bold 与 Deng.ttf")
    screen = draw_clock()
    canvas = make_canvas(screen)
    OUT.parent.mkdir(parents=True, exist_ok=True)
    canvas.save(OUT, "PNG")
    print(f"saved {OUT} ({canvas.width}x{canvas.height})")


if __name__ == "__main__":
    main()

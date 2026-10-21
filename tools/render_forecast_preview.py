#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""3 天天气页 UI 预览（设计稿，非实机）。"""
from pathlib import Path

from PIL import Image, ImageDraw, ImageFilter, ImageFont

ROOT = Path(__file__).resolve().parent.parent
DOCS = ROOT / "docs"
DIN = r"C:\Windows\Fonts\DINNextLTPro-Bold.ttf"
DENG = r"C:\Windows\Fonts\Deng.ttf"
SYM = r"C:\Windows\Fonts\seguisym.ttf"

W, H = 160, 128
S = 4
BG = (7, 12, 21)
TEXT = (242, 247, 255)
ACCENT = (94, 234, 212)
MUTED = (143, 160, 188)
DIM = (98, 116, 142)
HAIR = (26, 39, 64)
GREEN = (52, 211, 153)


def sc(v):
    return int(round(v * S))


def font(path, size):
    return ImageFont.truetype(path, size * S)


def text(d, xy, s, f, fill, anchor="la"):
    d.text((sc(xy[0]), sc(xy[1])), s, font=f, fill=fill, anchor=anchor)


def right(d, x, y, s, f, fill):
    w = d.textlength(s, font=f)
    d.text((sc(x) - w, sc(y)), s, font=f, fill=fill)


def hairline(d, y):
    d.rectangle([sc(9), sc(y), sc(151), sc(y) + S - 1], fill=HAIR)


def dots(d, active, count=4):
    positions = [70, 78, 86, 94] if count == 3 else [66, 74, 82, 90]
    for i, x in enumerate(positions[:count]):
        c = ACCENT if i == active else HAIR
        d.ellipse([sc(x - 2), sc(121 - 2), sc(x + 2), sc(121 + 2)], fill=c)


def header(d):
    text(d, (9, 5), "邵阳", font(DENG, 12), ACCENT)
    right(d, 151, 5, "10月19日", font(DENG, 12), MUTED)
    hairline(d, 23)


def icon(d, center_x, y, symbol, size=28, color=ACCENT):
    f = font(SYM, size)
    w = d.textlength(symbol, font=f)
    d.text((sc(center_x) - w / 2, sc(y)), symbol, font=f, fill=color)


def forecast_page():
    im = Image.new("RGB", (W * S, H * S), BG)
    d = ImageDraw.Draw(im)
    header(d)

    cols = [32, 80, 128]
    days = [("今天", "多云", "24°", "16°", "⛅"),
            ("明天", "小雨", "22°", "15°", "☔"),
            ("后天", "阴", "21°", "14°", "☁")]
    for i, (day, cond, hi, lo, sym) in enumerate(days):
        cx = cols[i]
        acc = ACCENT if i == 0 else MUTED
        text(d, (cx, 27), day, font(DENG, 12), acc, anchor="ma")
        icon(d, cx, 40, sym, 26, ACCENT if i == 0 else MUTED)
        text(d, (cx, 70), cond, font(DENG, 12), MUTED, anchor="ma")

        f_hi = font(DIN, 22)
        f_lo = font(DENG, 12)
        text(d, (cx, 84), hi, f_hi, TEXT, anchor="ma")
        text(d, (cx, 104), lo, f_lo, DIM, anchor="ma")

    return im


def compact_page():
    im = Image.new("RGB", (W * S, H * S), BG)
    d = ImageDraw.Draw(im)

    text(d, (9, 4), "邵阳", font(DENG, 12), ACCENT)
    right(d, 151, 4, "10月19日", font(DENG, 12), MUTED)
    hairline(d, 20)

    icon(d, 18, 23, "⛅", 22)
    f_temp = font(DIN, 30)
    d.text((sc(38), sc(21)), "19°", font=f_temp, fill=TEXT)
    text(d, (38, 51), "多云", font(DENG, 12), ACCENT)
    text(d, (94, 26), "24° / 16°", font(DENG, 12), MUTED)
    text(d, (94, 44), "湿度 76%", font(DENG, 12), DIM)
    hairline(d, 68)

    cols = [32, 80, 128]
    days = [("今天", "24/16", "⛅"), ("明天", "22/15", "☔"), ("后天", "21/14", "☁")]
    for i, (day, hl, sym) in enumerate(days):
        cx = cols[i]
        text(d, (cx, 70), day, font(DENG, 10), ACCENT if i == 0 else MUTED, anchor="ma")
        icon(d, cx, 83, sym, 18, ACCENT if i == 0 else MUTED)
        text(d, (cx, 100), hl, font(DENG, 10), MUTED, anchor="ma")

    return im


def panel(screen, caption):
    pad_x, pad_top, pad_bottom = 40, 40, 78
    pw = screen.width + pad_x * 2
    ph = screen.height + pad_top + pad_bottom
    canvas = Image.new("RGB", (pw, ph), (8, 13, 22))
    shadow = Image.new("RGBA", (pw, ph), (0, 0, 0, 0))
    sd = ImageDraw.Draw(shadow)
    sd.rounded_rectangle([pad_x - 10, pad_top - 10, pad_x + screen.width + 10,
                          pad_top + screen.height + 10], radius=22, fill=(0, 0, 0, 170))
    shadow = shadow.filter(ImageFilter.GaussianBlur(20))
    canvas = Image.alpha_composite(canvas.convert("RGBA"), shadow).convert("RGB")
    d = ImageDraw.Draw(canvas)
    d.rounded_rectangle([pad_x - 4, pad_top - 4, pad_x + screen.width + 4,
                         pad_top + screen.height + 4], radius=11,
                        fill=(18, 28, 44), outline=(42, 62, 90), width=2)
    canvas.paste(screen, (pad_x, pad_top))
    cf = ImageFont.truetype(DENG, 21)
    tw = d.textlength(caption, font=cf)
    d.text(((pw - tw) / 2, ph - 50), caption, font=cf, fill=(145, 162, 185))
    return canvas


def main():
    DOCS.mkdir(exist_ok=True)
    p_a = panel(forecast_page(), "方案 A · 独立 3 天预报页（推荐）")
    p_b = panel(compact_page(), "方案 B · 合并到天气页（3 页轮播）")
    p_a.save(DOCS / "forecast-page.png")
    p_b.save(DOCS / "forecast-compact.png")
    gap = 26
    sheet = Image.new("RGB", (p_a.width + p_b.width + gap, max(p_a.height, p_b.height)),
                      (6, 10, 17))
    sheet.paste(p_a, (0, 0))
    sheet.paste(p_b, (p_a.width + gap, 0))
    sheet.save(DOCS / "forecast-preview.png")
    print("saved forecast previews")


if __name__ == "__main__":
    main()

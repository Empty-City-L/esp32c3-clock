#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""生成天气 UI 方案预览图（设计稿，非实机截图）。

输出：
  docs/weather-option-a.png        同屏：时钟 + 天气侧栏
  docs/weather-option-b-clock.png  双页：时钟页
  docs/weather-option-b-weather.png 双页：天气页
  docs/weather-ui-options.png      三张对比图
"""
from __future__ import annotations

from pathlib import Path

from PIL import Image, ImageDraw, ImageFilter, ImageFont

ROOT = Path(__file__).resolve().parent.parent
DOCS = ROOT / "docs"
DIN = r"C:\Windows\Fonts\DINNextLTPro-Bold.ttf"
DENG = r"C:\Windows\Fonts\Deng.ttf"

W, H = 160, 128
S = 4

BG = (7, 12, 21)
TEXT = (242, 247, 255)
ACCENT = (94, 234, 212)
MUTED = (143, 160, 188)
DIM = (98, 116, 142)
HAIR = (26, 39, 64)
GREEN = (52, 211, 153)
YELLOW = (251, 191, 36)
CLOUD = (220, 235, 255)
BLUE = (96, 165, 250)


def font(path: str, size: int) -> ImageFont.FreeTypeFont:
    return ImageFont.truetype(path, size * S)


def sc(v: float) -> int:
    return int(round(v * S))


def text(d: ImageDraw.ImageDraw, xy, s: str, f, fill, anchor: str = "la") -> None:
    d.text((sc(xy[0]), sc(xy[1])), s, font=f, fill=fill, anchor=anchor)


def right_text(d: ImageDraw.ImageDraw, x_right: float, y: float, s: str, f, fill) -> None:
    w = d.textlength(s, font=f)
    d.text((sc(x_right) - w, sc(y)), s, font=f, fill=fill, anchor="la")


def hairline(d: ImageDraw.ImageDraw, x1: float, y: float, x2: float) -> None:
    d.rectangle([sc(x1), sc(y), sc(x2), sc(y) + S - 1], fill=HAIR)


def weather_icon(d: ImageDraw.ImageDraw, x: float, y: float, size: float) -> None:
    """简单的太阳 + 云朵图标，仅用于预览。"""
    s = size
    # sun
    cx, cy, r = x + s * 0.38, y + s * 0.34, s * 0.24
    d.ellipse([sc(cx - r), sc(cy - r), sc(cx + r), sc(cy + r)], fill=YELLOW)
    # rays
    for dx, dy in ((-0.42, 0), (0.42, 0), (0, -0.42), (0, 0.42),
                   (-0.3, -0.3), (0.3, -0.3), (-0.3, 0.3), (0.3, 0.3)):
        d.line([sc(cx + dx * s * 0.8), sc(cy + dy * s * 0.8),
                sc(cx + dx * s * 1.15), sc(cy + dy * s * 1.15)],
               fill=YELLOW, width=max(1, S // 2))
    # cloud
    d.ellipse([sc(x + s * 0.12), sc(y + s * 0.48), sc(x + s * 0.55), sc(y + s * 0.82)], fill=CLOUD)
    d.ellipse([sc(x + s * 0.32), sc(y + s * 0.36), sc(x + s * 0.75), sc(y + s * 0.78)], fill=CLOUD)
    d.ellipse([sc(x + s * 0.52), sc(y + s * 0.5), sc(x + s * 0.9), sc(y + s * 0.82)], fill=CLOUD)
    d.rectangle([sc(x + s * 0.18), sc(y + s * 0.66), sc(x + s * 0.84), sc(y + s * 0.82)], fill=CLOUD)


def sec_ring(d: ImageDraw.ImageDraw, x: float, y: float, size: float, sec: int) -> None:
    box = [sc(x), sc(y), sc(x + size), sc(y + size)]
    d.arc(box, start=135, end=405, fill=HAIR, width=3 * S)
    d.arc(box, start=135, end=135 + 270 * sec / 59, fill=ACCENT, width=3 * S)
    f = font(DIN, 16)
    d.text((sc(x + size / 2), sc(y + size / 2)), f"{sec:02d}", font=f, fill=ACCENT, anchor="mm")


def header(d: ImageDraw.ImageDraw, left: str, right: str) -> None:
    f = font(DENG, 12)
    d.text((sc(9), sc(5)), left, font=f, fill=ACCENT)
    right_text(d, 151, 5, right, f, MUTED)
    hairline(d, 9, 23, 151)


def status(d: ImageDraw.ImageDraw, label: str, right: str = "NTP") -> None:
    f = font(DENG, 12)
    cx, cy, r = sc(11.5), sc(107), sc(2.5)
    d.ellipse([cx - r, cy - r, cx + r, cy + r], fill=GREEN)
    d.text((sc(19), sc(101)), label, font=f, fill=MUTED)
    if right:
        right_text(d, 151, 101, right, f, DIM)
    hairline(d, 9, 120, 151)


def clock_screen() -> Image.Image:
    im = Image.new("RGB", (W * S, H * S), BG)
    d = ImageDraw.Draw(im)
    header(d, "周六", "10月19日")

    f_time = font(DIN, 44)
    t = "23:25"
    tw = d.textlength(t, font=f_time)
    bbox = f_time.getbbox(t)
    d.text((sc(2) + (sc(114) - tw) / 2, sc(40) - bbox[1]), t, font=f_time, fill=TEXT)

    sec_ring(d, 120, 37, 36, 38)
    status(d, "已同步", "NTP")
    return im


def option_a() -> Image.Image:
    """同屏：时间左侧，天气侧栏在右。"""
    im = Image.new("RGB", (W * S, H * S), BG)
    d = ImageDraw.Draw(im)
    header(d, "周六", "10月19日")

    f_time = font(DIN, 40)
    t = "23:25"
    tw = d.textlength(t, font=f_time)
    bbox = f_time.getbbox(t)
    d.text((sc(2) + (sc(106) - tw) / 2, sc(32) - bbox[1]), t, font=f_time, fill=TEXT)

    weather_icon(d, 124, 27, 20)
    f_temp = font(DIN, 24)
    temp = "26°"
    tw = d.textlength(temp, font=f_temp)
    bbox = f_temp.getbbox(temp)
    d.text((sc(151) - tw, sc(52) - bbox[1]), temp, font=f_temp, fill=TEXT)

    f_cn = font(DENG, 12)
    right_text(d, 151, 80, "多云", f_cn, MUTED)
    right_text(d, 151, 98, "18°/27°", f_cn, DIM)

    sec_ring(d, 70, 82, 28, 38)
    f = font(DENG, 12)
    cx, cy, r = sc(11.5), sc(107), sc(2.5)
    d.ellipse([cx - r, cy - r, cx + r, cy + r], fill=GREEN)
    d.text((sc(19), sc(101)), "已同步", font=f, fill=MUTED)
    hairline(d, 9, 120, 151)
    return im


def option_b_weather() -> Image.Image:
    """天气页：城市、图标、温度、天气、最高最低、湿度。"""
    im = Image.new("RGB", (W * S, H * S), BG)
    d = ImageDraw.Draw(im)
    header(d, "北京", "10月19日")

    weather_icon(d, 10, 30, 28)

    f_cn14 = font(DENG, 14)
    f_cn12 = font(DENG, 12)
    d.text((sc(10), sc(64)), "多云", font=f_cn14, fill=ACCENT)
    d.text((sc(10), sc(88)), "湿度 45%", font=f_cn12, fill=MUTED)

    f_temp = font(DIN, 44)
    temp = "26°"
    bbox = f_temp.getbbox(temp)
    d.text((sc(54), sc(33) - bbox[1]), temp, font=f_temp, fill=TEXT)

    d.text((sc(54), sc(70)), "18° / 27°", font=f_cn12, fill=MUTED)
    d.text((sc(62), sc(88)), "东南风 2级", font=f_cn12, fill=DIM)

    f = font(DENG, 12)
    cx, cy, r = sc(11.5), sc(109), sc(2.5)
    d.ellipse([cx - r, cy - r, cx + r, cy + r], fill=GREEN)
    d.text((sc(19), sc(103)), "天气已更新", font=f, fill=MUTED)
    right_text(d, 151, 103, "2分钟前", f, DIM)

    # page dots: clock / weather
    for i, x in enumerate((74, 82)):
        c = ACCENT if i == 1 else HAIR
        d.ellipse([sc(x), sc(118), sc(x + 4), sc(118 + 4)], fill=c)
    return im


def panel(screen: Image.Image, caption: str) -> Image.Image:
    pad_x, pad_top, pad_bottom = 40, 40, 78
    pw = screen.width + pad_x * 2
    ph = screen.height + pad_top + pad_bottom
    canvas = Image.new("RGB", (pw, ph), (8, 13, 22))
    d = ImageDraw.Draw(canvas)

    shadow = Image.new("RGBA", (pw, ph), (0, 0, 0, 0))
    sd = ImageDraw.Draw(shadow)
    sd.rounded_rectangle([pad_x - 12, pad_top - 12, pad_x + screen.width + 12,
                          pad_top + screen.height + 12], radius=24, fill=(0, 0, 0, 180))
    shadow = shadow.filter(ImageFilter.GaussianBlur(22))
    canvas = Image.alpha_composite(canvas.convert("RGBA"), shadow).convert("RGB")
    d = ImageDraw.Draw(canvas)

    d.rounded_rectangle([pad_x - 5, pad_top - 5, pad_x + screen.width + 5,
                         pad_top + screen.height + 5], radius=12,
                        fill=(18, 28, 44), outline=(42, 62, 90), width=2)
    canvas.paste(screen, (pad_x, pad_top))

    cf = ImageFont.truetype(DENG, 21)
    tw = d.textlength(caption, font=cf)
    d.text(((pw - tw) / 2, ph - 50), caption, font=cf, fill=(145, 162, 185))
    return canvas


def main() -> None:
    DOCS.mkdir(exist_ok=True)
    a = option_a()
    bc = clock_screen()
    bw = option_b_weather()

    pa = panel(a, "方案 A · 时钟 + 天气侧栏")
    pbc = panel(bc, "方案 B · 时钟页（保持不变）")
    pbw = panel(bw, "方案 B · 天气页（自动轮播）")

    pa.save(DOCS / "weather-option-a.png")
    pbc.save(DOCS / "weather-option-b-clock.png")
    pbw.save(DOCS / "weather-option-b-weather.png")

    gap = 26
    cw = pa.width * 3 + gap * 2
    ch = pa.height
    sheet = Image.new("RGB", (cw, ch), (6, 10, 17))
    sheet.paste(pa, (0, 0))
    sheet.paste(pbc, (pa.width + gap, 0))
    sheet.paste(pbw, ((pa.width + gap) * 2, 0))
    sheet.save(DOCS / "weather-ui-options.png")

    print("saved:")
    for n in ("weather-option-a.png", "weather-option-b-clock.png",
              "weather-option-b-weather.png", "weather-ui-options.png"):
        print(" ", DOCS / n)


if __name__ == "__main__":
    main()

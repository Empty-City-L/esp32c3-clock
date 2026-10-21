#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""照片页时间样式/颜色/渐变 UI 预览（设计稿）。"""
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

ROOT = Path(__file__).resolve().parent.parent
DOCS = ROOT / "docs"
TMP = Path(r"C:\Users\foam\AppData\Local\Temp\clock_shots")
DIN = r"C:\Windows\Fonts\DINNextLTPro-Bold.ttf"
DENG = r"C:\Windows\Fonts\Deng.ttf"

W, H, S = 160, 128, 4
WHITE = (242, 247, 255)
TEAL = (94, 234, 212)
AMBER = (251, 191, 36)
PINK = (244, 114, 182)
BLUE = (96, 165, 250)
MUTED = (143, 160, 188)
DIM = (98, 116, 142)
BG = (7, 12, 21)


def f(path, size):
    return ImageFont.truetype(path, size * S)


def gradient_overlay(base, color, strength, height):
    """底部渐隐：顶部透明，底部 color 的 strength 不透明度。"""
    w, h = base.size
    gh = height * S
    top = h - gh
    over = Image.new("RGBA", base.size, (0, 0, 0, 0))
    px = over.load()
    for y in range(top, h):
        t = (y - top) / max(1, gh - 1)
        # 平滑一点，底部更实
        a = int(255 * strength * (t ** 1.15))
        for x in range(w):
            px[x, y] = (color[0], color[1], color[2], a)
    return Image.alpha_composite(base.convert("RGBA"), over).convert("RGB")


def screen(style, color, gradient=None):
    base = Image.open(TMP / "base_photo.png").convert("RGB")
    if gradient:
        base = gradient_overlay(base, gradient[0], gradient[1], gradient[2])
    d = ImageDraw.Draw(base)

    if style == "large":
        d.text((9 * S, 88 * S), "13:16", font=f(DIN, 44), fill=color)
    elif style == "medium":
        d.text((9 * S, 98 * S), "13:16", font=f(DIN, 30), fill=color)
    else:
        d.text((151 * S, 98 * S), "13:16", font=f(DIN, 22), fill=color, anchor="ra")
    return base


def panel(img, caption):
    pad = 20
    cw, ch = img.width + pad * 2, img.height + pad * 2 + 46
    canvas = Image.new("RGB", (cw, ch), (8, 13, 22))
    d = ImageDraw.Draw(canvas)
    d.rounded_rectangle([pad - 4, pad - 4, pad + img.width + 4, pad + img.height + 4],
                        radius=8, fill=(18, 28, 44), outline=(42, 62, 90), width=2)
    canvas.paste(img, (pad, pad))
    cf = ImageFont.truetype(DENG, 18)
    tw = d.textlength(caption, font=cf)
    d.text(((cw - tw) / 2, ch - 34), caption, font=cf, fill=(145, 162, 185))
    return canvas


def web_mock(width):
    h = 250
    im = Image.new("RGB", (width, h), (8, 13, 22))
    d = ImageDraw.Draw(im)
    d.rounded_rectangle([8, 8, width - 8, h - 8], radius=14,
                        fill=(14, 23, 38), outline=(38, 56, 82), width=2)

    tf = ImageFont.truetype(DENG, 20)
    sf = ImageFont.truetype(DENG, 16)
    mf = ImageFont.truetype(DENG, 14)
    d.text((30, 24), "网页端 · 照片页时间", font=tf, fill=(235, 242, 255))

    # 样式
    d.text((30, 66), "时间样式", font=sf, fill=(150, 167, 192))
    chips = [("大号左下", 118, True), ("中号左下", 226, False), ("小号右下", 334, False)]
    for label, x, on in chips:
        d.rounded_rectangle([x, 58, x + 96, 88], radius=15,
                            fill=(24, 42, 64) if not on else (32, 72, 78),
                            outline=(94, 234, 212) if on else (48, 70, 98), width=2)
        d.text((x + 48, 73), label, font=mf, fill=(235, 242, 255), anchor="mm")

    # 颜色
    d.text((470, 66), "时间颜色", font=sf, fill=(150, 167, 192))
    swatches = [WHITE, TEAL, AMBER, PINK, BLUE]
    for i, c in enumerate(swatches):
        x = 566 + i * 34
        d.ellipse([x, 58, x + 22, 80], fill=c, outline=(255, 255, 255) if i == 1 else None, width=2)
    d.text((744, 61), "#5EEAD4", font=mf, fill=(145, 162, 185))

    # 渐变
    d.text((30, 118), "渐变", font=sf, fill=(150, 167, 192))
    # 开关
    d.rounded_rectangle([96, 112, 142, 138], radius=13, fill=(32, 72, 78), outline=(94, 234, 212), width=2)
    d.ellipse([120, 115, 139, 134], fill=(94, 234, 212))
    d.text((150, 117), "开", font=mf, fill=(94, 234, 212))

    d.text((204, 118), "颜色", font=sf, fill=(150, 167, 192))
    d.rounded_rectangle([248, 113, 338, 138], radius=10, fill=(16, 28, 44), outline=(48, 70, 98), width=1)
    d.ellipse([258, 118, 274, 134], fill=(0, 0, 0), outline=(120, 140, 170))
    d.text((282, 117), "纯黑", font=mf, fill=(220, 230, 245))

    d.text((368, 118), "强度", font=sf, fill=(150, 167, 192))
    d.rounded_rectangle([412, 123, 572, 129], radius=3, fill=(36, 54, 78))
    d.rounded_rectangle([412, 123, 518, 129], radius=3, fill=(94, 234, 212))
    d.ellipse([512, 118, 524, 130], fill=(235, 242, 255))
    d.text((584, 117), "60%", font=mf, fill=(235, 242, 255))

    d.text((30, 166), "高度", font=sf, fill=(150, 167, 192))
    d.rounded_rectangle([96, 161, 186, 186], radius=10, fill=(16, 28, 44), outline=(48, 70, 98), width=1)
    d.text((141, 173), "56 px", font=mf, fill=(220, 230, 245), anchor="mm")

    d.text((30, 208), "所有选项保存到设备 NVS；时间颜色支持取色器/十六进制，渐变支持开关、颜色、强度、高度。",
           font=mf, fill=(120, 137, 162))
    return im


def main():
    variants = [
        ("large", WHITE, ((0, 0, 0), 0.60, 56),
         "S1 大号左下 · 白色 · 黑色渐变 60%"),
        ("medium", TEAL, None,
         "S2 中号左下 · 青色 · 渐变关闭"),
        ("small", AMBER, ((8, 20, 42), 0.45, 48),
         "S3 小号右下 · 琥珀 · 深蓝渐变 45%"),
    ]
    panels = []
    for style, color, grad, cap in variants:
        panels.append(panel(screen(style, color, grad), cap))

    gap = 18
    top_w = sum(p.width for p in panels) + gap * (len(panels) - 1)
    top_h = max(p.height for p in panels)
    web = web_mock(top_w)
    out = Image.new("RGB", (top_w, top_h + 16 + web.height), (6, 10, 17))
    x = 0
    for p in panels:
        out.paste(p, (x, 0))
        x += p.width + gap
    out.paste(web, (0, top_h + 16))
    out.save(DOCS / "photo-style-preview.png")
    print("saved", DOCS / "photo-style-preview.png")


if __name__ == "__main__":
    main()

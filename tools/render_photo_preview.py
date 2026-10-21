#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""照片页 + Web 上传压缩 UI 预览（设计稿，非实机）。"""
from __future__ import annotations

import math
import random
from pathlib import Path

import numpy as np
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
YELLOW = (251, 191, 36)


def sc(v: float) -> int:
    return int(round(v * S))


def font(path: str, size: int) -> ImageFont.FreeTypeFont:
    return ImageFont.truetype(path, size * S)


def make_photo(w: int = 900, h: int = 720, variant: int = 0) -> Image.Image:
    """合成一张风景照片，避免使用外部图片版权。"""
    rng = random.Random(20261019 + variant * 17)
    sky_top = [(20, 36, 67), (31, 44, 83), (42, 35, 70)][variant]
    sky_bottom = [(248, 172, 120), (238, 147, 101), (211, 117, 105)][variant]
    horizon = int(h * 0.62)

    arr = np.zeros((h, w, 3), dtype=np.float32)
    for y in range(h):
        t = min(1.0, y / max(1, horizon))
        t = t ** 0.72
        arr[y, :, 0] = sky_top[0] * (1 - t) + sky_bottom[0] * t
        arr[y, :, 1] = sky_top[1] * (1 - t) + sky_bottom[1] * t
        arr[y, :, 2] = sky_top[2] * (1 - t) + sky_bottom[2] * t

    img = Image.fromarray(np.clip(arr, 0, 255).astype(np.uint8), "RGB")
    d = ImageDraw.Draw(img, "RGBA")

    # sun + glow
    sx, sy, sr = w * 0.72, h * 0.39, h * 0.075
    for i in range(18, 0, -1):
        r = sr * (1 + i * 0.22)
        alpha = int(5 + 12 * (1 - i / 18))
        d.ellipse([sx - r, sy - r, sx + r, sy + r], fill=(255, 196, 120, alpha))
    d.ellipse([sx - sr, sy - sr, sx + sr, sy + sr], fill=(255, 224, 168, 255))
    d.ellipse([sx - sr * 0.7, sy - sr * 0.7, sx + sr * 0.7, sy + sr * 0.7],
              fill=(255, 242, 207, 255))

    # distant mountains
    def mountain(base_y, amp, color, step, phase):
        pts = [(-20, h)]
        for x in range(-20, w + 30, step):
            yy = base_y + amp * math.sin(x / w * math.pi * 2.2 + phase) \
                 + amp * 0.45 * math.sin(x / w * math.pi * 5.1 + phase * 1.7)
            pts.append((x, yy))
        pts.append((w + 20, h))
        d.polygon(pts, fill=color)

    mountain(horizon - h * 0.02, h * 0.10, (47, 62, 89, 255), 26, 0.7)
    mountain(horizon + h * 0.05, h * 0.075, (33, 45, 68, 255), 22, 2.1)
    mountain(horizon + h * 0.12, h * 0.055, (22, 31, 48, 255), 18, 4.0)

    # lake
    lake_top = horizon + h * 0.16
    arr2 = np.zeros((h, w, 3), dtype=np.float32)
    for y in range(h):
        t = min(1.0, max(0.0, (y - lake_top) / max(1, h - lake_top)))
        arr2[y, :, 0] = 44 * (1 - t) + 14 * t
        arr2[y, :, 1] = 65 * (1 - t) + 24 * t
        arr2[y, :, 2] = 95 * (1 - t) + 38 * t
    lake = Image.fromarray(np.clip(arr2, 0, 255).astype(np.uint8), "RGB")
    mask = Image.new("L", (w, h), 0)
    md = ImageDraw.Draw(mask)
    md.rectangle([0, lake_top, w, h], fill=255)
    img.paste(lake, (0, 0), mask)
    d = ImageDraw.Draw(img, "RGBA")

    # sun reflection
    for i in range(30):
        yy = lake_top + i * (h - lake_top) / 30
        ww = w * (0.035 + 0.07 * i / 30)
        alpha = max(8, 90 - i * 3)
        d.ellipse([sx - ww, yy - 2, sx + ww, yy + 2], fill=(255, 214, 150, alpha))
    for i in range(9):
        yy = lake_top + 10 + i * 9
        d.line([sx - w * 0.11, yy, sx + w * 0.11, yy], fill=(255, 235, 200, 22), width=2)

    # foreground
    d.polygon([(-20, h), (-20, h * 0.87), (w * 0.22, h * 0.91), (w * 0.38, h),
               (-20, h)], fill=(9, 15, 24, 255))
    d.polygon([(w + 20, h), (w + 20, h * 0.9), (w * 0.78, h * 0.94), (w * 0.62, h),
               (w + 20, h)], fill=(9, 15, 24, 255))

    # film grain
    noise = np.random.default_rng(7 + variant).normal(0, 3.2, (h, w, 1))
    arr3 = np.asarray(img).astype(np.float32) + noise
    img = Image.fromarray(np.clip(arr3, 0, 255).astype(np.uint8), "RGB")
    return img


def device_screen(photo: Image.Image, overlay: bool) -> Image.Image:
    im = photo.resize((W * S, H * S), Image.LANCZOS).convert("RGBA")
    d = ImageDraw.Draw(im, "RGBA")

    if overlay:
        # bottom gradient
        grad = Image.new("RGBA", (W * S, H * S), (0, 0, 0, 0))
        gd = ImageDraw.Draw(grad)
        top = sc(78)
        for y in range(top, H * S):
            t = (y - top) / (H * S - top)
            a = int(15 + 220 * (t ** 1.35))
            gd.line([(0, y), (W * S, y)], fill=(3, 7, 13, a))
        im = Image.alpha_composite(im, grad)
        d = ImageDraw.Draw(im, "RGBA")

        # 只要时间
        f_time = font(DIN, 40)
        bbox = f_time.getbbox("23:25")
        d.text((sc(9), sc(88) - bbox[1]), "23:25", font=f_time, fill=TEXT)

    # 3 page dots: clock / weather / photo
    dots = [(74, 121), (82, 121), (90, 121)]
    # subtle dark pill
    d.rounded_rectangle([sc(68), sc(117), sc(96), sc(125)], radius=sc(4),
                        fill=(3, 7, 13, 115))
    for i, (x, y) in enumerate(dots):
        active = i == 2
        r = 2.2
        color = ACCENT if active else (210, 224, 240, 130)
        d.ellipse([sc(x - r), sc(y - r), sc(x + r), sc(y + r)], fill=color)
    return im.convert("RGB")


def panel(screen: Image.Image, caption: str) -> Image.Image:
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


def web_card(photo: Image.Image) -> Image.Image:
    cw, ch = 1180, 760
    im = Image.new("RGB", (cw, ch), (6, 10, 17))
    d = ImageDraw.Draw(im, "RGBA")

    # glow
    for r in range(520, 80, -40):
        d.ellipse([cw * 0.5 - r, -r * 0.55, cw * 0.5 + r, r * 0.55],
                  fill=(30, 80, 110, 3))

    card = [52, 48, cw - 52, ch - 48]
    d.rounded_rectangle(card, radius=26, fill=(11, 19, 32, 255),
                        outline=(32, 50, 77, 255), width=2)

    f_title = ImageFont.truetype(DENG, 27)
    f_sub = ImageFont.truetype(DENG, 13)
    f_label = ImageFont.truetype(DENG, 14)
    f_small = ImageFont.truetype(DENG, 12)
    f_btn = ImageFont.truetype(DENG, 14)
    f_num = ImageFont.truetype(DIN, 20)

    # header icon
    d.rounded_rectangle([88, 82, 136, 130], radius=14, fill=(17, 40, 48),
                        outline=(54, 130, 120), width=1)
    d.ellipse([104, 98, 110, 104], fill=ACCENT)
    d.rectangle([104, 108, 120, 114], fill=ACCENT)
    d.polygon([(106, 114), (116, 102), (124, 114)], fill=ACCENT)

    d.text((154, 84), "照片 · 相册", font=f_title, fill=TEXT)
    d.text((156, 116), "浏览器端自动裁切 160×128、压缩成 JPEG 后上传", font=f_sub, fill=MUTED)
    d.text((cw - 116, 88), "06", font=ImageFont.truetype(DIN, 18), fill=(94, 234, 212))

    # photo preview frame
    px, py, pw, ph = 88, 170, 430, 344
    d.rounded_rectangle([px - 1, py - 1, px + pw + 1, py + ph + 1], radius=15,
                        fill=(20, 34, 52), outline=(48, 72, 104), width=2)
    pv = photo.resize((pw, ph), Image.LANCZOS)
    mask = Image.new("L", (pw, ph), 0)
    ImageDraw.Draw(mask).rounded_rectangle([0, 0, pw, ph], radius=13, fill=255)
    im.paste(pv, (px, py), mask)

    # crop guides
    d.rectangle([px + 8, py + 8, px + pw - 8, py + ph - 8],
                outline=(255, 255, 255, 55), width=1)
    for gx in (px + pw // 3, px + 2 * pw // 3):
        d.line([gx, py + 8, gx, py + ph - 8], fill=(255, 255, 255, 25))
    for gy in (py + ph // 3, py + 2 * ph // 3):
        d.line([px + 8, gy, px + pw - 8, gy], fill=(255, 255, 255, 25))

    d.rounded_rectangle([px + 12, py + 12, px + 116, py + 40], radius=8,
                        fill=(4, 10, 18, 190))
    d.text((px + 22, py + 18), "160 × 128", font=f_small, fill=ACCENT)
    d.rounded_rectangle([px + pw - 112, py + 12, px + pw - 12, py + 40], radius=8,
                        fill=(4, 10, 18, 190))
    d.text((px + pw - 102, py + 18), "自动裁切", font=f_small, fill=(230, 240, 250))

    d.text((px, py + ph + 14), "预览 · 5:4 自动填充（可拖动微调）", font=f_small, fill=DIM)

    # right controls
    rx = 560
    rw = 520

    d.rounded_rectangle([rx, 170, rx + rw, 262], radius=13, fill=(8, 15, 25),
                        outline=(44, 72, 105), width=2)
    d.text((rx + 22, 188), "拖入照片，或点击选择", font=f_label, fill=(220, 233, 248))
    d.text((rx + 22, 214), "支持 JPG / PNG / WebP；原图不上传，只在浏览器里处理", font=f_small, fill=DIM)
    d.rounded_rectangle([rx + 22, 236, rx + 102, 254], radius=6, fill=(16, 42, 48),
                        outline=(52, 120, 112), width=1)
    d.text((rx + 31, 239), "选择照片", font=f_small, fill=ACCENT)

    # size comparison
    d.text((rx, 290), "原图", font=f_small, fill=DIM)
    d.text((rx, 310), "4.2 MB", font=ImageFont.truetype(DIN, 22), fill=(200, 212, 228))
    d.line([rx + 72, 322, rx + 120, 322], fill=(120, 136, 158), width=2)
    d.polygon([(rx + 116, 317), (rx + 126, 322), (rx + 116, 327)], fill=(120, 136, 158))
    d.text((rx + 150, 290), "上传到设备", font=f_small, fill=DIM)
    d.text((rx + 150, 310), "8.6 KB", font=ImageFont.truetype(DIN, 22), fill=ACCENT)
    d.text((rx + 262, 314), "JPEG 82% · 160×128", font=f_small, fill=DIM)

    # quality slider
    d.text((rx, 364), "压缩质量", font=f_label, fill=(210, 224, 240))
    d.text((rx + 470, 364), "82%", font=f_label, fill=ACCENT)
    d.rounded_rectangle([rx, 392, rx + 470, 398], radius=3, fill=(27, 43, 63))
    d.rounded_rectangle([rx, 392, rx + 470 * 0.82, 398], radius=3, fill=ACCENT)
    d.ellipse([rx + 470 * 0.82 - 8, 387, rx + 470 * 0.82 + 8, 403],
              fill=(240, 250, 255), outline=ACCENT, width=2)
    d.text((rx, 412), "质量越高越清晰，文件也越大；建议 80–85%。", font=f_small, fill=DIM)

    # upload button
    d.rounded_rectangle([rx, 458, rx + 250, 502], radius=12, fill=(30, 105, 96),
                        outline=(94, 234, 212), width=1)
    d.text((rx + 34, 470), "上传到设备", font=f_btn, fill=(240, 255, 252))
    d.rounded_rectangle([rx + 266, 458, rx + 410, 502], radius=12, fill=(14, 24, 38),
                        outline=(44, 70, 100), width=1)
    d.text((rx + 294, 470), "取消", font=f_btn, fill=(170, 188, 210))

    # gallery
    gy = 552
    d.line([88, gy - 18, cw - 88, gy - 18], fill=(28, 44, 64), width=1)
    d.text((88, gy), "设备相册", font=f_label, fill=(210, 224, 240))
    d.text((88 + 74, gy + 2), "3 / 8", font=f_small, fill=DIM)

    tw, th = 92, 74
    for i in range(3):
        tx = 88 + i * (tw + 14)
        crop = photo.crop((i * 60, 0, i * 60 + 540, 432)).resize((tw, th), Image.LANCZOS)
        mask = Image.new("L", (tw, th), 0)
        ImageDraw.Draw(mask).rounded_rectangle([0, 0, tw, th], radius=8, fill=255)
        im.paste(crop, (tx, gy + 26), mask)
        d.rounded_rectangle([tx, gy + 26, tx + tw, gy + 26 + th], radius=8,
                            outline=(52, 76, 104), width=1)
        d.rounded_rectangle([tx + tw - 22, gy + 30, tx + tw - 6, gy + 46], radius=4,
                            fill=(4, 8, 14, 190))
        d.text((tx + tw - 17, gy + 33), "×", font=f_small, fill=(220, 230, 244))

    # storage
    d.text((cw - 390, gy), "SPIFFS 已用", font=f_small, fill=DIM)
    d.rounded_rectangle([cw - 300, gy + 2, cw - 90, gy + 8], radius=3, fill=(27, 43, 63))
    d.rounded_rectangle([cw - 300, gy + 2, cw - 286, gy + 8], radius=3, fill=ACCENT)
    d.text((cw - 390, gy + 22), "26 KB / 896 KB（3 张）", font=f_small, fill=MUTED)

    return im


def main() -> None:
    DOCS.mkdir(exist_ok=True)
    photo = make_photo()
    photo.save(DOCS / "photo-sample.jpg", quality=92)

    overlay = device_screen(photo, True)
    clean = device_screen(photo, False)

    p_overlay = panel(overlay, "方案 A · 照片 + 底部时间叠加（推荐）")
    p_clean = panel(clean, "方案 B · 纯照片（只保留页码点）")
    p_overlay.save(DOCS / "photo-device-overlay.png")
    p_clean.save(DOCS / "photo-device-clean.png")

    web = web_card(photo)
    web.save(DOCS / "photo-web-upload.png")

    gap = 26
    sheet = Image.new("RGB", (p_overlay.width + p_clean.width + web.width + gap * 2,
                              max(p_overlay.height, p_clean.height, web.height)),
                      (6, 10, 17))
    sheet.paste(p_overlay, (0, 0))
    sheet.paste(p_clean, (p_overlay.width + gap, 0))
    sheet.paste(web, (p_overlay.width + p_clean.width + gap * 2, 0))
    sheet.save(DOCS / "photo-ui-preview.png")

    print("saved photo previews")


if __name__ == "__main__":
    main()

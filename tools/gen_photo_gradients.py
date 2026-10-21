#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""生成白色 + alpha 底部渐变图；运行时用 recolor 染成任意颜色。"""
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
OUT = ROOT / "src"
W = 160

for h in (48, 56, 72):
    data = bytearray()
    for y in range(h):
        t = y / max(1, h - 1)
        a = int(255 * (t ** 1.15))
        for _ in range(W):
            data += bytes((0xFF, 0xFF, a))  # RGB565 white + alpha
    name = f"photo_grad_{h}"
    lines = [
        "// 自动生成：白色 RGB565 + alpha 底部渐变，运行时 recolor 染色。",
        '#include "lvgl.h"',
        "",
        f"static const uint8_t {name}_map[] = {{",
    ]
    for i in range(0, len(data), 18):
        chunk = ", ".join(f"0x{b:02X}" for b in data[i:i + 18])
        lines.append("    " + chunk + ",")
    lines += [
        "};",
        "",
        f"const lv_img_dsc_t {name} = {{",
        "    .header.always_zero = 0,",
        f"    .header.w = {W},",
        f"    .header.h = {h},",
        "    .header.cf = LV_IMG_CF_TRUE_COLOR_ALPHA,",
        f"    .data_size = sizeof({name}_map),",
        f"    .data = {name}_map,",
        "};",
        "",
    ]
    (OUT / f"{name}.c").write_text("\n".join(lines), encoding="utf-8")
    print(name, len(data))

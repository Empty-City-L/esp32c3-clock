#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""接收 uidump 固件从串口吐出的整屏 RGB565 帧，保存为 PNG。

用法：
    pio run -e uidump -t upload
    python tools/dump_ui.py COM3

不传串口时会自动选择第一个串口。依赖：pyserial、numpy、Pillow。
"""
from __future__ import annotations

import sys
import time
from pathlib import Path

import numpy as np
import serial
from PIL import Image
from serial.tools import list_ports

ROOT = Path(__file__).resolve().parent.parent
BAUD = 921600
MAGIC = b"UIDUMP1\x00"


def pick_port() -> str:
    if len(sys.argv) > 1:
        return sys.argv[1]
    ports = list(list_ports.comports())
    if not ports:
        raise SystemExit("没有找到串口，请手动传入，例如：python tools/dump_ui.py COM3")
    return ports[0].device


def main() -> None:
    port = pick_port()
    print(f"port: {port} @ {BAUD}")

    s = serial.Serial(port, BAUD, timeout=0.1)
    # 复位到 uidump 固件（DTR/RTS 组合与 esptool 的自动复位一致）
    s.setDTR(False)
    s.setRTS(True)
    time.sleep(0.15)
    s.setRTS(False)

    buf = b""
    deadline = time.time() + 20
    while time.time() < deadline:
        buf += s.read(8192)
        if MAGIC in buf:
            break
    else:
        s.close()
        raise SystemExit("没有收到 UIDUMP 头；确认烧录的是 pio run -e uidump -t upload 的固件")

    print("magic found, receiving frame...")
    idx = buf.index(MAGIC) + len(MAGIC)
    while len(buf) - idx < 4:
        buf += s.read(8192)

    w = int.from_bytes(buf[idx:idx + 2], "little")
    h = int.from_bytes(buf[idx + 2:idx + 4], "little")
    idx += 4
    need = w * h * 2
    while len(buf) - idx < need:
        buf += s.read(65536)
    raw = buf[idx:idx + need]
    s.close()

    # LVGL RGB565 little-endian: bits 15..11 red, 10..5 green, 4..0 blue
    v = np.frombuffer(raw, dtype="<u2").reshape(h, w)
    r = ((v >> 11) & 0x1F) * 255 // 31
    g = ((v >> 5) & 0x3F) * 255 // 63
    b = (v & 0x1F) * 255 // 31
    img = Image.fromarray(np.dstack([r, g, b]).astype(np.uint8), "RGB")

    ROOT.joinpath("docs").mkdir(exist_ok=True)
    out = ROOT / "docs" / "ui-dump.png"
    img.save(out)
    img.resize((w * 3, h * 3), Image.LANCZOS).save(ROOT / "docs" / "ui-dump@3x.png")
    print(f"saved {out} ({w}x{h})")


if __name__ == "__main__":
    main()

#!/usr/bin/env python3
"""Builds the 256x256 NRO icon (JPEG) from Kirikiroid2's original launcher icon (APK res/drawable-xxhdpi).
usage: python3 make_icon.py <ic_launcher.png> <out.jpg>"""
import sys
from PIL import Image

src = Image.open(sys.argv[1]).convert("RGBA")
icon = src.resize((256, 256), Image.LANCZOS)
bg = Image.new("RGB", icon.size, (255, 255, 255))  # the original icon is drawn on white
bg.paste(icon, mask=icon.split()[3])
bg.save(sys.argv[2], "JPEG", quality=95)
print("icon:", sys.argv[2], bg.size)

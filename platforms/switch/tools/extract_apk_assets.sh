#!/usr/bin/env bash
# The compiled cocos UI (.csb), images, locales and DroidSansFallback.ttf of 1.3.9 exist only inside the
# official APK (the public repo has the .csd sources only). Extract them from the author's GitHub release
# into the RomFS staging dir at build time — they are NOT committed to this repo.
#   bash tools/extract_apk_assets.sh  -> build/romfs/
set -euo pipefail
APK=${K2NX_WS:-/mnt/d/Kirikiroid2-Switch}/.cache/apk/Kirikiroid2_1.3.9.apk
URL=https://github.com/zeas2/Kirikiroid2/releases/download/1.3.9/Kirikiroid2_1.3.9.apk
[ -f "$APK" ] || { mkdir -p "$(dirname "$APK")"; curl -fsSL -o "$APK" "$URL"; }
OUT=${K2NX_WS:-/mnt/d/Kirikiroid2-Switch}/build/romfs
rm -rf "$OUT" && mkdir -p "$OUT"
T=$(mktemp -d)
unzip -q "$APK" 'assets/*' -d "$T"
cp -r "$T"/assets/. "$OUT"/
rm -rf "$T"
echo "romfs: $(find "$OUT" -type f | wc -l) files, $(du -sh "$OUT" | cut -f1)"
# NRO icon: Kirikiroid2's original launcher icon, scaled to 256x256 JPEG
T=$(mktemp -d)
unzip -q -j "$APK" 'res/drawable-xxhdpi-v4/ic_launcher.png' -d "$T"
python3 "${K2NX_WS:-/mnt/d/Kirikiroid2-Switch}/src/platforms/switch/make_icon.py" "$T/ic_launcher.png" \
  "${K2NX_WS:-/mnt/d/Kirikiroid2-Switch}/build/icon.jpg"
rm -rf "$T"

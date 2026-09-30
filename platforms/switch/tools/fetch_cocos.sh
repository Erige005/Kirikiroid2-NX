#!/usr/bin/env bash
# Fetches cocos2d-x 3.11.1 (the version inside Kirikiroid2 APK 1.3.9) + its v3-deps-94 externals into
# src/vendor/cocos2d-x/current-3.11.1 and applies the Nintendo Switch patches:
#   patches/0001-switch-platform.patch  : Switch platform (GLES2, libnx touch/swkbd/shared font, paths) and the
#                                          changes the author's cocos build had (Widget roots, MouseButton, ...)
#   patches/0002-external-tinyxml2-virtual-print.patch : tinyxml2 XMLPrinter::Print virtual
set -euo pipefail
WS=${K2NX_WS:-/mnt/d/Kirikiroid2-Switch}
V=$WS/src/vendor/cocos2d-x/current-3.11.1
P=$WS/src/platforms/switch/cocos2dx/patches
C=$WS/.cache
mkdir -p "$C" "$(dirname "$V")"
if [ ! -d "$V/.git" ]; then
  git clone -q --depth 1 --branch cocos2d-x-3.11.1 https://github.com/cocos2d/cocos2d-x.git "$V"
fi
[ -f "$C/v3-deps-94.zip" ] || curl -fsSL -o "$C/v3-deps-94.zip" \
  https://github.com/cocos2d/cocos2d-x-3rd-party-libs-bin/archive/v3-deps-94.zip
T=$(mktemp -d)
unzip -q "$C/v3-deps-94.zip" -d "$T"
cp -r "$T"/cocos2d-x-3rd-party-libs-bin-3-deps-94/. "$V/external/"
rm -rf "$T"
cd "$V"
git apply --whitespace=nowarn "$P/0001-switch-platform.patch"
git apply --whitespace=nowarn "$P/0002-external-tinyxml2-virtual-print.patch"
echo "cocos2d-x 3.11.1 ready (patched) at $V"

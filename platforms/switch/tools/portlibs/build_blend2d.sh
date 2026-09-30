#!/usr/bin/env bash
# blend2d (2D vector graphics) for LayerExDraw (krkr2's GDI+-free implementation). Static, no JIT.
set -euo pipefail
source "$(dirname "$0")/env.sh"
S=$K2S_SRC_CACHE/blend2d
# krkr2's LayerExDraw uses the camelCase API: pin the commit its vcpkg baseline uses (2025-03-08)
REF=d2027ebfd6aaf53b190b6b3b497425fc85f14251
[ -d "$S/.git" ] || git clone https://github.com/blend2d/blend2d.git "$S"
git -C "$S" checkout -f -q $REF
# [K2S] newlib: 64-bit off_t by default, no *64 file API
grep -q "__SWITCH__" "$S/src/blend2d/api-build_p.h" || sed -i "s/#if defined(__APPLE__    )/#if defined(__SWITCH__) || defined(__APPLE__    )/" "$S/src/blend2d/api-build_p.h"
B=$HOME/k2s-build/portlibs/blend2d
XF="$CXXFLAGS"; unset LDFLAGS CFLAGS CXXFLAGS CPPFLAGS LIBS
rm -rf "$B"
$PORTLIBS/bin/aarch64-none-elf-cmake -S "$S" -B "$B" -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
  -DBLEND2D_STATIC=ON -DBLEND2D_NO_JIT=ON -DBLEND2D_NO_JIT_LOGGING=ON -DBLEND2D_TEST=OFF \
  -DBLEND2D_NO_TLS=ON -DBLEND2D_NO_FUTEX=ON \
  -DCMAKE_CXX_FLAGS="$XF -D__STDC_FORMAT_MACROS -I$(cd "$(dirname "$0")" && pwd)/shims" \
  -DCMAKE_INSTALL_PREFIX="$K2S_PORTLIBS"
ninja -j4 -C "$B"
ninja -C "$B" install
ls -la "$K2S_PORTLIBS/lib/libblend2d.a"

#!/usr/bin/env bash
# jxrlib (Microsoft JPEG XR reference, BSD) — libjpegxr + libjxrglue, static. LoadJXR.cpp includes "JXRGlue.h".
set -euo pipefail
source "$(dirname "$0")/env.sh"
fetch https://github.com/4creators/jxrlib/archive/refs/heads/master.tar.gz jxrlib-master.tar.gz
W=$HOME/k2s-build/portlibs/jxrlib
rm -rf "$W" && mkdir -p "$W" && tar -xzf "$K2S_SRC_CACHE/jxrlib-master.tar.gz" -C "$W" --strip-components=1
cd "$W"
INC="-I. -Icommon/include -Iimage/sys -Ijxrgluelib"
# MSVC byte-swap intrinsics -> GCC builtins (upstream relied on implicit declarations)
DEF="-D__ANSI__ -DDISABLE_PERF_MEASUREMENT -D_byteswap_ulong=__builtin_bswap32 -D_byteswap_ushort=__builtin_bswap16"
FL="$ARCH_FLAGS -O2 -D__SWITCH__ -std=gnu17 -w -Wno-error=implicit-function-declaration -include wchar.h $INC $DEF"
mkdir -p obj
SRCS="$(ls image/sys/*.c image/decode/*.c image/encode/*.c jxrgluelib/*.c)"
for s in $SRCS; do
  o=obj/$(echo "$s" | tr '/' '_' | sed 's/\.c$/.o/')
  $CC $FL -c "$s" -o "$o"
done
rm -f "$K2S_PORTLIBS/lib/libjxr.a"
$AR rcs "$K2S_PORTLIBS/lib/libjxr.a" obj/*.o
mkdir -p "$K2S_PORTLIBS/include/jxrlib"
cp common/include/*.h image/sys/*.h jxrgluelib/*.h "$K2S_PORTLIBS/include/jxrlib/"
cp image/x86/*.h "$K2S_PORTLIBS/include/jxrlib/" 2>/dev/null || true
ls -la "$K2S_PORTLIBS/lib/libjxr.a"

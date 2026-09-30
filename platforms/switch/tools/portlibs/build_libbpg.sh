#!/usr/bin/env bash
# libbpg 0.9.8 (Fabrice Bellard) — decoder library only (LoadBPG.cpp includes "libbpg/libbpg.h").
set -euo pipefail
source "$(dirname "$0")/env.sh"
V=0.9.8
fetch https://bellard.org/bpg/libbpg-$V.tar.gz libbpg-$V.tar.gz
W=$HOME/k2s-build/portlibs/libbpg-$V
rm -rf "$W" && mkdir -p "$W" && tar -xzf "$K2S_SRC_CACHE/libbpg-$V.tar.gz" -C "$W" --strip-components=1
cd "$W"
# Kirikiroid2's libbpg had bpg_decoder_get_gray_line() (see patches/libbpg_gray_line.c)
cat "$(dirname "$0")/patches/libbpg_gray_line.c" >> libbpg.c
sed -i 's/^int bpg_decoder_get_line(BPGDecoderContext \*s, void \*buf);/&\n\/* [K2S] Kirikiroid2 extension: one 8-bit gray line (glmGrayscale) *\/\nint bpg_decoder_get_gray_line(BPGDecoderContext *s, void *buf);/' libbpg.h
grep -q bpg_decoder_get_gray_line libbpg.h || { echo "header patch failed"; exit 1; }
# libbpg.a = libbpg.c + a trimmed HEVC decoder from libavcodec; build those objects directly.
# Own flags (not $CFLAGS): libbpg bundles its own libavutil headers, which must win over the installed FFmpeg.
make -k libbpg.a CROSS_PREFIX=aarch64-none-elf- CC="$CC" AR="$AR" \
  CFLAGS="$ARCH_FLAGS -O2 -D__SWITCH__ -Wno-incompatible-pointer-types -Wno-int-conversion -Wno-implicit-function-declaration -I. -D_ISOC99_SOURCE -D_POSIX_C_SOURCE=200112 -D_XOPEN_SOURCE=600 -DHAVE_AV_CONFIG_H -std=gnu17 -fno-asynchronous-unwind-tables -fdata-sections -ffunction-sections -fno-math-errno -fno-signed-zeros -fno-tree-vectorize -fomit-frame-pointer -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_REENTRANT -DCONFIG_BPG_VERSION=\\\"$V\\\"" \
  USE_X265= USE_JCTVC= USE_EMCC= USE_BPGVIEW= > build.log 2>&1 || { grep -E "error" build.log | head -20; exit 1; }
cp libbpg.a "$K2S_PORTLIBS/lib/"
mkdir -p "$K2S_PORTLIBS/include/libbpg"
cp libbpg.h "$K2S_PORTLIBS/include/libbpg/"
ls -la "$K2S_PORTLIBS/lib/libbpg.a"

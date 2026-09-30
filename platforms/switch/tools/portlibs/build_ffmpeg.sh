#!/usr/bin/env bash
# FFmpeg 3.2.x (APK 1.3.9 ships ijkplayer ff3.2): 1.3.9's movie/ffmpeg code uses the pre-5.0 API (AVStream->codec, avcodec_decode_video2, ...),
# devkitPro ships 7.1. Libs 1.3.9 uses: avcodec avformat avutil avfilter swscale swresample.
set -euo pipefail
source "$(dirname "$0")/env.sh"
V=3.2.19
fetch https://ffmpeg.org/releases/ffmpeg-$V.tar.xz ffmpeg-$V.tar.xz
W=$HOME/k2s-build/portlibs/ffmpeg-$V
rm -rf "$W" && tar -xJf "$K2S_SRC_CACHE/ffmpeg-$V.tar.xz" -C "$(dirname "$W")"
cd "$W"
# 3.2's aarch64 FFT/IMDCT asm keeps jump tables of absolute addresses in .rodata ->
# "read-only segment has dynamic relocations" in a PIE NRO. Put them in .data.rel.ro (relocate=1),
# as later FFmpeg versions do.
for f in libavcodec/aarch64/fft_neon.S libavcodec/aarch64/imdct15_neon.S; do
  sed -i -E 's/^(const[[:space:]]+[A-Za-z0-9_]+)([[:space:]]*,[[:space:]]*align=[0-9]+)?[[:space:]]*$/\1\2, relocate=1/' "$f"
  grep -nE "^const" "$f"
done
# remove any previously installed FFmpeg (e.g. the 4.4 build) so headers don't mix
rm -rf "$K2S_PORTLIBS"/include/libav* "$K2S_PORTLIBS"/include/libsw* \
       "$K2S_PORTLIBS"/lib/libav* "$K2S_PORTLIBS"/lib/libsw* "$K2S_PORTLIBS"/lib/pkgconfig/libav* "$K2S_PORTLIBS"/lib/pkgconfig/libsw*
./configure --prefix="$K2S_PORTLIBS" \
  --enable-cross-compile --cross-prefix=aarch64-none-elf- --arch=aarch64 --cpu=cortex-a57 --target-os=none \
  --pkg-config=pkg-config --extra-libs="-lnx" \
  --extra-cflags="-Wno-incompatible-pointer-types -Wno-int-conversion -Wno-implicit-function-declaration" \
  --enable-static --disable-shared --enable-pic --enable-pthreads \
  --disable-programs --disable-doc --disable-debug --disable-network --disable-devices --disable-avdevice \
  --disable-postproc --disable-encoders --disable-muxers --disable-hwaccels --disable-iconv \
  --disable-runtime-cpudetect --enable-neon \
  > "$W/configure.log" 2>&1 || { tail -30 "$W/configure.log"; tail -40 ffbuild/config.log; exit 1; }
# target-os=none leaves section_data_rel_ro off; the relocate=1 tables above need it (PIE NRO)
sed -i 's/^#define HAVE_SECTION_DATA_REL_RO 0$/#define HAVE_SECTION_DATA_REL_RO 1/' config.h
grep -q "HAVE_SECTION_DATA_REL_RO 1" config.h || { echo "config.h patch failed"; exit 1; }
make -j4 > "$W/make.log" 2>&1 || { grep -E "error" "$W/make.log" | head -30; exit 1; }
make install > /dev/null
ls -la "$K2S_PORTLIBS"/lib/libav*.a "$K2S_PORTLIBS"/lib/libsw*.a

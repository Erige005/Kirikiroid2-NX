#!/usr/bin/env bash
# OpenCV 3.4.20, core + imgproc only. 1.3.9 uses cv::Mat, resize, warpAffine/Perspective,
# get{Affine,Perspective}Transform, boxFilter (RenderManager*.cpp, tvpgl_arm.cpp).
# Author used an OpenCV 3.0/3.1-era build (separate libopencv_hal); API is identical for these calls.
set -euo pipefail
source "$(dirname "$0")/env.sh"
V=3.4.20
fetch https://github.com/opencv/opencv/archive/refs/tags/$V.tar.gz opencv-$V.tar.gz
S=$HOME/k2s-build/portlibs/opencv-$V
B=$S/build-switch
[ -d "$S" ] || tar -xzf "$K2S_SRC_CACHE/opencv-$V.tar.gz" -C "$(dirname "$S")"
# Switch has no /proc/cpuinfo or getauxval: treat it like Linux/Android aarch64 (NEON + FP16 always
# present on Cortex-A57); otherwise cv::HWFeatures throws "Missing support for required CPU baseline
# features" during static initialization.
SYS="$S/modules/core/src/system.cpp"
if ! grep -q "K2S: Switch CPU features" "$SYS"; then
  sed -i -E 's/^    #if defined __ANDROID__ \|\| defined __linux__ \|\| defined __FreeBSD__ \|\| defined __QNX__( \|\| defined __SWITCH__)?$/    #if defined __SWITCH__ \/\/ K2S: Switch CPU features\n        have[CV_CPU_NEON] = true;\n        have[CV_CPU_FP16] = true;\n    #elif defined __ANDROID__ || defined __linux__ || defined __FreeBSD__ || defined __QNX__/' "$SYS"
fi
grep -q "K2S: Switch CPU features" "$SYS" || { echo "system.cpp patch failed"; exit 1; }
rm -rf "$B" && mkdir -p "$B"
unset CFLAGS CXXFLAGS LDFLAGS CPPFLAGS LIBS   # let the devkitPro toolchain file set flags
$PORTLIBS/bin/aarch64-none-elf-cmake -S "$S" -B "$B" -G Ninja -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
  -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$K2S_PORTLIBS" \
  -DBUILD_LIST=core,imgproc -DBUILD_SHARED_LIBS=OFF -DBUILD_opencv_apps=OFF \
  -DBUILD_TESTS=OFF -DBUILD_PERF_TESTS=OFF -DBUILD_EXAMPLES=OFF -DBUILD_DOCS=OFF -DBUILD_JAVA=OFF \
  -DBUILD_opencv_python2=OFF -DBUILD_opencv_python3=OFF -DBUILD_ZLIB=OFF -DBUILD_ITT=OFF -DWITH_ITT=OFF \
  -DWITH_TBB=OFF -DWITH_OPENMP=OFF -DWITH_PTHREADS_PF=ON -DWITH_IPP=OFF -DWITH_OPENCL=OFF -DWITH_LAPACK=OFF \
  -DWITH_EIGEN=OFF -DWITH_PROTOBUF=OFF -DWITH_ADE=OFF -DWITH_QUIRC=OFF -DWITH_1394=OFF -DWITH_GTK=OFF \
  -DWITH_FFMPEG=OFF -DWITH_GSTREAMER=OFF -DWITH_V4L=OFF -DWITH_JPEG=OFF -DWITH_PNG=OFF -DWITH_TIFF=OFF \
  -DWITH_WEBP=OFF -DWITH_JASPER=OFF -DWITH_OPENEXR=OFF -DWITH_IMGCODEC_HDR=OFF -DWITH_IMGCODEC_SUNRASTER=OFF \
  -DWITH_IMGCODEC_PXM=OFF -DCV_ENABLE_INTRINSICS=ON -DENABLE_NEON=ON -DOPENCV_ENABLE_ALLOCATOR_STATS=OFF \
  -DCMAKE_CXX_FLAGS="-D__SWITCH__ -fpermissive" \
  > "$B/configure.log" 2>&1 || { tail -40 "$B/configure.log"; exit 1; }
ninja -j4 -C "$B" > "$B/make.log" 2>&1 || { grep -E "error" "$B/make.log" | sort -u | head -30; exit 1; }
ninja -C "$B" install > /dev/null
ls -la "$K2S_PORTLIBS"/lib/libopencv_*.a

#!/usr/bin/env bash
# UnRAR 5.6.5 (RARLab freeware source, 2018-07 — same time 1.3.9 "import unrar to support unpack rar with password").
# Built as a static RARDLL library; headers installed as <unrar/*.hpp> (UtilStreams.cpp includes "unrar/dll.hpp").
set -euo pipefail
source "$(dirname "$0")/env.sh"
V=5.6.5
fetch https://www.rarlab.com/rar/unrarsrc-$V.tar.gz unrarsrc-$V.tar.gz
W=$HOME/k2s-build/portlibs/unrar-$V
rm -rf "$W" && mkdir -p "$W" && tar -xzf "$K2S_SRC_CACHE/unrarsrc-$V.tar.gz" -C "$W" --strip-components=1
cd "$W"
# newlib has no utimensat/UTIME_NOW: fall back to utime()
sed -i 's/^#if _POSIX_C_SOURCE >= 200809L$/#if _POSIX_C_SOURCE >= 200809L \&\& !defined(__SWITCH__)/' os.hpp
# Use the upstream makefile's `lib` target to compile exactly its object list (LIB_OBJ + OBJECTS);
# the final shared-library link step fails (no .so on Switch) and is ignored: we archive the objects.
make -k lib CXX="$CXX" STRIP=true \
  CXXFLAGS="$CXXFLAGS -std=gnu++11 -Wno-dangling-else -Wno-deprecated-declarations" \
  DEFINES="-D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -DRAR_SMP" LDFLAGS="" > build.log 2>&1 || true
grep -E "error:" build.log | head -20 || true
rm -f "$K2S_PORTLIBS/lib/libunrar.a"
$AR rcs "$K2S_PORTLIBS/lib/libunrar.a" *.o
echo "objects: $(ls *.o | wc -l)"
mkdir -p "$K2S_PORTLIBS/include/unrar"
cp *.hpp "$K2S_PORTLIBS/include/unrar/"
ls -la "$K2S_PORTLIBS/lib/libunrar.a"

#!/usr/bin/env bash
set -uo pipefail
source /etc/profile.d/devkit-env.sh
source "$DEVKITPRO/switch2vars.sh"

TGT=${1:-all}; CFG=${2:-Release}
SRC=${K2NX_WS:-/mnt/d/Kirikiroid2-Switch}/src/platforms/switch
B=$HOME/k2s-nx2-build/k2-$CFG
mkdir -p "$B"

aarch64-none-elf-cmake -S "$SRC" -B "$B" -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE="$DEVKITPRO/libounce/cmake/Switch2.cmake" \
  -DCMAKE_BUILD_TYPE="$CFG" -DCMAKE_POLICY_VERSION_MINIMUM=3.5 > "$B/configure.log" 2>&1 \
  || { tail -30 "$B/configure.log"; exit 1; }

ninja -j4 -C "$B" -k 0 "$TGT" > "$B/build.log" 2>&1
RC=$?
bash ${K2NX_WS:-/mnt/d/Kirikiroid2-Switch}/src/platforms/switch/tools/errs.sh "$B/build.log" 60
echo "failed objects: $(grep -c '^FAILED' "$B/build.log")"
exit $RC

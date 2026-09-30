#!/usr/bin/env bash
# Builds Kirikiroid2 for Switch:  bash ${K2NX_WS:-/mnt/d/Kirikiroid2-Switch}/src/platforms/switch/tools/build_k2.sh [target] [Debug|Release]
# target: krkr2core | kr2plugin | kirikiroid2 (default: all). Log: ~/k2s-build/k2-<cfg>/build.log
set -uo pipefail
source /etc/profile.d/devkit-env.sh
TGT=${1:-all}; CFG=${2:-Debug}
SRC=${K2NX_WS:-/mnt/d/Kirikiroid2-Switch}/src/platforms/switch
B=$HOME/k2s-build/k2-$CFG
mkdir -p "$B"
/opt/devkitpro/portlibs/switch/bin/aarch64-none-elf-cmake -S "$SRC" -B "$B" -G Ninja \
  -DCMAKE_BUILD_TYPE="$CFG" -DCMAKE_POLICY_VERSION_MINIMUM=3.5 > "$B/configure.log" 2>&1 \
  || { tail -30 "$B/configure.log"; exit 1; }
ninja -j4 -C "$B" -k 0 "$TGT" > "$B/build.log" 2>&1
RC=$?
bash ${K2NX_WS:-/mnt/d/Kirikiroid2-Switch}/src/platforms/switch/tools/errs.sh "$B/build.log" 60
echo "failed objects: $(grep -c '^FAILED' "$B/build.log")"
exit $RC

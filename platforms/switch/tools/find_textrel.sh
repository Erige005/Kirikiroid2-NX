#!/usr/bin/env bash
# Finds static-lib members with absolute relocations (R_AARCH64_ABS64/ABS32) inside read-only
# sections (.text*/.rodata*) — those become "read-only segment has dynamic relocations" in a PIE NRO.
source /etc/profile.d/devkit-env.sh
B=$HOME/k2s-build/k2-Debug
LIBS="$B/libkrkr2core.a $B/libkr2plugin.a $B/cocos2dx/libcocos2d.a $HOME/k2s-portlibs/lib/*.a \
      $HOME/k2s-portlibs/share/OpenCV/3rdparty/lib/*.a"
for L in $LIBS; do
  aarch64-none-elf-readelf -rW "$L" 2>/dev/null | awk -v lib="$(basename $L)" '
    /^File: / {obj=$2}
    /^Relocation section/ {sec=$3; gsub(/\x27/,"",sec); ro=(sec ~ /^\.rela\.(text|rodata)/)}
    ro && ($3 ~ /R_AARCH64_ABS(64|32)/) {n[obj" "sec]++}
    END {for (k in n) print n[k], lib, k}'
done | sort -rn | head -30

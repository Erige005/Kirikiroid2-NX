#!/usr/bin/env bash
# 7-Zip sources for src/src/core/base/7zip (missing from the public zeas2 tree; 1.3.9 includes
# "7zip/C/7z.h", "7zip/CPP/7zip/Archive/7z/7zOut.h", ...).
# The C++ half of the plain LZMA SDK is Windows-only, so the POSIX port p7zip 16.02 (same 16.x
# generation as the 2017 code) is used: its C/ and CPP/ (incl. myWindows + include_windows) layout
# matches the include paths above. Run in WSL.
set -euo pipefail
C=${K2NX_WS:-/mnt/d/Kirikiroid2-Switch}/.cache/src
mkdir -p "$C"
[ -f "$C/p7zip_16.02_src_all.tar.bz2" ] || curl -fsSL -o "$C/p7zip_16.02_src_all.tar.bz2" \
  https://downloads.sourceforge.net/project/p7zip/p7zip/16.02/p7zip_16.02_src_all.tar.bz2
D=${K2NX_WS:-/mnt/d/Kirikiroid2-Switch}/src/src/core/base/7zip
T=$(mktemp -d)
tar -xjf "$C/p7zip_16.02_src_all.tar.bz2" -C "$T"
rm -rf "$D" && mkdir -p "$D"
cp -r "$T"/p7zip_16.02/C "$T"/p7zip_16.02/CPP "$D"/
cp "$T"/p7zip_16.02/DOC/License.txt "$D"/License.txt 2>/dev/null || true
rm -rf "$T"
# p7zip's C/ lacks the C 7z decoder (7z.h, 7zArcIn.c, 7zDec.c, 7zFile.c ...) that 7zArchive.cpp uses:
# overlay the C/ folder of LZMA SDK 16.04.
command -v 7z >/dev/null || { echo "need 7zip (apt install 7zip)"; exit 2; }
[ -f "$C/lzma1604.7z" ] || curl -fsSL -o "$C/lzma1604.7z" https://www.7-zip.org/a/lzma1604.7z
7z x -aos -o"$D" "$C/lzma1604.7z" C > /dev/null   # -aos: only add files p7zip lacks
ls "$D" "$D/CPP"

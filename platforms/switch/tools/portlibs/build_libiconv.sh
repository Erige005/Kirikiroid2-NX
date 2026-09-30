#!/usr/bin/env bash
# GNU libiconv 1.17 (1.3.9 used vendor/libiconv; newlib has no iconv).
set -euo pipefail
source "$(dirname "$0")/env.sh"
V=1.17
fetch https://ftp.gnu.org/pub/gnu/libiconv/libiconv-$V.tar.gz libiconv-$V.tar.gz
W=$HOME/k2s-build/portlibs/libiconv-$V
rm -rf "$W" && mkdir -p "$W" && tar -xzf "$K2S_SRC_CACHE/libiconv-$V.tar.gz" -C "$(dirname "$W")"
cd "$W"
./configure --host=$HOST --prefix="$K2S_PORTLIBS" --disable-shared --enable-static --disable-nls >/dev/null
make -j4 -C libcharset >/dev/null
cp libcharset/include/localcharset.h lib/   # top-level make normally does this
make -j4 -C lib >/dev/null
make -C lib install >/dev/null
mkdir -p "$K2S_PORTLIBS/include" && cp include/iconv.h "$K2S_PORTLIBS/include/"
ls -la "$K2S_PORTLIBS/lib/libiconv.a"

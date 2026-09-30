# Common environment for cross-compiling third-party libs (the missing `vendor/` of 1.3.9) for Switch.
# Libraries are installed to $K2S_PORTLIBS (default ~/k2s-portlibs), separate from devkitPro's portlibs.
source /etc/profile.d/devkit-env.sh
export K2S_PORTLIBS=${K2S_PORTLIBS:-$HOME/k2s-portlibs}
export K2S_SRC_CACHE=${K2NX_WS:-/mnt/d/Kirikiroid2-Switch}/.cache/src
mkdir -p "$K2S_PORTLIBS/include" "$K2S_PORTLIBS/lib" "$K2S_SRC_CACHE" "$HOME/k2s-build/portlibs"

export PORTLIBS=$DEVKITPRO/portlibs/switch
export CC=aarch64-none-elf-gcc CXX=aarch64-none-elf-g++ AR=aarch64-none-elf-gcc-ar RANLIB=aarch64-none-elf-gcc-ranlib
export ARCH_FLAGS="-march=armv8-a+crc+crypto -mtune=cortex-a57 -mtp=soft -fPIC -ftls-model=local-exec"
BASE_FLAGS="$ARCH_FLAGS -O2 -ffunction-sections -fdata-sections -D__SWITCH__ -I$K2S_PORTLIBS/include -I$PORTLIBS/include -I$DEVKITPRO/libnx/include"
# GCC 15 defaults to C23 (where `f()` means no args): old C libs need gnu17
export CFLAGS="$BASE_FLAGS -std=gnu17"
export CXXFLAGS="$BASE_FLAGS"
export CPPFLAGS="-D__SWITCH__ -I$K2S_PORTLIBS/include -I$PORTLIBS/include -I$DEVKITPRO/libnx/include"
export LDFLAGS="-specs=$DEVKITPRO/libnx/switch.specs $ARCH_FLAGS -L$K2S_PORTLIBS/lib -L$PORTLIBS/lib -L$DEVKITPRO/libnx/lib"
export LIBS="-lnx"
export PKG_CONFIG_LIBDIR="$K2S_PORTLIBS/lib/pkgconfig:$PORTLIBS/lib/pkgconfig"
HOST=aarch64-none-elf

# fetch <url> <file>: download once into the source cache on D:
fetch() { [ -f "$K2S_SRC_CACHE/$2" ] || curl -fsSL -o "$K2S_SRC_CACHE/$2" "$1"; }

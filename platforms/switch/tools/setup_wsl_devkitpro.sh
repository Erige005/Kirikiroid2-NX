#!/usr/bin/env bash
# Sets up the build environment inside WSL2 Ubuntu (run as root):
#   wsl -d Ubuntu -u root -- bash ${K2NX_WS:-/mnt/d/Kirikiroid2-Switch}/src/platforms/switch/tools/setup_wsl_devkitpro.sh
# Log: /root/dkp_install.log
set -euo pipefail
export DEBIAN_FRONTEND=noninteractive

apt-get update -q
apt-get install -y -q build-essential cmake ninja-build git pkg-config bison flex python3 \
  nasm yasm wget curl ca-certificates gnupg xz-utils unzip file autoconf automake libtool

# devkitPro pacman (official installer: https://devkitpro.org/wiki/devkitPro_pacman)
if ! command -v dkp-pacman >/dev/null; then
  cd /tmp
  wget -q https://apt.devkitpro.org/install-devkitpro-pacman -O install-devkitpro-pacman
  chmod +x install-devkitpro-pacman
  yes | ./install-devkitpro-pacman
fi
[ -e /etc/mtab ] || ln -sf /proc/self/mounts /etc/mtab

dkp-pacman -Syu --noconfirm
dkp-pacman -S --needed --noconfirm switch-dev switch-portlibs

cat > /etc/profile.d/devkit-env.sh <<'EOF'
export DEVKITPRO=/opt/devkitpro
export DEVKITARM=/opt/devkitpro/devkitARM
export DEVKITPPC=/opt/devkitpro/devkitPPC
export PATH=/opt/devkitpro/tools/bin:/opt/devkitpro/devkitA64/bin:$PATH
EOF
echo INSTALL_DONE

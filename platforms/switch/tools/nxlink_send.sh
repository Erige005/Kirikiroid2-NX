#!/usr/bin/env bash
# Sends an .nro to the Switch over Wi-Fi (hbmenu netloader: press Y in hbmenu first).
#   wsl -d Ubuntu -- bash ${K2NX_WS:-/mnt/d/Kirikiroid2-Switch}/src/platforms/switch/tools/nxlink_send.sh <switch-ip> [file.nro]
# WSL2 is behind NAT, so broadcast discovery doesn't work: the Switch IP is required.
set -euo pipefail
source /etc/profile.d/devkit-env.sh
IP=${1:?usage: nxlink_send.sh <switch-ip> [file.nro]}
NRO=${2:-${K2NX_WS:-/mnt/d/Kirikiroid2-Switch}/build/smoke/smoke.nro}
nxlink -a "$IP" "$NRO"

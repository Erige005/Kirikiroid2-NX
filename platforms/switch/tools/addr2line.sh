#!/usr/bin/env bash
# Resolves elf offsets from k2s_crash.txt to function/file:line.
#   bash tools/addr2line.sh <k2s_crash.txt | offsets...>
source /etc/profile.d/devkit-env.sh
ELF=${ELF:-$HOME/k2s-build/k2-Debug/kirikiroid2.elf}
if [ -f "$1" ]; then
  OFFS=$(grep -oE "elf\+0x[0-9a-f]+|^  #[0-9]+ 0x[0-9a-f]+" "$1" | grep -oE "0x[0-9a-f]+$")
else
  OFFS="$*"
fi
# the ELF is linked at base 0, so an offset is directly an ELF address
for o in $OFFS; do
  printf '%s  ' "$o"
  aarch64-none-elf-addr2line -f -C -i -e "$ELF" "$o" | paste -sd ' ' | sed 's#${K2NX_WS:-/mnt/d/Kirikiroid2-Switch}/##g'
done

#!/usr/bin/env bash
# Checks that symbols (demangled substrings) are linked into the ELF:  bash tools/has_symbols.sh name1 name2 ...
source /etc/profile.d/devkit-env.sh
ELF=${ELF:-$HOME/k2s-build/k2-Debug/kirikiroid2.elf}
SYMS=$(aarch64-none-elf-nm -C "$ELF")
for s in "$@"; do
  if grep -qF -- "$s" <<< "$SYMS"; then echo "linked : $s"; else echo "MISSING: $s"; fi
done

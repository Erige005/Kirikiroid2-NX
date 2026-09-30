#!/usr/bin/env bash
# Checks that a string is present in the built NRO (guards against testing a stale build).
#   bash tools/nro_has.sh "<string>" [nro]
NRO=${2:-${K2NX_WS:-/mnt/d/Kirikiroid2-Switch}/build/k2/kirikiroid2.nro}
if grep -aqF -- "$1" "$NRO"; then echo "FOUND: $1"; else echo "MISSING: $1"; fi
md5sum "$NRO"

#!/usr/bin/env bash
# Compiler errors whose location matches a file name:  bash tools/errs_for.sh <name-regex> [log]
LOG=${2:-$HOME/k2s-build/k2-Debug/build.log}
grep -E "($1)[^ ]*:[0-9]+:[0-9]+: (fatal )?error" "$LOG" | sed -E 's#${K2NX_WS:-/mnt/d/Kirikiroid2-Switch}/##' | sort -u | head -40

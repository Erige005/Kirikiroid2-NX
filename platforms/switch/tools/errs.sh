#!/usr/bin/env bash
# Summarizes unique compiler errors of a build log:  bash tools/errs.sh <log> [N]
LOG=${1:?log file}; N=${2:-40}
grep -hoE "[^ ]+\.(c|cc|cpp|h|hpp):[0-9]+:[0-9]+: (fatal )?error: .*" "$LOG" \
  | sed -E 's#^${K2NX_WS:-/mnt/d/Kirikiroid2-Switch}/##' | sort -u | head -n "$N"
echo "--- unique error messages:"
grep -hoE "(fatal )?error: .*" "$LOG" | sort | uniq -c | sort -rn | head -n "$N"

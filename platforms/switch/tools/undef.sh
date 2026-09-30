#!/usr/bin/env bash
# Unique undefined symbols from a link log (demangled):  bash tools/undef.sh [log]
LOG=${1:-$HOME/k2s-build/k2-Debug/build.log}
grep -oE "undefined reference to \`[^']+'" "$LOG" | sed -E "s/undefined reference to \`//; s/'$//" \
  | sort | uniq -c | sort -rn
echo "--- multiple definitions:"
grep -oE "multiple definition of \`[^']+'" "$LOG" | sort -u | head -30

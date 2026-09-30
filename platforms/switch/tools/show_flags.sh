#!/usr/bin/env bash
# Shows the -f/-W/-std flags actually used to compile a source file:  bash tools/show_flags.sh <file-name> [target]
B=$HOME/k2s-build/k2-Debug
cd "$B" || exit 1
ninja -t commands "${2:-krkr2core}" | grep -m1 -F "$1" | tr ' ' '\n' | grep -E '^-(f|W|std)' | sort -u

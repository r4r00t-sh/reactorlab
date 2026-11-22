#!/usr/bin/env bash
# Fail if PLACEHOLDER nuclear-data markers remain (release gate).
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

# Fail on PLACEHOLDER tokens in nuclear data / fuelcycle sources (not docs).
data_hits="$(grep -RIn --exclude='*.md' -e 'PLACEHOLDER' data/ 2>/dev/null || true)"
code_hits="$(grep -RIn -e 'PLACEHOLDER' \
  core/include/reactorlab/fuelcycle core/src/fuelcycle 2>/dev/null || true)"
inc_missing=""
if [[ ! -f core/include/reactorlab/fuelcycle/ahwr_collapsed_xs.inc ]]; then
  inc_missing="missing core/include/reactorlab/fuelcycle/ahwr_collapsed_xs.inc"
fi

if [[ -n "$data_hits" || -n "$code_hits" || -n "$inc_missing" ]]; then
  echo "ERROR: PLACEHOLDER nuclear data still present:" >&2
  [[ -n "$data_hits" ]] && echo "$data_hits" >&2
  [[ -n "$code_hits" ]] && echo "$code_hits" >&2
  [[ -n "$inc_missing" ]] && echo "$inc_missing" >&2
  exit 1
fi

echo "OK: no PLACEHOLDER nuclear data markers."
exit 0

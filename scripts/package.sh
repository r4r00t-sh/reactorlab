#!/usr/bin/env bash
# Build Release packages (DEB + TGZ) into dist/
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

PRESET="${PRESET:-release}"
PREFIX="${PREFIX:-$ROOT/dist/install}"
DIST="${DIST:-$ROOT/dist}"

mkdir -p "$DIST"

EXTRA=()
if [[ -n "${Python_EXECUTABLE:-}" ]]; then
  EXTRA+=(-DPython_EXECUTABLE="$Python_EXECUTABLE")
elif [[ -x "$ROOT/venv/bin/python" ]]; then
  EXTRA+=(-DPython_EXECUTABLE="$ROOT/venv/bin/python")
fi
if [[ -n "${Qt6_DIR:-}" ]]; then
  EXTRA+=(-DQt6_DIR="$Qt6_DIR")
elif [[ -d "$ROOT/third_party/Qt/6.8.3/gcc_64/lib/cmake/Qt6" ]]; then
  EXTRA+=(-DQt6_DIR="$ROOT/third_party/Qt/6.8.3/gcc_64/lib/cmake/Qt6")
fi

cmake --preset "$PRESET" "${EXTRA[@]}"
cmake --build --preset "$PRESET"
ctest --preset "$PRESET" --output-on-failure

# Stage install for inspection
cmake --install "$ROOT/build/$PRESET" --prefix "$PREFIX"

# CPack packages land in the build directory
cmake --build "$ROOT/build/$PRESET" --target package
shopt -s nullglob
for f in "$ROOT/build/$PRESET"/reactorlab-*.{deb,tar.gz}; do
  cp -f "$f" "$DIST/"
  echo "Packed: $DIST/$(basename "$f")"
done

echo "Install tree: $PREFIX"
echo "Packages: $DIST"
echo "NOTE: Educational tool — not safety-grade."

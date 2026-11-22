#!/usr/bin/env bash
# Optional AppImage via linuxdeploy (if installed). Falls back to documenting TGZ.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
DIST="${DIST:-$ROOT/dist}"
mkdir -p "$DIST"

if ! command -v linuxdeploy >/dev/null 2>&1; then
  cat <<'EOF'
linuxdeploy not found on PATH.

AppImage packaging is optional. Options:
  1) Install linuxdeploy + linuxdeploy-plugin-qt, then re-run this script.
  2) Use the portable TGZ from ./scripts/package.sh instead.

ReactorLab remains an educational/research tool (not safety-grade).
EOF
  # Still ensure a TGZ exists for users who expected a portable bundle.
  shopt -s nullglob
  tgz=("$DIST"/reactorlab-*.tar.gz)
  shopt -u nullglob
  if ((${#tgz[@]} == 0)); then
    "$ROOT/scripts/package.sh"
  else
    echo "Portable TGZ already present: ${tgz[0]}"
  fi
  exit 0
fi

"$ROOT/scripts/package.sh"

APPDIR="$DIST/ReactorLab.AppDir"
rm -rf "$APPDIR"
mkdir -p "$APPDIR/usr"
# Re-install into AppDir
PRESET="${PRESET:-release}"
cmake --install "$ROOT/build/$PRESET" --prefix "$APPDIR/usr"

# Wrapper
cat > "$APPDIR/AppRun" <<'EOF'
#!/bin/sh
HERE="$(dirname "$(readlink -f "$0")")"
export PATH="$HERE/usr/bin:$PATH"
exec "$HERE/usr/bin/reactorlab_gui" "$@"
EOF
chmod +x "$APPDIR/AppRun"
cp "$ROOT/build/$PRESET/reactorlab.desktop" "$APPDIR/" || true

linuxdeploy --appdir "$APPDIR" --output appimage
mv -f ReactorLab*.AppImage "$DIST/" 2>/dev/null || true
echo "AppImage (if produced) is under $DIST"

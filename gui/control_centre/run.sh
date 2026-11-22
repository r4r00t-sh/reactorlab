#!/usr/bin/env bash
# Launch the live control-centre binary (falls back to QML demo if not built).
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
QT="${QT_ROOT:-$ROOT/third_party/Qt/6.8.3/gcc_64}"
export LD_LIBRARY_PATH="$QT/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
export QML_IMPORT_PATH="$ROOT/gui/control_centre/imports:$QT/qml${QML_IMPORT_PATH:+:$QML_IMPORT_PATH}"
export QML2_IMPORT_PATH="$QML_IMPORT_PATH"
export QT_PLUGIN_PATH="$QT/plugins${QT_PLUGIN_PATH:+:$QT_PLUGIN_PATH}"

BIN=""
for c in "$ROOT/build/dev/gui/control_centre/reactorlab_control_centre" \
         "$ROOT/build/release/gui/control_centre/reactorlab_control_centre"; do
  if [[ -x "$c" ]]; then BIN="$c"; break; fi
done

if [[ -n "$BIN" ]]; then
  exec "$BIN"
fi

echo "control_centre binary not found — building hint:" >&2
echo "  cmake --build --preset dev --target reactorlab_control_centre" >&2
echo "Falling back to QML DemoClock…" >&2
exec "$QT/bin/qml" "$ROOT/gui/control_centre/qml/Main.qml"

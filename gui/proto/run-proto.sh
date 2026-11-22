#!/usr/bin/env bash
# Launch UI-0 QML prototype (fake data only).
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
QT="${QT_ROOT:-$ROOT/third_party/Qt/6.8.3/gcc_64}"
export LD_LIBRARY_PATH="$QT/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
export QML_IMPORT_PATH="$ROOT/gui/proto/imports${QML_IMPORT_PATH:+:$QML_IMPORT_PATH}"
export QML2_IMPORT_PATH="$QML_IMPORT_PATH"
exec "$QT/bin/qml" "$ROOT/gui/proto/qml/Main.qml"

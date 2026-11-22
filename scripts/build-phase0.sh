#!/usr/bin/env bash
# Configure, build, and test Phase 0.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "${ROOT}"

PRESET="${1:-dev}"
PYTHON_BIN="${PYTHON_BIN:-${ROOT}/venv/bin/python}"
if [[ ! -x "${PYTHON_BIN}" ]]; then
  PYTHON_BIN="$(command -v python3)"
fi

EXTRA_ARGS=()
if [[ -d "${ROOT}/third_party/Qt/6.8.3/gcc_64" ]]; then
  EXTRA_ARGS+=("-DQt6_DIR=${ROOT}/third_party/Qt/6.8.3/gcc_64/lib/cmake/Qt6")
elif [[ -d "${ROOT}/third_party/Qt/6.8.3/linux_gcc_64" ]]; then
  EXTRA_ARGS+=("-DQt6_DIR=${ROOT}/third_party/Qt/6.8.3/linux_gcc_64/lib/cmake/Qt6")
fi

export Python_EXECUTABLE="${PYTHON_BIN}"
cmake --preset "${PRESET}" -DPython_EXECUTABLE="${PYTHON_BIN}" "${EXTRA_ARGS[@]}"
cmake --build --preset "${PRESET}"
ctest --preset "${PRESET}" --output-on-failure

export PYTHONPATH="${ROOT}/build/${PRESET}/python${PYTHONPATH:+:${PYTHONPATH}}"
"${PYTHON_BIN}" -m pytest "${ROOT}/python/tests" -q

echo "Phase 0 build+tests OK (preset=${PRESET})"

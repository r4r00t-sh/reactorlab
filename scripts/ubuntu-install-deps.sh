#!/usr/bin/env bash
# Install Ubuntu packages needed for ReactorLab Phase 0+.
# Requires sudo. Safe to re-run.
set -euo pipefail

if [[ "${EUID}" -eq 0 ]]; then
  SUDO=""
else
  SUDO="sudo"
fi

export DEBIAN_FRONTEND=noninteractive

${SUDO} apt-get update
${SUDO} apt-get install -y \
  build-essential ninja-build cmake pkg-config git \
  clang clang-format clang-tidy \
  python3 python3-dev python3-pip python3-venv \
  libgl1-mesa-dev \
  libeigen3-dev libfmt-dev libspdlog-dev libyaml-cpp-dev \
  libhdf5-dev libsundials-dev libgtest-dev libomp-dev \
  qt6-base-dev qt6-base-dev-tools

echo "System packages installed. Next:"
echo "  python3 -m venv venv && source venv/bin/activate"
echo "  pip install nanobind pytest ruff pre-commit"
echo "  cmake --preset dev && cmake --build --preset dev"

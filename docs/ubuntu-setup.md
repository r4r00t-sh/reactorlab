# Ubuntu toolchain setup (22.04 / 24.04)

Run these commands on a fresh Ubuntu desktop/server before building ReactorLab.

## Base toolchain

```bash
sudo apt-get update
sudo apt-get install -y \
  build-essential ninja-build cmake pkg-config git \
  clang clang-format clang-tidy \
  python3 python3-dev python3-pip python3-venv \
  libgl1-mesa-dev
```

CMake must be **≥ 3.25**. On older Ubuntu images, install a newer CMake from Kitware or use a pip/snap build if `apt` is too old.

## Core science libraries

```bash
sudo apt-get install -y \
  libeigen3-dev \
  libfmt-dev \
  libspdlog-dev \
  libyaml-cpp-dev \
  libhdf5-dev \
  libsundials-dev \
  libgtest-dev \
  libomp-dev
```

Phase 0 can also fetch fmt / spdlog / GTest / Eigen via CMake FetchContent (`REACTORLAB_FETCH_DEPS=ON`).

## Qt 6 (GUI)

```bash
sudo apt-get install -y qt6-base-dev qt6-base-dev-tools
```

Verify:

```bash
cmake --find-package -DNAME=Qt6 -DCOMPILER_ID=GNU -DLANGUAGE=CXX -DMODE=EXIST
```

## Python tooling (venv)

```bash
cd /path/to/reactorlab
python3 -m venv venv
source venv/bin/activate
pip install -U pip
pip install nanobind pytest ruff pre-commit numpy matplotlib pyyaml
pre-commit install
```

## Optional: vcpkg (reproducible CI / locked deps)

```bash
git clone https://github.com/microsoft/vcpkg.git ~/vcpkg
~/vcpkg/bootstrap-vcpkg.sh
export VCPKG_ROOT=~/vcpkg
cmake --preset release \
  -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake
```

## Sanitizer builds

```bash
cmake --preset asan
cmake --build --preset asan
ctest --preset asan --output-on-failure
```

## Verify Phase 0

```bash
cmake --preset dev
cmake --build --preset dev
ctest --preset dev --output-on-failure
PYTHONPATH=build/dev/python pytest python/tests -q
./build/dev/gui/reactorlab_gui
```

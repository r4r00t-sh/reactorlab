# Third-party local installs

This directory holds optional local toolchains that are **not** committed to git
(see `.gitignore`).

## Qt (no-sudo / offline)

```bash
source ../venv/bin/activate
pip install aqtinstall
aqt install-qt linux desktop 6.8.3 linux_gcc_64 -O .
# Lands at Qt/6.8.3/gcc_64
```

Then configure with:

```bash
cmake --preset dev -DQt6_DIR=$PWD/Qt/6.8.3/gcc_64/lib/cmake/Qt6
```

Prefer system `qt6-base-dev` when `sudo` is available.

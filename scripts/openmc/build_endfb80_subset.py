#!/usr/bin/env python3
"""Build a minimal ENDF/B-VIII.0 HDF5 library (NJOY) for AHWR-style collapse.

Sources ENDF files previously extracted under .tools/nuclear_data/endf/.
Requires the micromamba `openmc` env (openmc + njoy2016).
"""
from __future__ import annotations

import argparse
import os
import sys
from pathlib import Path

import openmc.data

ROOT = Path(__file__).resolve().parents[2]
DEFAULT_ENDF = ROOT / ".tools" / "nuclear_data" / "endf"
DEFAULT_HDF5 = ROOT / ".tools" / "nuclear_data" / "hdf5" / "endfb-viii.0-subset"


NEUTRONS = {
    "H1": "neutron/n-001_H_001.endf",
    "H2": "neutron/n-001_H_002.endf",
    "O16": "neutron/n-008_O_016.endf",
    "Zr90": "neutron/n-040_Zr_090.endf",
    "Zr91": "neutron/n-040_Zr_091.endf",
    "Zr92": "neutron/n-040_Zr_092.endf",
    "Zr94": "neutron/n-040_Zr_094.endf",
    "Zr96": "neutron/n-040_Zr_096.endf",
    "Th232": "neutron/n-090_Th_232.endf",
    "Pa233": "neutron/n-091_Pa_233.endf",
    "U233": "neutron/n-092_U_233.endf",
    "U235": "neutron/n-092_U_235.endf",
    "Pu239": "neutron/n-094_Pu_239.endf",
}

# (name, endf_rel, neutron_keys for sab)
THERMAL = [
    ("c_H_in_H2O", "thermal/tsl-HinH2O.endf", ["H1"]),
    ("c_D_in_D2O", "thermal/tsl-DinD2O.endf", ["H2"]),
]


def build(endf_dir: Path, out_dir: Path, temperatures: list[float]) -> Path:
    neutron_dir = out_dir / "neutron"
    thermal_dir = out_dir / "thermal"
    neutron_dir.mkdir(parents=True, exist_ok=True)
    thermal_dir.mkdir(parents=True, exist_ok=True)

    library = openmc.data.DataLibrary()

    for name, rel in NEUTRONS.items():
        endf_path = endf_dir / rel
        h5_path = neutron_dir / f"{name}.h5"
        if not endf_path.is_file():
            raise FileNotFoundError(endf_path)
        if h5_path.is_file():
            print(f"[skip] {name}")
        else:
            print(f"[njoy] {name} @ {temperatures} K …", flush=True)
            data = openmc.data.IncidentNeutron.from_njoy(
                str(endf_path), temperatures=temperatures
            )
            data.export_to_hdf5(str(h5_path), "w")
        library.register_file(h5_path)

    for name, rel, _neutrons in THERMAL:
        endf_path = endf_dir / rel
        h5_path = thermal_dir / f"{name}.h5"
        if not endf_path.is_file():
            raise FileNotFoundError(endf_path)
        if h5_path.is_file():
            print(f"[skip] {name}")
        else:
            print(f"[njoy-sab] {name} …", flush=True)
            data = openmc.data.ThermalScattering.from_njoy(
                str(endf_dir / NEUTRONS[_neutrons[0]]),
                str(endf_path),
            )
            data.export_to_hdf5(str(h5_path), "w")
        library.register_file(h5_path)

    xs = out_dir / "cross_sections.xml"
    library.export_to_xml(xs)
    print(f"Wrote {xs}")
    return xs


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--endf-dir", type=Path, default=DEFAULT_ENDF)
    ap.add_argument("--out-dir", type=Path, default=DEFAULT_HDF5)
    ap.add_argument(
        "--temperatures",
        type=float,
        nargs="+",
        default=[293.6, 600.0],
        help="Kelvin temperatures for incident-neutron data",
    )
    args = ap.parse_args()
    build(args.endf_dir, args.out_dir, args.temperatures)
    return 0


if __name__ == "__main__":
    sys.exit(main())

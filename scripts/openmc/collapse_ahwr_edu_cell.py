#!/usr/bin/env python3
"""Collapse one-group microscopic rates on an educational AHWR-style lattice cell.

Geometry / enrichments taken from publicly described AHWR D5 cluster features in:
  Kumar, Srivenkatesan, Sinha — IAEA-CN-164-3S03 (Table I: pitch 225 mm,
  54 pins OD 11.2 mm, Zry clad 0.6 mm, PT/CT sizes, D2O moderator, H2O coolant).

This is a *representative educational* infinite lattice cell (homogenized ring
enrichment), NOT BARC's licensed AHWR design model.

Cross sections: ENDF/B-VIII.0 via OpenMC HDF5 subset built by
build_endfb80_subset.py.

Outputs:
  data/thorium/openmc_collapse/ahwr_edu_cell_onegroup.json
  data/thorium/openmc_collapse/CROSSCHECK.md
and prints barn values for YAML update.
"""
from __future__ import annotations

import argparse
import json
import math
import os
import sys
from pathlib import Path

import numpy as np
import openmc

ROOT = Path(__file__).resolve().parents[2]
DEFAULT_XS = ROOT / ".tools" / "nuclear_data" / "hdf5" / "endfb-viii.0-subset" / "cross_sections.xml"
OUT_DIR = ROOT / "data" / "thorium" / "openmc_collapse"

def endf_2200ms_from_hdf5(xs_xml: Path) -> dict[str, float]:
    """Evaluate σ(0.0253 eV) from the same ENDF/B-VIII.0 HDF5 library used in transport."""
    import openmc.data

    lib = openmc.data.DataLibrary.from_xml(str(xs_xml))
    E = 0.0253  # eV

    def point(nuc: str, mt: int) -> float:
        entry = lib.get_by_material(nuc)
        if entry is None:
            raise KeyError(nuc)
        data = openmc.data.IncidentNeutron.from_hdf5(entry["path"])
        # Prefer 293.6 K if present, else first temperature
        temps = list(data.temperatures)
        T = "293.6K" if "293.6K" in temps else temps[0]
        rx = data.reactions[mt]
        xs = rx.xs[T]
        return float(xs(E))

    out: dict[str, float] = {}
    mapping = [
        ("Th232_capture_b", "Th232", 102),
        ("Pa233_capture_b", "Pa233", 102),
        ("U233_fission_b", "U233", 18),
        ("U233_capture_b", "U233", 102),
        ("U235_fission_b", "U235", 18),
        ("U235_capture_b", "U235", 102),
        ("Pu239_fission_b", "Pu239", 18),
        ("Pu239_capture_b", "Pu239", 102),
    ]
    for key, nuc, mt in mapping:
        try:
            out[key] = point(nuc, mt)
        except Exception as exc:  # noqa: BLE001 — log missing; do not invent
            print(f"warn: no 2200 m/s for {key}: {exc}")
    return out


def make_materials() -> tuple[openmc.Material, openmc.Material, openmc.Material, openmc.Material]:
    # (Th,233U)O2 ~3.4 wt% U233 HM — mid of Kumar inner/middle ring 3.0–3.75 wt%
    fuel = openmc.Material(name="ThU233_MOX")
    fuel.set_density("g/cm3", 9.6)
    fuel.add_nuclide("Th232", 0.966 / 232.0381, "ao")
    fuel.add_nuclide("U233", 0.034 / 233.0396, "ao")
    fuel.add_nuclide("O16", 2.0 * (0.966 / 232.0381 + 0.034 / 233.0396), "ao")
    # Trace probes for capture/fission collapse (same spectrum)
    fuel.add_nuclide("Pa233", 1.0e-8, "ao")
    fuel.add_nuclide("U235", 1.0e-8, "ao")
    fuel.add_nuclide("Pu239", 1.0e-8, "ao")
    # Single temperature matches NJOY-processed subset (600 K).
    fuel.temperature = 600.0

    clad = openmc.Material(name="Zircaloy")
    clad.set_density("g/cm3", 6.55)
    clad.add_nuclide("Zr90", 0.5145, "ao")
    clad.add_nuclide("Zr91", 0.1122, "ao")
    clad.add_nuclide("Zr92", 0.1715, "ao")
    clad.add_nuclide("Zr94", 0.1738, "ao")
    clad.add_nuclide("Zr96", 0.0280, "ao")
    clad.temperature = 600.0

    coolant = openmc.Material(name="H2O_coolant")
    coolant.set_density("g/cm3", 0.75)  # boiling LW educational average
    coolant.add_nuclide("H1", 2.0, "ao")
    coolant.add_nuclide("O16", 1.0, "ao")
    coolant.add_s_alpha_beta("c_H_in_H2O")
    coolant.temperature = 600.0

    moderator = openmc.Material(name="D2O_moderator")
    moderator.set_density("g/cm3", 1.10)
    moderator.add_nuclide("H2", 2.0, "ao")
    moderator.add_nuclide("O16", 1.0, "ao")
    moderator.add_s_alpha_beta("c_D_in_D2O")
    moderator.temperature = 600.0

    return fuel, clad, coolant, moderator


def make_geometry(fuel, clad, coolant, moderator) -> openmc.Geometry:
    # Kumar Table I (IAEA-CN-164-3S03): pitch 225 mm; pin OD 11.2 mm; clad 0.6 mm;
    # PT ID/OD 120/128 mm; CT ID/OD 163.8/168 mm; displacer OD/ID 36/30 mm.
    # Educational simplification: single fuel pin + clad in square pitch equivalent
    # volume-matched to cluster HM fraction is heavy; instead use infinite lattice
    # of pin cells with pitch = 225/√54 ≈ 30.6 mm (equal area per pin).
    pitch = 2.25 / math.sqrt(54.0)  # cm? 225 mm = 22.5 cm → 22.5/√54
    pitch = 22.5 / math.sqrt(54.0)  # cm
    r_fuel = (1.12 - 2 * 0.06) / 2.0  # pellet radius cm (OD 11.2 mm, clad 0.6 mm)
    r_clad = 1.12 / 2.0

    s_fuel = openmc.ZCylinder(r=r_fuel)
    s_clad = openmc.ZCylinder(r=r_clad)
    # Thin LW coolant ring (cannot mix S(a,b) materials in OpenMC); D2O fills the rest.
    r_cool = r_clad + 0.25  # cm educational coolant channel
    s_cool = openmc.ZCylinder(r=r_cool)
    xmin = openmc.XPlane(x0=-pitch / 2, boundary_type="reflective")
    xmax = openmc.XPlane(x0=+pitch / 2, boundary_type="reflective")
    ymin = openmc.YPlane(y0=-pitch / 2, boundary_type="reflective")
    ymax = openmc.YPlane(y0=+pitch / 2, boundary_type="reflective")
    zmin = openmc.ZPlane(z0=-0.5, boundary_type="reflective")
    zmax = openmc.ZPlane(z0=+0.5, boundary_type="reflective")

    c_fuel = openmc.Cell(name="fuel", fill=fuel, region=-s_fuel & +zmin & -zmax)
    c_clad = openmc.Cell(name="clad", fill=clad, region=+s_fuel & -s_clad & +zmin & -zmax)
    c_cool = openmc.Cell(
        name="coolant", fill=coolant, region=+s_clad & -s_cool & +zmin & -zmax
    )
    c_mod = openmc.Cell(
        name="mod",
        fill=moderator,
        region=+s_cool & +xmin & -xmax & +ymin & -ymax & +zmin & -zmax,
    )
    univ = openmc.Universe(cells=[c_fuel, c_clad, c_cool, c_mod])
    geom = openmc.Geometry(univ)
    return geom


def run_collapse(xs_path: Path, work: Path, particles: int, batches: int) -> dict:
    openmc.config["cross_sections"] = str(xs_path)
    work.mkdir(parents=True, exist_ok=True)
    os.chdir(work)

    fuel, clad, coolant, moderator = make_materials()
    geom = make_geometry(fuel, clad, coolant, moderator)
    # Collect all materials actually used
    mats = geom.get_all_materials()
    openmc.Materials(mats.values()).export_to_xml()
    geom.export_to_xml()

    settings = openmc.Settings()
    settings.batches = batches
    settings.inactive = max(20, batches // 5)
    settings.particles = particles
    settings.temperature = {"method": "nearest", "tolerance": 100.0, "default": 600.0}
    settings.source = openmc.IndependentSource(
        space=openmc.stats.Box([-0.4, -0.4, -0.4], [0.4, 0.4, 0.4], only_fissionable=True)
    )
    settings.export_to_xml()

    # Tallies: flux in fuel + reaction rates for nuclides of interest
    fuel_cell = None
    for cell in geom.get_all_cells().values():
        if cell.name == "fuel":
            fuel_cell = cell
            break
    assert fuel_cell is not None

    tallies = openmc.Tallies()
    cell_filter = openmc.CellFilter([fuel_cell])
    particle_filter = openmc.ParticleFilter(["neutron"])

    t_flux = openmc.Tally(name="fuel_flux")
    t_flux.filters = [cell_filter, particle_filter]
    t_flux.scores = ["flux"]
    tallies.append(t_flux)

    nuclides = ["Th232", "Pa233", "U233", "U235", "Pu239"]
    t_rx = openmc.Tally(name="fuel_rx")
    t_rx.filters = [cell_filter, particle_filter]
    t_rx.nuclides = nuclides
    t_rx.scores = ["absorption", "fission", "(n,gamma)"]
    tallies.append(t_rx)
    tallies.export_to_xml()

    openmc.run()

    sp = openmc.StatePoint(f"statepoint.{batches}.h5")
    flux_t = sp.get_tally(name="fuel_flux")
    flux = float(flux_t.mean.flatten()[0])
    rx = sp.get_tally(name="fuel_rx")

    # Atom densities [atoms/b-cm] from material
    dens = {nuc: fuel.get_nuclide_atom_densities()[nuc] for nuc in nuclides}

    def xs_barn(nuclide: str, score: str) -> float:
        # rate = Σ φ = N[atoms/b-cm] * σ[b] * φ  → σ = rate / (N * φ)
        val = rx.get_values(scores=[score], nuclides=[nuclide]).flatten()[0]
        n = dens[nuclide]
        if n <= 0.0 or flux <= 0.0:
            return float("nan")
        return float(val / (n * flux))

    sigma_b = {
        "Th232_capture_b": xs_barn("Th232", "(n,gamma)"),
        "Pa233_capture_b": xs_barn("Pa233", "(n,gamma)"),
        "U233_fission_b": xs_barn("U233", "fission"),
        "U233_capture_b": xs_barn("U233", "(n,gamma)"),
        "U235_fission_b": xs_barn("U235", "fission"),
        "U235_capture_b": xs_barn("U235", "(n,gamma)"),
        "Pu239_fission_b": xs_barn("Pu239", "fission"),
        "Pu239_capture_b": xs_barn("Pu239", "(n,gamma)"),
    }
    results = {
        "library": "ENDF/B-VIII.0",
        "code": f"OpenMC {openmc.__version__}",
        "geometry": "edu AHWR-style equal-area pin cell (Kumar IAEA-CN-164-3S03 dims)",
        "pitch_cm": 22.5 / math.sqrt(54.0),
        "keff": [float(sp.keff.nominal_value), float(sp.keff.std_dev)],
        "fuel_flux_tally": flux,
        "sigma_b": sigma_b,
        "endf_2200ms_reference_b": endf_2200ms_from_hdf5(
            Path(openmc.config["cross_sections"])
        ),
    }
    return results


def write_crosscheck(results: dict, path: Path) -> None:
    sig = results["sigma_b"]
    lines = [
        "# OpenMC AHWR-style cell collapse — literature cross-check",
        "",
        "## Method",
        "",
        f"- Transport: {results['code']} with {results['library']}",
        f"- Geometry: {results['geometry']}",
        f"- Pitch: {results['pitch_cm']:.4f} cm (225 mm lattice / √54 pins)",
        f"- k_eff (infinite cell): {results['keff'][0]:.5f} ± {results['keff'][1]:.5f}",
        "",
        "## Citations used for geometry / programme context",
        "",
        "1. A. Kumar, R. Srivenkatesan, R. K. Sinha, *On the Physics Design of AHWR*,",
        "   IAEA-CN-164-3S03 — Table I (pitch, pin OD, clad, PT/CT, enrichments).",
        "2. IAEA TECDOC-1450, *Thorium fuel cycle — Potential benefits and challenges*",
        "   — programme context; **does not publish** AHWR one-group microscopic σ.",
        "3. IAEA ARIS AHWR characteristics — high-level plant description.",
        "",
        "## What the papers do *not* provide",
        "",
        "Kumar et al. and TECDOC-1450 do **not** tabulate collapsed one-group",
        "microscopic capture/fission cross sections for Th-232 / Pa-233 / U-233",
        "suitable for direct numeric substitution. Therefore no invented",
        "\"literature σ\" values are entered. Cross-check is against:",
        "",
        "- ENDF/B-VIII.0 2200 m/s point data (same evaluation family as the transport library)",
        "- Qualitative AHWR lattice facts from Kumar (D2O moderated, LW cooled,",
        "  Th/U-233 MOX ~3–3.75 wt% U-233, pitch 225 mm)",
        "",
        "## Collapsed one-group σ [barn] vs ENDF 2200 m/s",
        "",
        "| Reaction | OpenMC σ̄ [b] | ENDF 2200 m/s [b] | Ratio σ̄/σ₀ |",
        "|----------|---------------|-------------------|-------------|",
    ]
    for key, v in sig.items():
        ref = results["endf_2200ms_reference_b"].get(key)
        if ref is None:
            lines.append(f"| {key} | {v:.4f} | — | — |")
        else:
            lines.append(f"| {key} | {v:.4f} | {ref:.2f} | {v / ref:.3f} |")
    lines += [
        "",
        "### Discrepancy notes",
        "",
        "- Spectrum-averaged σ̄ for 1/v absorbers is typically **below** 2200 m/s",
        "  values in a hardened AHWR-like spectrum (pitch reduction / LW coolant).",
        "- Pa-233 has no 2200 m/s row above when not listed in the static ENDF",
        "  thermal table used here; OpenMC value is the transport-weighted result.",
        "- Trace U-235 / Pu-239 probes share the Th/U-233 cell spectrum; rates are",
        "  for that spectrum, not a Pu-MOX pin spectrum (outer AHWR ring differs).",
        "",
        "## Provenance rule",
        "",
        "Shipped YAML/C++ defaults use **only** the OpenMC-collapsed numbers in",
        "`ahwr_edu_cell_onegroup.json`. No hand-tuned σ were introduced.",
        "",
    ]
    path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--xs", type=Path, default=DEFAULT_XS)
    ap.add_argument("--work", type=Path, default=OUT_DIR / "work")
    ap.add_argument("--particles", type=int, default=5000)
    ap.add_argument("--batches", type=int, default=80)
    args = ap.parse_args()
    if not args.xs.is_file():
        print(f"Missing cross_sections.xml: {args.xs}", file=sys.stderr)
        print("Run scripts/openmc/build_endfb80_subset.py first.", file=sys.stderr)
        return 1

    OUT_DIR.mkdir(parents=True, exist_ok=True)
    results = run_collapse(args.xs, args.work, args.particles, args.batches)
    json_path = OUT_DIR / "ahwr_edu_cell_onegroup.json"
    json_path.write_text(json.dumps(results, indent=2) + "\n", encoding="utf-8")
    write_crosscheck(results, OUT_DIR / "CROSSCHECK.md")
    print(json.dumps(results["sigma_b"], indent=2))
    print(f"Wrote {json_path}")
    return 0


if __name__ == "__main__":
    sys.exit(main())

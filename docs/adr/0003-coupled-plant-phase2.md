# ADR 0003: Coupled plant ODE (Phase 2)

- **Status:** Accepted
- **Date:** 2026-11-23

## Context

Phase 1 point kinetics needs temperature feedback and fission-product poisons for
educational transients (xenon dip / peak, rod motion, Doppler).

## Decision

- Single **13-state CVODE** system in `libreactorlab` (`reactorlab::plant`).
- Poisons written in atom-density form with documented \(\varphi_0\), \(\Sigma_{f0}\).
- Two-node lumped fuel/coolant thermal model with constant \(\alpha_D\), \(\alpha_M\).
- Linear rod worth; external reactivity schedules reuse Phase 1 helpers.
- Python reference mirrors the same RHS for cross-checks.

## Consequences

- GUI becomes a plant dashboard (P, Tf, Tc, Xe, rod, alarms) still without physics logic.
- Nuclear/thermal defaults live in one `PlantParams` struct + PROVENANCE table.
- Still not a safety analysis model (no decay heat, no 3D, no licensing basis).

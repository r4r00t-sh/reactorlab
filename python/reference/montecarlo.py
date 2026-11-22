"""Multigroup analog Monte Carlo k-eigenvalue reference.

Theory: docs/theory/monte-carlo-multigroup.md
Units: cm, cm^-1; seeded RNG for reproducibility.
"""

from __future__ import annotations

from dataclasses import dataclass

import numpy as np


@dataclass
class MgMaterial:
    """Multigroup macroscopic XS. Arrays length G."""

    Sigma_t: np.ndarray
    Sigma_a: np.ndarray
    nuSigma_f: np.ndarray
    # Scatter matrix Σs[g_from, g_to] (G×G); only down/within used educationally
    Sigma_s: np.ndarray

    def __post_init__(self) -> None:
        self.Sigma_t = np.asarray(self.Sigma_t, dtype=float)
        self.Sigma_a = np.asarray(self.Sigma_a, dtype=float)
        self.nuSigma_f = np.asarray(self.nuSigma_f, dtype=float)
        self.Sigma_s = np.asarray(self.Sigma_s, dtype=float)
        g = self.Sigma_t.shape[0]
        if self.Sigma_a.shape != (g,) or self.nuSigma_f.shape != (g,) or self.Sigma_s.shape != (g, g):
            raise ValueError("inconsistent group structure")

    @property
    def n_groups(self) -> int:
        return int(self.Sigma_t.shape[0])


def material_2g_infinite() -> MgMaterial:
    """Analytic k_inf = 0.8333... (see data/benchmarks/mg_infinite.yaml)."""
    # g0 fast, g1 thermal
    St = np.array([0.06, 0.08])
    Sa = np.array([0.02, 0.08])
    nSf = np.array([0.0, 0.10])
    Ss = np.zeros((2, 2))
    Ss[0, 1] = 0.04  # downscatter only
    return MgMaterial(St, Sa, nSf, Ss)


def analytic_k_inf(mat: MgMaterial) -> float:
    """Two-group χ=(1,0), no upscatter: k=(νΣf1 + νΣf2 Σs12/Σa2)/Σr1."""
    if mat.n_groups != 2:
        raise ValueError("analytic helper is 2-group only")
    Sr1 = mat.Sigma_a[0] + mat.Sigma_s[0, 1]
    return float((mat.nuSigma_f[0] + mat.nuSigma_f[1] * mat.Sigma_s[0, 1] / mat.Sigma_a[1]) / Sr1)


@dataclass
class McResult:
    keff: float
    keff_std: float
    cycle_keff: np.ndarray
    flux: np.ndarray  # mean track-length flux by group (relative)
    flux_std: np.ndarray
    n_active: int
    seed: int


def _sample_group_scatter(rng: np.random.Generator, mat: MgMaterial, g: int) -> int | None:
    """Return new group or None if absorption (non-scatter)."""
    St = mat.Sigma_t[g]
    # Build discrete CDF: scatter to g' then absorption
    probs = mat.Sigma_s[g] / St
    p_abs = mat.Sigma_a[g] / St
    # Numerical guard: remaining probability → absorb
    u = rng.random()
    cum = 0.0
    for gp in range(mat.n_groups):
        cum += probs[gp]
        if u < cum:
            return gp
    # absorption (or fission lumped into absorption channel for tracking)
    _ = p_abs
    return None


def infinite_medium_keff(
    mat: MgMaterial,
    n_histories: int = 5000,
    seed: int = 1,
) -> tuple[float, float]:
    """Collision-estimator k_inf from χ=(1,0,...,0) source (no leakage)."""
    rng = np.random.default_rng(seed)
    scores = np.empty(n_histories)
    for h in range(n_histories):
        g = 0  # fission spectrum χ1=1
        score = 0.0
        alive = True
        while alive:
            # collision estimator contribution
            score += mat.nuSigma_f[g] / mat.Sigma_t[g]
            g_new = _sample_group_scatter(rng, mat, g)
            if g_new is None:
                alive = False
            else:
                g = g_new
        scores[h] = score
    mean = float(np.mean(scores))
    std = float(np.std(scores, ddof=1) / np.sqrt(n_histories))
    return mean, std


@dataclass
class SlabGeometry:
    """1D slab 0..L with vacuum BC; infinite transverse."""

    L: float = 20.0


@dataclass
class FissionSite:
    x: float
    group: int  # birth group from χ


def _isotropic_mu(rng: np.random.Generator) -> float:
    return 2.0 * rng.random() - 1.0


def power_iteration_slab(
    mat: MgMaterial,
    geom: SlabGeometry,
    n_histories: int = 2000,
    n_inactive: int = 20,
    n_active: int = 40,
    seed: int = 42,
) -> McResult:
    """Analog power iteration in a homogeneous 1D slab (vacuum BC)."""
    rng = np.random.default_rng(seed)
    # Initial fission bank: uniform in slab, group 0
    bank = [FissionSite(x=rng.random() * geom.L, group=0) for _ in range(n_histories)]
    cycle_k: list[float] = []
    flux_sum = np.zeros(mat.n_groups)
    flux_sumsq = np.zeros(mat.n_groups)
    active_count = 0

    for cycle in range(n_inactive + n_active):
        new_bank: list[FissionSite] = []
        fission_neutrons = 0.0
        track = np.zeros(mat.n_groups)
        # Transport each source neutron
        for site in bank:
            rng_h = np.random.default_rng(rng.integers(0, 2**63 - 1))
            x = site.x
            g = site.group
            mu = _isotropic_mu(rng_h)
            alive = True
            while alive:
                St = mat.Sigma_t[g]
                ell = -np.log(rng_h.random()) / St
                x_new = x + mu * ell
                if x_new < 0.0 or x_new > geom.L:
                    # leak — track length to boundary
                    if mu > 0:
                        dist = (geom.L - x) / mu
                    else:
                        dist = (0.0 - x) / mu
                    track[g] += max(dist, 0.0)
                    alive = False
                    break
                track[g] += ell
                x = x_new
                # collision
                fission_neutrons += mat.nuSigma_f[g] / St
                # bank fission sites (expected value → Bernoulli branch)
                nu_expect = mat.nuSigma_f[g] / St
                n_f = int(nu_expect)
                if rng_h.random() < (nu_expect - n_f):
                    n_f += 1
                for _ in range(n_f):
                    new_bank.append(FissionSite(x=x, group=0))
                g_new = _sample_group_scatter(rng_h, mat, g)
                if g_new is None:
                    alive = False
                else:
                    g = g_new
                    mu = _isotropic_mu(rng_h)

        k_cycle = fission_neutrons / max(len(bank), 1)
        # Resample bank to fixed size
        if not new_bank:
            new_bank = [FissionSite(x=geom.L * 0.5, group=0) for _ in range(n_histories)]
        idx = rng.integers(0, len(new_bank), size=n_histories)
        bank = [new_bank[i] for i in idx]

        if cycle >= n_inactive:
            cycle_k.append(k_cycle)
            flux_sum += track / (n_histories * geom.L)
            flux_sumsq += (track / (n_histories * geom.L)) ** 2
            active_count += 1

    ks = np.array(cycle_k)
    keff = float(np.mean(ks))
    keff_std = float(np.std(ks, ddof=1) / np.sqrt(len(ks))) if len(ks) > 1 else 0.0
    flux = flux_sum / active_count
    flux_std = np.sqrt(np.maximum(flux_sumsq / active_count - flux**2, 0.0) / active_count)
    return McResult(keff, keff_std, ks, flux, flux_std, n_active, seed)

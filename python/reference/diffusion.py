"""Two-group diffusion eigenvalue reference (1D/2D FD + power iteration).

Theory: docs/theory/two-group-diffusion.md
Benchmark data: data/benchmarks/*.yaml (see data/PROVENANCE.md)

Units: length cm, D cm, Σ cm^-1, flux arbitrary.
"""

from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path

import numpy as np
from scipy import sparse
from scipy.sparse.linalg import factorized

ROOT = Path(__file__).resolve().parents[2]
BENCH_DIR = ROOT / "data" / "benchmarks"


@dataclass
class TwoGroupXS:
    D: np.ndarray  # (2,)
    Sigma_a: np.ndarray
    nuSigma_f: np.ndarray
    Sigma_s12: float

    @property
    def Sigma_r1(self) -> float:
        return float(self.Sigma_a[0] + self.Sigma_s12)


def _parse_map(text: str) -> np.ndarray:
    rows = []
    for line in text.strip().splitlines():
        rows.append([int(x) for x in line.split()])
    return np.array(rows, dtype=int)


def load_yaml_benchmark(path: Path) -> dict:
    import yaml

    with path.open() as f:
        return yaml.safe_load(f)


@dataclass
class DiffusionResult:
    keff: float
    phi1: np.ndarray
    phi2: np.ndarray
    iterations: int


def power_iteration(
    M: sparse.spmatrix,
    F: sparse.spmatrix,
    n_unknowns: int,
    tol: float = 1e-7,
    max_iter: int = 500,
) -> tuple[float, np.ndarray, int]:
    """Power iteration: M φ = (1/k) F φ  ⇒  solve M ψ = F φ, k = ||Fψ||/||Fφ||."""
    solve = factorized(M.tocsc())
    phi = np.ones(n_unknowns)
    # normalize by fission source norm
    s = F @ phi
    snorm = np.linalg.norm(s)
    if snorm == 0.0:
        raise RuntimeError("zero fission source — check materials")
    s /= snorm
    k = 1.0
    for it in range(1, max_iter + 1):
        phi = solve(s)
        s_new = F @ phi
        sn_new = np.linalg.norm(s_new)
        k_new = sn_new  # since ||s||=1
        s_new /= sn_new
        if abs(k_new - k) / max(abs(k_new), 1e-12) < tol and np.linalg.norm(s_new - s) < tol:
            return float(k_new), phi, it
        k = k_new
        s = s_new
    return float(k), phi, max_iter


def solve_1d_homogeneous(
    xs: TwoGroupXS,
    L: float,
    n: int,
    tol: float = 1e-8,
) -> DiffusionResult:
    """1D slab, zero-flux BC, single material."""
    dx = L / n
    # unknowns: [φ1_0..φ1_{n-1}, φ2_0..φ2_{n-1}] interior cells
    N = n
    D1, D2 = xs.D
    Sr1 = xs.Sigma_r1
    Sa2 = float(xs.Sigma_a[1])
    Ss12 = xs.Sigma_s12
    nf1, nf2 = xs.nuSigma_f

    rows: list[int] = []
    cols: list[int] = []
    data: list[float] = []

    def add(r: int, c: int, v: float) -> None:
        rows.append(r)
        cols.append(c)
        data.append(v)

    for i in range(N):
        # Interior face coupling uses cell-center spacing dx.
        a_int1 = D1 / dx**2
        a_int2 = D2 / dx**2
        # Zero-flux BC at domain edge: distance from cell center to boundary is dx/2
        # ⇒ leakage coeff 2D/dx^2 on the diagonal.
        a_bnd1 = 2.0 * D1 / dx**2
        a_bnd2 = 2.0 * D2 / dx**2

        diag1 = Sr1
        diag2 = Sa2
        r1 = i
        r2 = N + i

        if i > 0:
            add(r1, i - 1, -a_int1)
            add(r2, N + i - 1, -a_int2)
            diag1 += a_int1
            diag2 += a_int2
        else:
            diag1 += a_bnd1
            diag2 += a_bnd2

        if i < N - 1:
            add(r1, i + 1, -a_int1)
            add(r2, N + i + 1, -a_int2)
            diag1 += a_int1
            diag2 += a_int2
        else:
            diag1 += a_bnd1
            diag2 += a_bnd2

        add(r1, r1, diag1)
        add(r2, r2, diag2)
        add(r2, r1, -Ss12)

    M = sparse.coo_matrix((data, (rows, cols)), shape=(2 * N, 2 * N)).tocsr()

    # F: only couples to fission production in group 1 equation
    f_rows: list[int] = []
    f_cols: list[int] = []
    f_data: list[float] = []
    for i in range(N):
        if nf1 != 0.0:
            f_rows.append(i)
            f_cols.append(i)
            f_data.append(nf1)
        if nf2 != 0.0:
            f_rows.append(i)
            f_cols.append(N + i)
            f_data.append(nf2)
    F = sparse.coo_matrix((f_data, (f_rows, f_cols)), shape=(2 * N, 2 * N)).tocsr()

    keff, phi, nit = power_iteration(M, F, 2 * N, tol=tol)
    return DiffusionResult(keff, phi[:N], phi[N:], nit)


def _face_D(D_i: float, D_j: float) -> float:
    if D_i <= 0.0 or D_j <= 0.0:
        return 0.0
    return 2.0 * D_i * D_j / (D_i + D_j)


def solve_2d(
    xs_map: np.ndarray,
    materials: dict[int, TwoGroupXS],
    dx: float,
    dy: float,
    bc: str = "zero_flux",
    tol: float = 1e-7,
    max_iter: int = 400,
) -> DiffusionResult:
    """Cell-centered 2D two-group FD. xs_map shape (ny, nx) with material ids."""
    ny, nx = xs_map.shape
    N = nx * ny

    def idx(i: int, j: int) -> int:
        return j * nx + i  # i=x, j=y

    rows: list[int] = []
    cols: list[int] = []
    data: list[float] = []
    f_rows: list[int] = []
    f_cols: list[int] = []
    f_data: list[float] = []

    def add(r: int, c: int, v: float) -> None:
        rows.append(r)
        cols.append(c)
        data.append(v)

    for j in range(ny):
        for i in range(nx):
            mat = materials[int(xs_map[j, i])]
            p = idx(i, j)
            r1 = p
            r2 = N + p
            D1, D2 = float(mat.D[0]), float(mat.D[1])
            Sr1 = mat.Sigma_r1
            Sa2 = float(mat.Sigma_a[1])
            Ss12 = mat.Sigma_s12
            nf1, nf2 = float(mat.nuSigma_f[0]), float(mat.nuSigma_f[1])

            diag1 = Sr1
            diag2 = Sa2

            # neighbors: W E S N
            neighbors = [(-1, 0), (1, 0), (0, -1), (0, 1)]
            for di, dj in neighbors:
                ii, jj = i + di, j + dj
                h = dx if di != 0 else dy
                if 0 <= ii < nx and 0 <= jj < ny:
                    mat_n = materials[int(xs_map[jj, ii])]
                    Dn1 = _face_D(D1, float(mat_n.D[0]))
                    Dn2 = _face_D(D2, float(mat_n.D[1]))
                    a1 = Dn1 / h**2
                    a2 = Dn2 / h**2
                    q = idx(ii, jj)
                    add(r1, q, -a1)
                    add(r2, N + q, -a2)
                    diag1 += a1
                    diag2 += a2
                else:
                    reflective = False
                    if bc == "mixed_reflective_zero":
                        if (di == -1 and i == 0) or (dj == -1 and j == 0):
                            reflective = True
                    if reflective:
                        pass
                    else:
                        # Cell center to boundary = h/2 ⇒ coeff 2D/h^2
                        a1 = 2.0 * D1 / h**2
                        a2 = 2.0 * D2 / h**2
                        diag1 += a1
                        diag2 += a2

            add(r1, r1, diag1)
            add(r2, r2, diag2)
            add(r2, r1, -Ss12)

            if nf1 != 0.0:
                f_rows.append(r1)
                f_cols.append(r1)
                f_data.append(nf1)
            if nf2 != 0.0:
                f_rows.append(r1)
                f_cols.append(r2)
                f_data.append(nf2)

    M = sparse.coo_matrix((data, (rows, cols)), shape=(2 * N, 2 * N)).tocsr()
    F = sparse.coo_matrix((f_data, (f_rows, f_cols)), shape=(2 * N, 2 * N)).tocsr()
    keff, phi, nit = power_iteration(M, F, 2 * N, tol=tol, max_iter=max_iter)
    return DiffusionResult(keff, phi[:N].reshape(ny, nx), phi[N:].reshape(ny, nx), nit)


def twigl_material_map(nx: int, ny: int, Lx: float = 80.0, Ly: float = 80.0) -> np.ndarray:
    """TWIGL map matching OFELIA/TWIGL2D.geo (reflective at x=0,y=0).

    domain1 (id=1): central square [24,56]×[24,56]
    domain2 (id=2): arms [0,24]×[24,56] and [24,56]×[0,24]
    domain3 (id=3): blanket (corner [0,24]² and outer frame)
    """
    xs = np.ones((ny, nx), dtype=int) * 3
    dx, dy = Lx / nx, Ly / ny
    for j in range(ny):
        for i in range(nx):
            x = (i + 0.5) * dx
            y = (j + 0.5) * dy
            in_cx = 24.0 <= x < 56.0
            in_cy = 24.0 <= y < 56.0
            if in_cx and in_cy:
                xs[j, i] = 1
            elif (x < 24.0 and in_cy) or (in_cx and y < 24.0):
                xs[j, i] = 2
            else:
                xs[j, i] = 3
    return xs


def run_twigl(n: int = 40, tol: float = 1e-7) -> DiffusionResult:
    data = load_yaml_benchmark(BENCH_DIR / "twigl.yaml")
    seed = TwoGroupXS(
        D=np.array(data["materials"]["seed"]["D"], float),
        Sigma_a=np.array(data["materials"]["seed"]["Sigma_a"], float),
        nuSigma_f=np.array(data["materials"]["seed"]["nuSigma_f"], float),
        Sigma_s12=float(data["materials"]["seed"]["Sigma_s12"]),
    )
    blanket = TwoGroupXS(
        D=np.array(data["materials"]["blanket"]["D"], float),
        Sigma_a=np.array(data["materials"]["blanket"]["Sigma_a"], float),
        nuSigma_f=np.array(data["materials"]["blanket"]["nuSigma_f"], float),
        Sigma_s12=float(data["materials"]["blanket"]["Sigma_s12"]),
    )
    mats = {1: seed, 2: seed, 3: blanket}
    amap = twigl_material_map(n, n)
    dx = data["Lx_cm"] / n
    return solve_2d(amap, mats, dx, dx, bc="mixed_reflective_zero", tol=tol)


def run_homogeneous_1d_analytic_check(n: int = 80) -> tuple[DiffusionResult, float]:
    """Infinite medium k_inf vs finite slab k_eff (k_eff < k_inf)."""
    xs = TwoGroupXS(
        D=np.array([1.0, 0.5]),
        Sigma_a=np.array([0.02, 0.08]),
        nuSigma_f=np.array([0.0, 0.10]),
        Sigma_s12=0.04,
    )
    # k_inf = νΣf2 / Σa2 * Σs12 / (Σa1+Σs12) for χ=(1,0), νΣf1=0
    k_inf = (xs.nuSigma_f[1] / xs.Sigma_a[1]) * (xs.Sigma_s12 / xs.Sigma_r1)
    res = solve_1d_homogeneous(xs, L=100.0, n=n)
    return res, float(k_inf)

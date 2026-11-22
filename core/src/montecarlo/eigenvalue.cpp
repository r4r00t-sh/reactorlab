#include "reactorlab/montecarlo/eigenvalue.hpp"

#include "reactorlab/montecarlo/rng.hpp"

#include <cmath>
#include <cstdint>
#include <numeric>
#include <stdexcept>

#ifdef _OPENMP
#include <omp.h>
#endif

namespace reactorlab::montecarlo {
namespace {

int sample_scatter_or_absorb(Rng& rng, const MgMaterial& mat, int g) {
  const double St = mat.Sigma_t[static_cast<std::size_t>(g)];
  double u = rng.uniform();
  double cum = 0.0;
  for (int gp = 0; gp < mat.n_groups; ++gp) {
    cum += mat.Sigma_s_at(g, gp) / St;
    if (u < cum) {
      return gp;
    }
  }
  return -1;  // absorbed
}

struct Site {
  double x;
  int group;
};

}  // namespace

McResult infinite_medium_keff(const MgMaterial& mat, int n_histories, std::uint64_t seed) {
  if (n_histories <= 1) {
    throw std::invalid_argument("n_histories must be > 1");
  }
  std::vector<double> scores(static_cast<std::size_t>(n_histories), 0.0);

#ifdef _OPENMP
#pragma omp parallel for schedule(static)
#endif
  for (int h = 0; h < n_histories; ++h) {
    Rng rng = stream_rng(seed, 0, static_cast<std::uint64_t>(h));
    int g = 0;
    double score = 0.0;
    bool alive = true;
    while (alive) {
      score += mat.nuSigma_f[static_cast<std::size_t>(g)] / mat.Sigma_t[static_cast<std::size_t>(g)];
      const int g_new = sample_scatter_or_absorb(rng, mat, g);
      if (g_new < 0) {
        alive = false;
      } else {
        g = g_new;
      }
    }
    scores[static_cast<std::size_t>(h)] = score;
  }

  const double mean =
      std::accumulate(scores.begin(), scores.end(), 0.0) / static_cast<double>(n_histories);
  double var = 0.0;
  for (double s : scores) {
    const double d = s - mean;
    var += d * d;
  }
  var /= static_cast<double>(n_histories - 1);
  McResult out;
  out.keff = mean;
  out.keff_std = std::sqrt(var / static_cast<double>(n_histories));
  out.seed = seed;
  out.n_active = 1;
  out.flux.assign(static_cast<std::size_t>(mat.n_groups), 0.0);
  out.flux_std.assign(static_cast<std::size_t>(mat.n_groups), 0.0);
  return out;
}

McResult power_iteration_slab(const MgMaterial& mat,
                              const SlabGeometry& geom,
                              int n_histories,
                              int n_inactive,
                              int n_active,
                              std::uint64_t seed) {
  if (n_histories < 1 || n_active < 2) {
    throw std::invalid_argument("invalid MC controls");
  }
  Rng master(seed);
  std::vector<Site> bank(static_cast<std::size_t>(n_histories));
  for (int i = 0; i < n_histories; ++i) {
    bank[static_cast<std::size_t>(i)] = Site{master.uniform() * geom.L, 0};
  }

  std::vector<double> cycle_k;
  cycle_k.reserve(static_cast<std::size_t>(n_active));
  std::vector<double> flux_sum(static_cast<std::size_t>(mat.n_groups), 0.0);
  std::vector<double> flux_sumsq(static_cast<std::size_t>(mat.n_groups), 0.0);

  for (int cycle = 0; cycle < n_inactive + n_active; ++cycle) {
    std::vector<Site> new_bank;
    new_bank.reserve(static_cast<std::size_t>(n_histories * 2));
    double fission_neutrons = 0.0;
    std::vector<double> track(static_cast<std::size_t>(mat.n_groups), 0.0);

    // Thread-local banks merged after parallel region
    std::vector<std::vector<Site>> tls_banks;
    std::vector<double> tls_fission;
    std::vector<std::vector<double>> tls_track;
    int nthreads = 1;
#ifdef _OPENMP
    nthreads = omp_get_max_threads();
#endif
    tls_banks.resize(static_cast<std::size_t>(nthreads));
    tls_fission.assign(static_cast<std::size_t>(nthreads), 0.0);
    tls_track.assign(static_cast<std::size_t>(nthreads),
                     std::vector<double>(static_cast<std::size_t>(mat.n_groups), 0.0));

#ifdef _OPENMP
#pragma omp parallel
#endif
    {
      int tid = 0;
#ifdef _OPENMP
      tid = omp_get_thread_num();
#endif
#ifdef _OPENMP
#pragma omp for schedule(static)
#endif
      for (int h = 0; h < n_histories; ++h) {
        Rng rng = stream_rng(seed, static_cast<std::uint64_t>(cycle), static_cast<std::uint64_t>(h));
        const Site& src = bank[static_cast<std::size_t>(h)];
        double x = src.x;
        int g = src.group;
        double mu = 2.0 * rng.uniform() - 1.0;
        bool alive = true;
        while (alive) {
          const double St = mat.Sigma_t[static_cast<std::size_t>(g)];
          const double ell = -std::log(rng.uniform()) / St;
          const double x_new = x + mu * ell;
          if (x_new < 0.0 || x_new > geom.L) {
            const double dist = (mu > 0.0) ? (geom.L - x) / mu : (0.0 - x) / mu;
            tls_track[static_cast<std::size_t>(tid)][static_cast<std::size_t>(g)] +=
                (dist > 0.0 ? dist : 0.0);
            alive = false;
            break;
          }
          tls_track[static_cast<std::size_t>(tid)][static_cast<std::size_t>(g)] += ell;
          x = x_new;
          const double nu_expect = mat.nuSigma_f[static_cast<std::size_t>(g)] / St;
          tls_fission[static_cast<std::size_t>(tid)] += nu_expect;
          int n_f = static_cast<int>(nu_expect);
          if (rng.uniform() < (nu_expect - static_cast<double>(n_f))) {
            ++n_f;
          }
          for (int f = 0; f < n_f; ++f) {
            tls_banks[static_cast<std::size_t>(tid)].push_back(Site{x, 0});
          }
          const int g_new = sample_scatter_or_absorb(rng, mat, g);
          if (g_new < 0) {
            alive = false;
          } else {
            g = g_new;
            mu = 2.0 * rng.uniform() - 1.0;
          }
        }
      }
    }

    for (int t = 0; t < nthreads; ++t) {
      fission_neutrons += tls_fission[static_cast<std::size_t>(t)];
      for (int g = 0; g < mat.n_groups; ++g) {
        track[static_cast<std::size_t>(g)] +=
            tls_track[static_cast<std::size_t>(t)][static_cast<std::size_t>(g)];
      }
      new_bank.insert(new_bank.end(), tls_banks[static_cast<std::size_t>(t)].begin(),
                      tls_banks[static_cast<std::size_t>(t)].end());
    }

    const double k_cycle = fission_neutrons / static_cast<double>(n_histories);
    if (new_bank.empty()) {
      new_bank.push_back(Site{0.5 * geom.L, 0});
    }
    bank.resize(static_cast<std::size_t>(n_histories));
    for (int i = 0; i < n_histories; ++i) {
      const auto j = static_cast<std::size_t>(master.uniform() * static_cast<double>(new_bank.size()));
      bank[static_cast<std::size_t>(i)] = new_bank[j % new_bank.size()];
    }

    if (cycle >= n_inactive) {
      cycle_k.push_back(k_cycle);
      for (int g = 0; g < mat.n_groups; ++g) {
        const double f = track[static_cast<std::size_t>(g)] /
                         (static_cast<double>(n_histories) * geom.L);
        flux_sum[static_cast<std::size_t>(g)] += f;
        flux_sumsq[static_cast<std::size_t>(g)] += f * f;
      }
    }
  }

  McResult out;
  out.seed = seed;
  out.n_active = n_active;
  out.cycle_keff = cycle_k;
  const double mean =
      std::accumulate(cycle_k.begin(), cycle_k.end(), 0.0) / static_cast<double>(cycle_k.size());
  double var = 0.0;
  for (double k : cycle_k) {
    const double d = k - mean;
    var += d * d;
  }
  var /= static_cast<double>(cycle_k.size() - 1);
  out.keff = mean;
  out.keff_std = std::sqrt(var / static_cast<double>(cycle_k.size()));
  out.flux.resize(static_cast<std::size_t>(mat.n_groups));
  out.flux_std.resize(static_cast<std::size_t>(mat.n_groups));
  for (int g = 0; g < mat.n_groups; ++g) {
    const double m = flux_sum[static_cast<std::size_t>(g)] / static_cast<double>(n_active);
    const double m2 = flux_sumsq[static_cast<std::size_t>(g)] / static_cast<double>(n_active);
    out.flux[static_cast<std::size_t>(g)] = m;
    out.flux_std[static_cast<std::size_t>(g)] =
        std::sqrt(std::max(m2 - m * m, 0.0) / static_cast<double>(n_active));
  }
  return out;
}

}  // namespace reactorlab::montecarlo

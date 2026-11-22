#pragma once

#include <cstdint>

namespace reactorlab::montecarlo {

/// Tiny deterministic LCG (Numerical Recipes style) for per-history streams.
class Rng {
public:
  explicit Rng(std::uint64_t seed) : state_(seed ? seed : 1ULL) {}

  double uniform() {
    state_ = state_ * 6364136223846793005ULL + 1ULL;
    return static_cast<double>((state_ >> 11) & 0x1fffffffffffffULL) / static_cast<double>(1ULL << 53);
  }

  std::uint64_t state() const { return state_; }

private:
  std::uint64_t state_;
};

[[nodiscard]] inline Rng stream_rng(std::uint64_t seed, std::uint64_t cycle, std::uint64_t history) {
  // SplitMix-ish combine then LCG
  std::uint64_t z = seed + 0x9e3779b97f4a7c15ULL;
  z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
  z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
  z ^= cycle * 0x2545f4914f6cdd1dULL;
  z ^= history * 0x9e3779b97f4a7c15ULL;
  return Rng(z ^ (z >> 33));
}

}  // namespace reactorlab::montecarlo

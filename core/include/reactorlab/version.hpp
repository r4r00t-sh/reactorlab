#pragma once

#include <string_view>

namespace reactorlab {

/// Semantic version of libreactorlab (compile-time).
inline constexpr int kVersionMajor = 0;
inline constexpr int kVersionMinor = 8;
inline constexpr int kVersionPatch = 0;

/// Human-readable version string, e.g. "0.8.0".
[[nodiscard]] constexpr std::string_view version_string() noexcept {
  return "0.8.0";
}

/// Short product disclaimer (not safety-grade). Full text also in README/About.
[[nodiscard]] constexpr std::string_view educational_disclaimer() noexcept {
  return "ReactorLab is an educational/research tool. It is not licensed, "
         "safety-grade, or suitable for real reactor design or operations.";
}

} // namespace reactorlab

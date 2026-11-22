#pragma once

#include <string>
#include <string_view>

namespace reactorlab {

/// Phase 0 smoke API: returns a greeting that embeds the library version.
/// Units: none (string only).
[[nodiscard]] std::string hello(std::string_view name = "ReactorLab");

} // namespace reactorlab

#include "reactorlab/hello.hpp"

#include "reactorlab/version.hpp"

#include <fmt/format.h>

namespace reactorlab {

std::string hello(std::string_view name) {
  return fmt::format(
      "Hello from {}, version {} — {}", name, version_string(), educational_disclaimer());
}

} // namespace reactorlab

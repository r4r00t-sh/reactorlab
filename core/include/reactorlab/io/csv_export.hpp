#pragma once

#include <stdexcept>
#include <string>
#include <vector>

namespace reactorlab::io {

struct CsvColumn {
  std::string name;   // header (include units in name, e.g. "t[s]")
  std::vector<double> values;
};

/// Write columns as CSV. All columns must have equal length.
void write_csv(const std::string& path,
               const std::vector<CsvColumn>& columns,
               const std::string& comment_header = {});

class IoError : public std::runtime_error {
public:
  using std::runtime_error::runtime_error;
};

}  // namespace reactorlab::io

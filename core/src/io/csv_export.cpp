#include "reactorlab/io/csv_export.hpp"

#include <fstream>
#include <iomanip>

namespace reactorlab::io {

void write_csv(const std::string& path,
               const std::vector<CsvColumn>& columns,
               const std::string& comment_header) {
  if (columns.empty()) {
    throw IoError("write_csv: no columns");
  }
  const std::size_t n = columns.front().values.size();
  for (const auto& c : columns) {
    if (c.values.size() != n) {
      throw IoError("write_csv: column length mismatch");
    }
  }
  std::ofstream out(path);
  if (!out) {
    throw IoError("write_csv: cannot open " + path);
  }
  out << std::setprecision(16);
  if (!comment_header.empty()) {
    out << "# " << comment_header << '\n';
  }
  out << "# ReactorLab educational export — not for design/operations\n";
  for (std::size_t j = 0; j < columns.size(); ++j) {
    if (j) out << ',';
    out << columns[j].name;
  }
  out << '\n';
  for (std::size_t i = 0; i < n; ++i) {
    for (std::size_t j = 0; j < columns.size(); ++j) {
      if (j) out << ',';
      out << columns[j].values[i];
    }
    out << '\n';
  }
}

}  // namespace reactorlab::io

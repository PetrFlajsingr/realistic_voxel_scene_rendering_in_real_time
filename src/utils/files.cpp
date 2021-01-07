//
// Created by petr on 1/7/21.
//

#include "files.h"
#include <pf_common/exceptions/StackTraceException.h>

namespace pf{

std::vector<std::filesystem::path> filesInFolder(const std::filesystem::path &folder) {
  if (!std::filesystem::is_directory(folder)) {
    throw InvalidArgumentException::fmt("Path is not a directory: '{}'", folder.string());
  }
  auto result = std::vector<std::filesystem::path>();
  for (const auto &p : std::filesystem::directory_iterator(folder)) {
    if (std::filesystem::is_regular_file(p.path())) {
      result.emplace_back(p.path());
    }
  }
  return result;
}
}
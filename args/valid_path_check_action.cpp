//
// Created by petr on 9/23/20.
//

#include "valid_path_check_action.h"


std::filesystem::path valid_path_check_action::operator()(std::string_view path_str) {
  auto path = std::filesystem::path(path_str);
  if (!std::filesystem::is_directory(path)) {
    throw std::runtime_error{fmt::format("Provided path: '{}' is not a directory.", path_str)};
  }
  return path;
}

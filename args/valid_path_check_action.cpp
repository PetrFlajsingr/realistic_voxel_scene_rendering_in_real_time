//
// Created by petr on 9/23/20.
//

#include "valid_path_check_action.h"

valid_path_check_action::valid_path_check_action(path_type type) : type(type) {}

std::filesystem::path valid_path_check_action::operator()(std::string_view path_str) const {
  auto path = std::filesystem::path(path_str);
  if (type == path_type::directory && !std::filesystem::is_directory(path)) {
    throw std::runtime_error{fmt::format("Provided path: '{}' is not a directory.", path_str)};
  }
  if (type == path_type::file && !std::filesystem::is_regular_file(path)) {
    throw std::runtime_error{fmt::format("Provided path: '{}' is not a directory.", path_str)};
  }
  return path;
}

//
// Created by petr on 9/23/20.
//

#include "ValidPathCheckAction.h"

ValidPathCheckAction::ValidPathCheckAction(PathType pathType) : type(pathType) {}

std::filesystem::path ValidPathCheckAction::operator()(std::string_view pathStr) const {
  auto path = std::filesystem::path(pathStr);
  if (type == PathType::Directory && !std::filesystem::is_directory(path)) {
    throw std::runtime_error{fmt::format("Provided path: '{}' is not a directory.", pathStr)};
  }
  if (type == PathType::File && !std::filesystem::is_regular_file(path)) {
    throw std::runtime_error{fmt::format("Provided path: '{}' is not a directory.", pathStr)};
  }
  return path;
}

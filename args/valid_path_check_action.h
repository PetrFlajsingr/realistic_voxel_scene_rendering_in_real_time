//
// Created by petr on 9/23/20.
//

#ifndef REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_VALID_PATH_CHECK_ACTION_H
#define REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_VALID_PATH_CHECK_ACTION_H
#include "fmt/format.h"
#include <filesystem>

enum class path_type { file, directory };

struct valid_path_check_action {
  explicit valid_path_check_action(path_type type);
  std::filesystem::path operator()(std::string_view path_str) const;
  const path_type type;
};

#endif//REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_VALID_PATH_CHECK_ACTION_H

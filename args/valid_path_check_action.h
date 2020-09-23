//
// Created by petr on 9/23/20.
//

#ifndef REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_VALID_PATH_CHECK_ACTION_H
#define REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_VALID_PATH_CHECK_ACTION_H
#include <filesystem>
#include "fmt/format.h"

struct valid_path_check_action {
  std::filesystem::path operator()(std::string_view path_str);
};

#endif//REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_VALID_PATH_CHECK_ACTION_H

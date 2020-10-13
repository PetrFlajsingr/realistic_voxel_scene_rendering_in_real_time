//
// Created by petr on 9/23/20.
//

#ifndef REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_VALIDPATHCHECKACTION_H
#define REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_VALIDPATHCHECKACTION_H
#include "fmt/format.h"
#include <filesystem>

enum class PathType { File, Directory };

struct ValidPathCheckAction {
  explicit ValidPathCheckAction(PathType type);
  std::filesystem::path operator()(std::string_view pathStr) const;
  const PathType type;
};

#endif//REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_VALIDPATHCHECKACTION_H

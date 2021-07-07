/**
 * @file ValidPathCheckAction.h
 * @brief An action to test argument value as valid path.
 * @author Petr Flajšingr
 * @date 23.9.20
 */


#ifndef VOXEL_RENDER_VALIDPATHCHECKACTION_H
#define VOXEL_RENDER_VALIDPATHCHECKACTION_H
#include "fmt/format.h"
#include <filesystem>

/**
 * @brief Type of required path type.
 */
enum class PathType { File, Directory };

/**
 * @brief Checks argument to be a valid path to directory/file. For argparse.
 */
struct ValidPathCheckAction {
  explicit ValidPathCheckAction(PathType pathType);
  std::filesystem::path operator()(std::string_view pathStr) const;
  const PathType type;
};

#endif//VOXEL_RENDER_VALIDPATHCHECKACTION_H

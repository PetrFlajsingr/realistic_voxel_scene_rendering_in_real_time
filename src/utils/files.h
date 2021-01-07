//
// Created by petr on 1/7/21.
//

#ifndef REALISTIC_VOXEL_RENDERING_SRC_UTILS_FILES_H
#define REALISTIC_VOXEL_RENDERING_SRC_UTILS_FILES_H

#include <vector>
#include <filesystem>

namespace pf {
std::vector<std::filesystem::path> filesInFolder(const std::filesystem::path &folder);
}

#endif//REALISTIC_VOXEL_RENDERING_SRC_UTILS_FILES_H

//
// Created by petr on 5/24/21.
//

#ifndef REALISTIC_VOXEL_RENDERING_SRC_RUNINFO_H
#define REALISTIC_VOXEL_RENDERING_SRC_RUNINFO_H

#include <filesystem>
#include <string>

namespace pf {
struct RunInfo {
  std::filesystem::path exePath;
  std::vector<std::string> args;
};
}

#endif//REALISTIC_VOXEL_RENDERING_SRC_RUNINFO_H

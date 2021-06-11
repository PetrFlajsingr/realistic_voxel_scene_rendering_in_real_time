//
// Created by petr on 12/7/20.
//

#ifndef REALISTIC_VOXEL_RENDERING_SRC_VOXEL_RAWVOXELMODEL_H
#define REALISTIC_VOXEL_RENDERING_SRC_VOXEL_RAWVOXELMODEL_H

#include <glm/vec4.hpp>
#include <string>
#include <vector>

namespace pf::vox {

struct VoxelInfo {
  VoxelInfo(const glm::vec4 &position, const glm::vec4 &color);
  glm::vec4 position;
  glm::vec4 color;
};

class RawVoxelModel {
 public:
  RawVoxelModel(std::string name, std::vector<VoxelInfo> voxels);

  [[nodiscard]] const std::string &getName() const;
  [[nodiscard]] const std::vector<VoxelInfo> &getVoxels() const;

 private:
  std::string name;
  std::vector<VoxelInfo> voxels;
};

}// namespace pf::vox

#endif//REALISTIC_VOXEL_RENDERING_SRC_VOXEL_RAWVOXELMODEL_H

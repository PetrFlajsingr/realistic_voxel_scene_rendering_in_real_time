/**
 * @file RawVoxelModel.h
 * @brief Structures to store raw voxel models.
 * @author Petr Flajšingr
 * @date 7.12.20
 */

#ifndef REALISTIC_VOXEL_RENDERING_SRC_VOXEL_RAWVOXELMODEL_H
#define REALISTIC_VOXEL_RENDERING_SRC_VOXEL_RAWVOXELMODEL_H

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <string>
#include <vector>

namespace pf::vox {

/**
 * @brief Inefficient storage of voxel's position and its material.
 */
struct VoxelInfo {
  VoxelInfo(const glm::vec4 &position, std::uint32_t matId);
  glm::vec4 position;
  std::uint32_t materialId;
};
/**
 * @brief Model storing its voxel data and size.
 */
class RawVoxelModel {
 public:
  RawVoxelModel(std::string name, std::vector<VoxelInfo> voxels, glm::ivec3 size);

  [[nodiscard]] const std::string &getName() const;
  [[nodiscard]] const std::vector<VoxelInfo> &getVoxels() const;

 private:
  std::string name;
  std::vector<VoxelInfo> voxels;
  glm::ivec3 modelSize;
};

}// namespace pf::vox

#endif//REALISTIC_VOXEL_RENDERING_SRC_VOXEL_RAWVOXELMODEL_H

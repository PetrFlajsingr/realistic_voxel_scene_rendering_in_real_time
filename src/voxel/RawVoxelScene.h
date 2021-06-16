//
// Created by petr on 12/7/20.
//

#ifndef REALISTIC_VOXEL_RENDERING_SRC_VOXEL_RAWVOXELSCENE_H
#define REALISTIC_VOXEL_RENDERING_SRC_VOXEL_RAWVOXELSCENE_H

#include "RawVoxelModel.h"
#include <glm/vec3.hpp>
#include <memory>
#include <string>
#include <vector>

namespace pf::vox {
class RawVoxelScene {
 public:
  RawVoxelScene(std::string name, std::vector<std::unique_ptr<RawVoxelModel>> models, glm::vec3 center);

  [[nodiscard]] const std::string &getName() const;
  [[nodiscard]] const std::vector<std::unique_ptr<RawVoxelModel>> &getModels() const;
  const glm::vec3 &getSceneCenter() const;

  [[nodiscard]] RawVoxelModel &getModelByName(std::string_view modelName);

 private:
  std::string name;
  std::vector<std::unique_ptr<RawVoxelModel>> models;
  glm::vec3 sceneCenter{};
};
}// namespace pf::vox

#endif//REALISTIC_VOXEL_RENDERING_SRC_VOXEL_RAWVOXELSCENE_H

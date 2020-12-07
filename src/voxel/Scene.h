//
// Created by petr on 12/7/20.
//

#ifndef REALISTIC_VOXEL_RENDERING_SRC_VOXEL_SCENE_H
#define REALISTIC_VOXEL_RENDERING_SRC_VOXEL_SCENE_H

#include "Model.h"
#include <memory>
#include <string>
#include <vector>

namespace pf::vox {
class Scene {
 public:
  Scene(std::string name, std::vector<std::unique_ptr<Model>> models);

  [[nodiscard]] const std::string &getName() const;
  [[nodiscard]] const std::vector<std::unique_ptr<Model>> &getModels() const;

  [[nodiscard]] Model &getModelByName(std::string_view modelName);

 private:
  std::string name;
  std::vector<std::unique_ptr<Model>> models;
};
}// namespace pf::vox

#endif//REALISTIC_VOXEL_RENDERING_SRC_VOXEL_SCENE_H

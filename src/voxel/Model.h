//
// Created by petr on 12/7/20.
//

#ifndef REALISTIC_VOXEL_RENDERING_SRC_VOXEL_MODEL_H
#define REALISTIC_VOXEL_RENDERING_SRC_VOXEL_MODEL_H

#include <glm/vec4.hpp>
#include <string>
#include <vector>

namespace pf::vox {

struct Voxel {
  Voxel(const glm::vec4 &position, const glm::vec4 &color);
  glm::vec4 position;
  glm::vec4 color;
};

class Model {
 public:
  Model(std::string name, std::vector<Voxel> voxels);

  [[nodiscard]] const std::string &getName() const;
  [[nodiscard]] const std::vector<Voxel> &getVoxels() const;

 private:
  std::string name;
  std::vector<Voxel> voxels;
};

}// namespace pf::vox

#endif//REALISTIC_VOXEL_RENDERING_SRC_VOXEL_MODEL_H

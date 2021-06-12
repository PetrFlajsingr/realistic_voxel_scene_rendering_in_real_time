//
// Created by petr on 6/12/21.
//

#ifndef REALISTIC_VOXEL_RENDERING_SRC_RENDERING_LIGHTFIELDPROBEGENERATOR_H
#define REALISTIC_VOXEL_RENDERING_SRC_RENDERING_LIGHTFIELDPROBEGENERATOR_H

#include <fmt/format.h>
#include <glm/vec3.hpp>
#include <ostream>
#include <voxel/GPUModelManager.h>

namespace pf::lfp {

struct LightFieldProbe {
  glm::vec3 position;

  inline friend std::ostream &operator<<(std::ostream &os, const LightFieldProbe &probe) {
    os << "position: " << fmt::format("[{}, {}, {}]", probe.position.x, probe.position.y, probe.position.z);
    return os;
  }
};

class ProbeGenerator {
 public:
  virtual std::vector<LightFieldProbe> generateLightFieldProbes(vox::GPUModelManager &modelManager) = 0;
  virtual ~ProbeGenerator() = default;
};

}// namespace pf::lfp
#endif//REALISTIC_VOXEL_RENDERING_SRC_RENDERING_LIGHTFIELDPROBEGENERATOR_H

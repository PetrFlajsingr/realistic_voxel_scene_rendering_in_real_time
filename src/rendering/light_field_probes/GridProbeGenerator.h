//
// Created by petr on 6/12/21.
//

#ifndef REALISTIC_VOXEL_RENDERING_SRC_RENDERING_LIGHT_FIELD_PROBES_GRIDPROBEGENERATOR_H
#define REALISTIC_VOXEL_RENDERING_SRC_RENDERING_LIGHT_FIELD_PROBES_GRIDPROBEGENERATOR_H

#include "ProbeGenerator.h"

namespace pf::lfp {
class GridProbeGenerator : public ProbeGenerator {
 public:
  explicit GridProbeGenerator(float step);
  [[nodiscard]] std::vector<LightFieldProbe> generateLightFieldProbes(vox::GPUModelManager &modelManager) override;

  [[nodiscard]] float getGridStep() const;
  void setGridStep(float gridStep);

 private:
  float gridStep;
};

}// namespace pf::lfp
#endif//REALISTIC_VOXEL_RENDERING_SRC_RENDERING_LIGHT_FIELD_PROBES_GRIDPROBEGENERATOR_H

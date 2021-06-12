//
// Created by petr on 6/12/21.
//

#include "GridProbeGenerator.h"
#include <ranges>

namespace pf::lfp {

GridProbeGenerator::GridProbeGenerator(float step) : gridStep(step) {}

std::vector<LightFieldProbe> GridProbeGenerator::generateLightFieldProbes(vox::GPUModelManager &modelManager) {
  const auto &bvh = modelManager.getBvh();
  if (!bvh.data.hasRoot()) { return {}; }
  const auto sceneBB = modelManager.getBvh().data.getRoot()->aabb;
  auto result = std::vector<LightFieldProbe>{};
  const auto gridWidth = static_cast<int>(sceneBB.width() / gridStep) + 1;
  const auto gridHeight = static_cast<int>(sceneBB.height() / gridStep) + 1;
  const auto gridDepth = static_cast<int>(sceneBB.depth() / gridStep) + 1;
  const auto nodeCount = gridWidth * gridHeight * gridDepth;
  return std::views::iota(0, nodeCount) | std::views::transform([&, this](auto index) {
           const auto posInGrid = glm::vec3{index % gridWidth * gridStep, index / gridWidth % gridHeight * gridStep,
                                   index / (gridWidth * gridHeight) * gridStep};
           return LightFieldProbe{posInGrid + sceneBB.p1};
         })
      | ranges::to_vector;
}
float GridProbeGenerator::getGridStep() const { return gridStep; }

void GridProbeGenerator::setGridStep(float step) { gridStep = step; }

}// namespace pf::lfp

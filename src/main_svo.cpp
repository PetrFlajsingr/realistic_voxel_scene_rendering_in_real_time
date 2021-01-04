//
// Created by petr on 1/2/21.
//
#include <voxel/SparseVoxelOctreeCreation.h>
#include <utils/interface/Serializable.h>

int main() {
  auto tree =
      pf::vox::loadFileAsSVO("/home/petr/CLionProjects/realistic_voxel_scene_rendering_in_real_time/models/8x8x8.vox");

  const auto data = tree.serialize();
  pf::saveToFile("/home/petr/Desktop/test.pf_vox", tree);
  auto tree2 = pf::loadFromFile<pf::vox::SparseVoxelOctree>("/home/petr/Desktop/test.pf_vox");
  const auto data2 = tree2.serialize();
  for (auto [B1, B2] : ranges::views::zip(data, data2)) {
    assert(B1 == B2);
  }
  return 0;
}
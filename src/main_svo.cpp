//
// Created by petr on 1/2/21.
//
#include <pf_common/concepts/Serializable.h>
#include <range/v3/view/transform.hpp>
#include <voxel/SparseVoxelOctreeCreation.h>


int main(int argc, char **argv) {
  assert(argc > 1);
  auto tree = pf::vox::loadFileAsSVO(
      fmt::format("/home/petr/CLionProjects/realistic_voxel_scene_rendering_in_real_time/models/{}.vox", argv[1]));

  //  fmt::print(tree.getBlocks()[0].pages[0].toString());

  for (const auto &[idx, desc] : ranges::views::enumerate(tree.getBlocks()[0].pages[0].childDescriptors)) {
    fmt::print("#{:3}: {}", idx, desc.toString() + '\n');
    fmt::print(desc.stringDraw());
    fmt::print("_______\n");
  }

  auto chData = tree.getBlocks()[0].pages[0].childDescriptors[0].childData;
  [[maybe_unused]] auto test = *reinterpret_cast<uint *>(&chData);

  const auto data = tree.serialize();
  pf::saveToFile("/home/petr/Desktop/test.pf_vox", tree);
  auto tree2 = pf::loadFromFile<pf::vox::SparseVoxelOctree>("/home/petr/Desktop/test.pf_vox");
  const auto data2 = tree2.serialize();
  for (auto [B1, B2] : ranges::views::zip(data, data2)) { assert(B1 == B2); }
  return 0;
}
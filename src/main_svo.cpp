//
// Created by petr on 1/2/21.
//
#include "logging/loggers.h"
#include <pf_common/concepts/Serializable.h>
#include <pf_common/files.h>
#include <range/v3/view/join.hpp>
#include <range/v3/view/transform.hpp>
#include <voxel/SparseVoxelOctreeCreation.h>
#define ANKERL_NANOBENCH_IMPLEMENT
#include <nanobench.h>

using namespace pf;
using namespace ranges;
using namespace std::string_literals;

int main([[maybe_unused]] int argc, char **argv) {
  assert(argc > 1);
  const auto loggerSettings =
      GlobalLoggerSettings{.verbose = true, .console = true, .debug = true, .logDir = std::filesystem::current_path()};
  pf::initGlobalLogger(loggerSettings);

  const auto files = filesInFolder("/home/petr/Desktop/test_voxels") | actions::sort;

  auto bench = ankerl::nanobench::Bench();
  bench.title("Voxel loading").warmup(0).relative(true);
  bench.performanceCounters(true);

  std::ranges::for_each(files, [&](const auto &file) {
    const auto scene = vox::loadScene(file);
    const auto voxelCnt =
        (scene.getModels() | views::transform([](const auto &model) { return model->getVoxels() | views::all; })
         | views::join | to_vector)
            .size();
    bench.run("Voxels: "s + std::to_string(voxelCnt) + " file: " + file.string(),
              [&] { ankerl::nanobench::doNotOptimizeAway(vox::convertSceneToSVO(scene)); });
  });

  return 0;

  auto tree = pf::vox::loadFileAsSVO(fmt::format("/home/petr/Desktop/magica_voxel/vox/{}.vox", argv[1]));

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
  for (auto [B1, B2] : ranges::views::zip(data, data2)) {
    if (B1 != B2) { throw "hihihi"; };
  }
  return 0;
}
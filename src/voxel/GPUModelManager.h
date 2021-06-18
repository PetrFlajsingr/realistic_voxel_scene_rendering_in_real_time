//
// Created by petr on 6/8/21.
//

#ifndef REALISTIC_VOXEL_RENDERING_SRC_VOXEL_GPUMODELMANAGER_H
#define REALISTIC_VOXEL_RENDERING_SRC_VOXEL_GPUMODELMANAGER_H

#include "AABB_BVH.h"
#include "GPUModelInfo.h"
#include "RawVoxelModel.h"
#include "RawVoxelScene.h"
#include <memory>
#include <pf_glfw_vulkan/vulkan/types/BufferMemoryPool.h>
#include <range/v3/view/addressof.hpp>
#include <tl/expected.hpp>
#include <vector>

namespace pf::vox {

class GPUModelManager {
 public:
  GPUModelManager(std::shared_ptr<vulkan::BufferMemoryPool> svoMemoryPool,
                  std::shared_ptr<vulkan::BufferMemoryPool> modelInfoMemoryPool, std::size_t defaultSvoHeightSize);
  using ModelPtr = std::experimental::observer_ptr<GPUModelInfo>;

  struct Callbacks {
    Callbacks(std::invocable<float> auto &&onProgress) : progress(onProgress) {}
    std::function<void(float)> progress;
  };

  tl::expected<std::vector<ModelPtr>, std::string>
  loadModel(const std::filesystem::path &path, const Callbacks &callbacks, bool sceneAsOneSVO, bool autoScale = false);

  // TODO: fix this interface
  tl::expected<std::vector<ModelPtr>, std::string> loadModel(RawVoxelScene &scene, bool autoScale = true);
  tl::expected<ModelPtr, std::string> loadModel(RawVoxelModel &model, bool autoScale = true);
  tl::expected<ModelPtr, std::string> createModelInstance(ModelPtr model);
  tl::expected<ModelPtr, std::string> duplicateModel(ModelPtr model);

  [[nodiscard]] const BVHCreateInfo &rebuildBVH(bool createStats);
  [[nodiscard]] const BVHCreateInfo &getBvh() const;

  void removeModel(ModelPtr toRemove);

  [[nodiscard]] auto getModels() const {
    return models | std::views::transform([](auto &model) -> GPUModelInfo & { return *model; });
  }

 private:
  tl::expected<std::unique_ptr<GPUModelInfo>, std::string> prepareDuplicate(ModelPtr original);
  std::size_t defaultSVOHeightSize = 5;
  std::vector<std::unique_ptr<GPUModelInfo>> models{};
  std::shared_ptr<vulkan::BufferMemoryPool> svoMemoryPool;
  std::shared_ptr<vulkan::BufferMemoryPool> modelInfoMemoryPool;
  std::mutex mutex;
  BVHCreateInfo bvh;
};
}// namespace pf::vox
#endif//REALISTIC_VOXEL_RENDERING_SRC_VOXEL_GPUMODELMANAGER_H

//
// Created by petr on 6/8/21.
//

#ifndef REALISTIC_VOXEL_RENDERING_SRC_VOXEL_GPUMODELMANAGER_H
#define REALISTIC_VOXEL_RENDERING_SRC_VOXEL_GPUMODELMANAGER_H

#include "AABB_BVH.h"
#include "GPUModelInfo.h"
#include <memory>
#include <pf_glfw_vulkan/vulkan/types/BufferMemoryPool.h>
#include <range/v3/view/addressof.hpp>
#include <tl/expected.hpp>
#include <vector>

namespace pf::vox {

class GPUModelManager {
 public:
  GPUModelManager(const std::shared_ptr<vulkan::BufferMemoryPool> &svoMemoryPool,
                  const std::shared_ptr<vulkan::BufferMemoryPool> &modelInfoMemoryPool,
                  std::size_t defaultSvoHeightSize);
  using ModelPtr = std::experimental::observer_ptr<GPUModelInfo>;

  struct Callbacks {
    Callbacks(std::invocable<float> auto &&onProgress) : progress(onProgress) {}
    std::function<void(float)> progress;
  };

  tl::expected<ModelPtr, std::string> loadModel(const std::filesystem::path &path, const Callbacks &callbacks,
                                                bool autoScale = false);
  tl::expected<ModelPtr, std::string> createModelInstance(ModelPtr model);
  tl::expected<ModelPtr, std::string> duplicateModel(ModelPtr model);

  [[nodiscard]] BVHCreateInfo buildBVH(bool createStats);

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
};
}// namespace pf::vox
#endif//REALISTIC_VOXEL_RENDERING_SRC_VOXEL_GPUMODELMANAGER_H

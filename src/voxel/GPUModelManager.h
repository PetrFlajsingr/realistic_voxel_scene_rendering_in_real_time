/**
 * @file GPUModelManager.h
 * @brief Management for models and gpu memory used for them.
 * @author Petr Flajšingr
 * @date 8.6.21
 */

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
/**
 * @brief A manager for models and gpu memory used by them.
 */
class GPUModelManager {
 public:
  /**
   * Construct GPUModelManager.
   * @param svoMemoryPool a memory pool for SVO storage
   * @param modelInfoMemoryPool a memory pool for model info storage
   * @param materialMemoryPool a memory pool for material storage
   * @param defaultSvoHeightSize default SVO height from which voxel size will be calculated upon new model load
   */
  GPUModelManager(std::shared_ptr<vulkan::BufferMemoryPool> svoMemoryPool,
                  std::shared_ptr<vulkan::BufferMemoryPool> modelInfoMemoryPool,
                  std::shared_ptr<vulkan::BufferMemoryPool> materialMemoryPool, std::size_t defaultSvoHeightSize);
  using ModelPtr = std::experimental::observer_ptr<GPUModelInfo>;

  struct Callbacks {
    explicit(false) Callbacks(std::invocable<float> auto &&onProgress) : progress(onProgress) {}
    std::function<void(float)> progress;
  };

  /**
   * Load a model from given path.
   * @param path source file
   * @param callbacks progress callbacks
   * @param sceneAsOneSVO if false all models inside the scene will me loaded as separate entities
   * @param autoScale autoscale to size provided in the cosntructor `defaultSvoHeightSize`
   * @return an error string if loading fails, otherwise vector of loaded models
   */
  tl::expected<std::vector<ModelPtr>, std::string>
  loadModel(const std::filesystem::path &path, const Callbacks &callbacks, bool sceneAsOneSVO, bool autoScale = false);

  /**
   * Load a model for raw scene data.
   * @param scene raw scene data
   * @param autoScale autoscale to size provided in the cosntructor `defaultSvoHeightSize`
   * @return an error string if loading fails, otherwise vector of loaded models
   */
  tl::expected<std::vector<ModelPtr>, std::string> loadModel(RawVoxelScene &scene, bool autoScale = true);
  /**
   * Load a model from raw model data along with material info.
   * @param model raw model data
   * @param materials materials used in the model
   * @param autoScale autoscale to size provided in the cosntructor `defaultSvoHeightSize`
   * @return an error string if loading fails, otherwise the loaded model
   */
  tl::expected<ModelPtr, std::string> loadModel(RawVoxelModel &model, const std::vector<MaterialProperties> &materials,
                                                bool autoScale = true);
  /**
   * Create an instance from already loaded model.
   * @param model model to create an instance off
   * @return an error string if loading fails, otherwise the new model
   */
  tl::expected<ModelPtr, std::string> createModelInstance(ModelPtr model);
  /**
   * Create a duplicate from already loaded model.
   * @param model model to create an instance off
   * @return an error string if loading fails, otherwise the new model
   */
  tl::expected<ModelPtr, std::string> duplicateModel(ModelPtr model);

  /**
   * Rebuild bounding volume hierarchies for models managed by this manager.
   * @param createStats if true extra stats will be created
   * @return BVH for models managed by this object
   */
  [[nodiscard]] const BVHCreateInfo &rebuildBVH(bool createStats);
  /**
   * Get bounding volume hierarchy currently stored inside the manager.
   * @return BVH for models managed by this object
   */
  [[nodiscard]] const BVHCreateInfo &getBvh() const;
  /**
   * Remove a model, freeing its memory.
   * @param toRemove model to remove
   */
  void removeModel(ModelPtr toRemove);
  /**
   * Get a range of references to all models managed by this object.
   * @return range of references to models
   */
  [[nodiscard]] auto getModels() const {
    return models | std::views::transform([](auto &model) -> GPUModelInfo & { return *model; });
  }

 private:
  tl::expected<std::unique_ptr<GPUModelInfo>, std::string> prepareDuplicate(ModelPtr original);
  std::size_t defaultSVOHeightSize = 5;
  std::vector<std::unique_ptr<GPUModelInfo>> models{};
  std::shared_ptr<vulkan::BufferMemoryPool> svoMemoryPool;
  std::shared_ptr<vulkan::BufferMemoryPool> modelInfoMemoryPool;
  std::shared_ptr<vulkan::BufferMemoryPool> materialsMemoryPool;
  std::mutex mutex;
  BVHCreateInfo bvh;
};
}// namespace pf::vox
#endif//REALISTIC_VOXEL_RENDERING_SRC_VOXEL_GPUMODELMANAGER_H

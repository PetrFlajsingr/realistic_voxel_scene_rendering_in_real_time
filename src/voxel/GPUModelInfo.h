/**
 * @file GPUModelInfo.h
 * @brief Information about a model stored on the GPU.
 * @author Petr Flajšingr
 * @date 3.6.21
 */

#ifndef REALISTIC_VOXEL_RENDERING_SRC_VOXEL_GPUMODELINFO_H
#define REALISTIC_VOXEL_RENDERING_SRC_VOXEL_GPUMODELINFO_H

#include <filesystem>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <memory>
#include <optional>
#include <pf_common/coroutines/Sequence.h>
#include <pf_common/math/BoundingBox.h>
#include <pf_glfw_vulkan/vulkan/types/BufferMemoryPool.h>
#include <string>
#include <toml++/toml.h>
#include "Materials.h"

namespace pf::vox {

constexpr static auto MODEL_INFO_BLOCK_SIZE = sizeof(glm::mat4) * 2 + sizeof(glm::vec4) * 3 + sizeof(glm::ivec4);
constexpr static auto ONE_MATERIAL_SIZE = sizeof(std::uint32_t) + 14 * sizeof(float);

/**
 * @brief Struct for information of a model stored in the gpu.
 */
struct GPUModelInfo {
  std::filesystem::path path; /**< Path to file from which the model was loaded */
  std::uint32_t svoHeight{0}; /**< Height of the SVO */
  std::uint32_t voxelCount{0}; /**< Total loaded voxel count */
  std::uint32_t minimizedVoxelCount{0}; /**< Total voxel count after minimisation */
  glm::vec3 translateVec{0, 0, 0}; /**< Translation vector */
  glm::vec3 scaleVec{1, 1, 1}; /**< Scale vector */
  glm::vec3 rotateVec{0, 0, 0}; /**< Rotation vector */
  glm::vec3 center{}; /**< Center of the model */
  math::BoundingBox<3> AABB{}; /**< Axis aligned bounding box of the model */
  std::vector<MaterialProperties> materials; /**< All materials used in the model */
  std::shared_ptr<vulkan::BufferMemoryPool::Block> svoMemoryBlock = nullptr; /**< Block of gpu memory storing svo data */
  std::shared_ptr<vulkan::BufferMemoryPool::Block> modelInfoMemoryBlock = nullptr; /**< Block of gpu memory storing model info */
  std::shared_ptr<vulkan::BufferMemoryPool::Block> materialsMemoryBlock = nullptr; /**< Block of gpu memory storing materials */
  std::uint32_t id = getNext(IdGenerator); /**< A unique id of the model */
  glm::mat4 transformMatrix{}; /**< Model's transform matrix */

  bool operator==(const GPUModelInfo &rhs) const;
  bool operator!=(const GPUModelInfo &rhs) const;
  friend std::ostream &operator<<(std::ostream &os, const GPUModelInfo &info);
  inline static auto IdGenerator = iota<std::uint32_t>(); /**< Generator for unique model IDs */
  /**
   * Index of model within model info buffer.
   * @return index in model info buffer
   */
  [[nodiscard]] std::optional<std::uint32_t> getModelIndex() const;
  /**
   * Convert the model to it's toml representation.
   * @return model info as toml
   */
  [[nodiscard]] toml::table toToml() const;
  /**
   * Load model info from toml.
   * @param src source toml data
   */
  void fromToml(const toml::table &src);
  /**
   * Update transform information and upload all necessary changes to gpu memory.
   */
  void updateInfoToGPU();
  /**
   * Generate a new ID for this model. Should be used when creating a new instance or a duplicate.
   */
  void assignNewId();
};

}// namespace pf::vox
#endif//REALISTIC_VOXEL_RENDERING_SRC_VOXEL_GPUMODELINFO_H

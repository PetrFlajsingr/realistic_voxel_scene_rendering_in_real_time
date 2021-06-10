//
// Created by petr on 6/3/21.
//

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

namespace pf::vox {

constexpr static auto MODEL_INFO_BLOCK_SIZE = sizeof(glm::mat4) * 2 + sizeof(glm::vec4) * 3;

struct GPUModelInfo {
  std::filesystem::path path;
  std::uint32_t svoHeight{0};
  std::uint32_t voxelCount{0};
  std::uint32_t minimizedVoxelCount{0};
  glm::vec3 translateVec{0, 0, 0};
  glm::vec3 scaleVec{1, 1, 1};
  glm::vec3 rotateVec{0, 0, 0};
  math::BoundingBox<3> AABB{};
  std::shared_ptr<vulkan::BufferMemoryPool::Block> svoMemoryBlock = nullptr;// TODO change this into unique_ptr
  std::shared_ptr<vulkan::BufferMemoryPool::Block> modelInfoMemoryBlock = nullptr;
  std::uint32_t id = getNext(IdGenerator);
  glm::mat4 transformMatrix{};

  bool operator==(const GPUModelInfo &rhs) const;
  bool operator!=(const GPUModelInfo &rhs) const;
  friend std::ostream &operator<<(std::ostream &os, const GPUModelInfo &info);
  inline static auto IdGenerator = iota<std::uint32_t>();

  [[nodiscard]] std::optional<std::uint32_t> getModelIndex() const;

  [[nodiscard]] toml::table toToml() const;
  void fromToml(const toml::table &src);

  void updateInfoToGPU();
  void assignNewId();
};

}// namespace pf::vox
#endif//REALISTIC_VOXEL_RENDERING_SRC_VOXEL_GPUMODELINFO_H

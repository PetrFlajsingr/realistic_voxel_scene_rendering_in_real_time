//
// Created by petr on 6/19/21.
//

#ifndef REALISTIC_VOXEL_RENDERING_SRC_RENDERING_LIGHT_FIELD_PROBES_PROBEMANAGER_H
#define REALISTIC_VOXEL_RENDERING_SRC_RENDERING_LIGHT_FIELD_PROBES_PROBEMANAGER_H

#include <cppcoro/generator.hpp>
#include <glm/glm.hpp>
#include <pf_glfw_vulkan/vulkan/types/Image.h>
#include <pf_glfw_vulkan/vulkan/types/ImageView.h>
#include <pf_glfw_vulkan/vulkan/types/LogicalDevice.h>
#include <ranges>

namespace pf::lfp {
// probe count should be power of two

struct ProbeCount {
  explicit(false) ProbeCount(glm::ivec3 count);
  explicit(false) operator glm::ivec3() const;
  glm::ivec3 value;
};

/**
 * Probes are generated in +x, +y and +z directions
 */
class ProbeManager {
 public:
  constexpr static glm::ivec2 TEXTURE_SIZE{1024, 1024};

  ProbeManager(ProbeCount probeCount, const glm::vec3 &gridStart, float gridStep,
               const std::shared_ptr<vulkan::LogicalDevice> &logicalDevice);

  [[nodiscard]] uint32_t getTotalProbeCount() const;
  [[nodiscard]] const glm::ivec3 &getProbeCount() const;
  [[nodiscard]] const glm::vec3 &getGridStart() const;
  [[nodiscard]] float getGridStep() const;
  [[nodiscard]] const std::shared_ptr<vulkan::Image> &getProbesImage() const;
  [[nodiscard]] const std::shared_ptr<vulkan::ImageView> &getProbesImageView() const;

  [[nodiscard]] cppcoro::generator<glm::vec3> getProbePositions() const {
    for (int x = 0; x < probeCount.x; ++x) {
      for (int y = 0; y < probeCount.x; ++y) {
        for (int z = 0; z < probeCount.x; ++z) {
          co_yield glm::vec3{x * gridStep, y * gridStep, z * gridStep} + gridStart;
        }
      }
    }
  }

 private:
  glm::ivec3 probeCount;
  glm::vec3 gridStart;
  float gridStep;

  std::shared_ptr<vulkan::Image> probesImage;
  std::shared_ptr<vulkan::ImageView> probesImageView;
};
}// namespace pf::lfp
#endif//REALISTIC_VOXEL_RENDERING_SRC_RENDERING_LIGHT_FIELD_PROBES_PROBEMANAGER_H
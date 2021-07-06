//
// Created by petr on 6/19/21.
//

#include "ProbeManager.h"

namespace pf::lfp {
ProbeCount::ProbeCount(glm::ivec3 count) : value(count) {
  assert((count.x & (count.x - 1)) == 0 && "count has to be a power of 2");
  assert((count.y & (count.y - 1)) == 0 && "count has to be a power of 2");
  assert((count.z & (count.z - 1)) == 0 && "count has to be a power of 2");
}
ProbeManager::ProbeManager(ProbeCount probeCount, const glm::vec3 &gridStart, float gridStep, glm::ivec3 proxGridSize,
                           const std::shared_ptr<vulkan::LogicalDevice> &logicalDevice)
    : probeCount(probeCount), gridStart(gridStart), gridStep(gridStep), proximityGridSize(proxGridSize) {
  const auto totalGridSize = glm::vec3{ProbeManager::probeCount} * gridStep;
  proximityGridStep = totalGridSize / glm::vec3{proxGridSize};
  probesImage =
      logicalDevice->createImage({.imageType = vk::ImageType::e2D,
                                  .format = vk::Format::eR32G32Sfloat,
                                  .extent = vk::Extent3D{.width = TEXTURE_SIZE.x, .height = TEXTURE_SIZE.y, .depth = 1},
                                  .mipLevels = 1,
                                  .arrayLayers = getTotalProbeCount(),
                                  .sampleCount = vk::SampleCountFlagBits::e1,
                                  .tiling = vk::ImageTiling::eOptimal,
                                  .usage = vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eTransferSrc
                                      | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
                                  .sharingQueues = {},
                                  .layout = vk::ImageLayout::eUndefined});
  probesImageView = probesImage->createImageView(
      vk::ColorSpaceKHR::eSrgbNonlinear, vk::ImageViewType::e2DArray,
      vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, getTotalProbeCount()});
  probesImageSmall = logicalDevice->createImage(
      {.imageType = vk::ImageType::e2D,
       .format = vk::Format::eR32Sfloat,
       .extent = vk::Extent3D{.width = TEXTURE_SIZE_SMALL.x, .height = TEXTURE_SIZE_SMALL.y, .depth = 1},
       .mipLevels = 1,
       .arrayLayers = getTotalProbeCount(),
       .sampleCount = vk::SampleCountFlagBits::e1,
       .tiling = vk::ImageTiling::eOptimal,
       .usage = vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eTransferSrc
           | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
       .sharingQueues = {},
       .layout = vk::ImageLayout::eUndefined});
  probesImageViewSmall = probesImageSmall->createImageView(
      vk::ColorSpaceKHR::eSrgbNonlinear, vk::ImageViewType::e2DArray,
      vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, getTotalProbeCount()});
  probesImageSmallest = logicalDevice->createImage(
      {.imageType = vk::ImageType::e2D,
       .format = vk::Format::eR32Sfloat,
       .extent = vk::Extent3D{.width = TEXTURE_SIZE_SMALL.x, .height = TEXTURE_SIZE_SMALLEST.y, .depth = 1},
       .mipLevels = 1,
       .arrayLayers = getTotalProbeCount(),
       .sampleCount = vk::SampleCountFlagBits::e1,
       .tiling = vk::ImageTiling::eOptimal,
       .usage = vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eTransferSrc
           | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
       .sharingQueues = {},
       .layout = vk::ImageLayout::eUndefined});
  probesImageViewSmallest = probesImageSmallest->createImageView(
      vk::ColorSpaceKHR::eSrgbNonlinear, vk::ImageViewType::e2DArray,
      vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, getTotalProbeCount()});
}

uint32_t ProbeManager::getTotalProbeCount() const { return probeCount.x * probeCount.y * probeCount.z; }

const glm::ivec3 &ProbeManager::getProbeCount() const { return probeCount; }

const glm::vec3 &ProbeManager::getGridStart() const { return gridStart; }

float ProbeManager::getGridStep() const { return gridStep; }

const std::shared_ptr<vulkan::Image> &ProbeManager::getProbesImage() const { return probesImage; }

const std::shared_ptr<vulkan::ImageView> &ProbeManager::getProbesImageView() const { return probesImageView; }
const std::shared_ptr<vulkan::Image> &ProbeManager::getProbesImageSmall() const { return probesImageSmall; }
const std::shared_ptr<vulkan::ImageView> &ProbeManager::getProbesImageViewSmall() const { return probesImageViewSmall; }
cppcoro::generator<glm::vec3> ProbeManager::getProbePositions() const {
  for (int z = 0; z < probeCount.z; ++z) {
    for (int y = 0; y < probeCount.y; ++y) {
      for (int x = 0; x < probeCount.x; ++x) {
        co_yield glm::vec3{x * gridStep, y * gridStep, z * gridStep} + gridStart;
      }
    }
  }
}
const glm::ivec3 &ProbeManager::getProximityGridSize() const { return proximityGridSize; }
const glm::vec3 &ProbeManager::getProximityGridStep() const { return proximityGridStep; }
const std::shared_ptr<vulkan::Image> &ProbeManager::getProbesImageSmallest() const { return probesImageSmallest; }
const std::shared_ptr<vulkan::ImageView> &ProbeManager::getProbesImageViewSmallest() const {
  return probesImageViewSmallest;
}
void ProbeManager::setGridStart(const glm::vec3 &gridStart) { ProbeManager::gridStart = gridStart; }
void ProbeManager::setGridStep(float gridStep) { ProbeManager::gridStep = gridStep; }
void ProbeManager::setProximityGridSize(const glm::ivec3 &proximityGridSize) {
  ProbeManager::proximityGridSize = proximityGridSize;
}

ProbeCount::operator glm::ivec3() const { return value; }
}// namespace pf::lfp
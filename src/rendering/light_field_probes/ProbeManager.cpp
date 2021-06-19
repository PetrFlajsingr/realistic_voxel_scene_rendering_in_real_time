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
ProbeManager::ProbeManager(ProbeCount probeCount, const glm::vec3 &gridStart, float gridStep,
                           const std::shared_ptr<vulkan::LogicalDevice> &logicalDevice)
    : probeCount(probeCount), gridStart(gridStart), gridStep(gridStep) {
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
}

uint32_t ProbeManager::getTotalProbeCount() const { return probeCount.x * probeCount.y * probeCount.z; }

const glm::ivec3 &ProbeManager::getProbeCount() const { return probeCount; }

const glm::vec3 &ProbeManager::getGridStart() const { return gridStart; }

float ProbeManager::getGridStep() const { return gridStep; }

const std::shared_ptr<vulkan::Image> &ProbeManager::getProbesImage() const { return probesImage; }

const std::shared_ptr<vulkan::ImageView> &ProbeManager::getProbesImageView() const { return probesImageView; }

ProbeCount::operator glm::ivec3() const { return value; }
}// namespace pf::lfp
//
// Created by petr on 11/1/20.
//

#include "Texture.h"
#include <pf_common/RAII.h>
#include "../VulkanException.h"
#include "LogicalDevice.h"
#include <stb/stb_image.h>
#include <vulkan/vulkan.hpp>

namespace pf::vulkan {

Texture::Texture(std::shared_ptr<LogicalDevice> device, CommandPool &pool,
                 FileTextureConfig &&config)
    : logicalDevice(std::move(device)) {
  int width;
  int height;
  int channels;
  auto pixels = stbi_load(config.path.string().c_str(), &width, &height, &channels,
                          static_cast<int>(config.channels));
  const auto pixelsFree = RAII([pixels] { stbi_image_free(pixels); });
  if (pixels == nullptr) {
    throw VulkanException::fmt("Texture could not be loaded: {}", config.path.string());
  }
  const auto imageSize = vk::DeviceSize(width * height * static_cast<int>(config.channels));
  auto buffer = logicalDevice->createBuffer(
      {.size = imageSize,
       .usageFlags = vk::BufferUsageFlagBits::eTransferSrc,
       .sharingMode = vk::SharingMode::eExclusive,
       .queueFamilyIndices = {logicalDevice->getQueueIndices()[vk::QueueFlagBits::eGraphics]}},
      true);

  {
    auto mapping = buffer->mapping(0);
    mapping.set(std::span<uint8_t>{pixels, imageSize});
  }

  image = logicalDevice->createImage(
      {.imageType = vk::ImageType::e2D,
       .format = TextureChannelsToVkFormat(config.channels),
       .extent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1},
       .mipLevels = config.mipLevels,
       .arrayLayers = 1,
       .sampleCount = vk::SampleCountFlagBits::e1,
       .tiling = vk::ImageTiling::eOptimal,
       .usage = config.usage,
       .sharingQueues = {},
       .layout = vk::ImageLayout::eUndefined});

  auto subRange = vk::ImageSubresourceRange();
  subRange.aspectMask = vk::ImageAspectFlagBits::eColor;
  subRange.baseMipLevel = 0;
  subRange.levelCount = 1;
  subRange.baseArrayLayer = 0;
  subRange.layerCount = 1;
  image->transitionLayout(pool, vk::ImageLayout::eTransferDstOptimal, subRange);
  {
    auto bufferCopyCmd = pool.createCommandBuffers({vk::CommandBufferLevel::ePrimary, 1})[0];
    auto subresource = vk::ImageSubresourceLayers();
    subresource.aspectMask = vk::ImageAspectFlagBits::eColor;
    subresource.mipLevel = 0;
    subresource.baseArrayLayer = 0;
    subresource.layerCount = 1;
    bufferCopyCmd->begin(vk::CommandBufferUsageFlagBits::eOneTimeSubmit)
        .copyBufferToImage(*buffer, *image, 0, 0, 0, {0, 0, 0}, subresource);
    bufferCopyCmd->submit({.waitSemaphores = {},
                           .signalSemaphores = {},
                           .flags = {},
                           .fence = std::nullopt,
                           .wait = true});
  }
  image->transitionLayout(pool, vk::ImageLayout::eShaderReadOnlyOptimal, subRange);
}

std::string Texture::info() const { return "Vulkan texture"; }
LogicalDevice &Texture::getLogicalDevice() const { return *logicalDevice; }
Image &Texture::getImage() const { return *image; }

}// namespace pf::vulkan
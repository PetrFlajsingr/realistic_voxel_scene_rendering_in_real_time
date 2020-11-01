//
// Created by petr on 11/1/20.
//

#ifndef REALISTIC_VOXEL_RENDERING_VULKAN_TYPES_TEXTURE_H
#define REALISTIC_VOXEL_RENDERING_VULKAN_TYPES_TEXTURE_H

#include "../../exceptions/StackTraceException.h"
#include "VulkanObject.h"
#include "fwd.h"
#include <filesystem>
#include <magic_enum.hpp>
#include <vulkan/vulkan.hpp>

namespace pf::vulkan {
enum class TextureChannels { grey = 1, grey_alpha = 2, rgb = 3, rgb_alpha = 4 };

inline vk::Format TextureChannelsToVkFormat(TextureChannels channels) {
  switch (channels) {
    case TextureChannels::grey: return vk::Format::eR8Srgb;
    case TextureChannels::grey_alpha: return vk::Format::eR8G8Srgb;
    case TextureChannels::rgb: return vk::Format::eR8G8B8Srgb;
    case TextureChannels::rgb_alpha: return vk::Format::eR8G8B8A8Srgb;
    default:
      throw StackTraceException::fmt("Value not handled in switch: {}",
                                     magic_enum::enum_name(channels));
  }
}

struct FileTextureConfig {
  std::filesystem::path path;
  TextureChannels channels;
  uint32_t mipLevels;
  vk::ImageUsageFlags usage;
};

class Texture : public VulkanObject {
 public:
  Texture(std::shared_ptr<LogicalDevice> device, CommandPool &pool, FileTextureConfig &&config);

  [[nodiscard]] std::string info() const override;

  [[nodiscard]] LogicalDevice &getLogicalDevice() const;
  [[nodiscard]] Image &getImage() const;

 private:
  std::shared_ptr<LogicalDevice> logicalDevice;
  std::shared_ptr<Image> image;
};

}// namespace pf::vulkan

#endif//REALISTIC_VOXEL_RENDERING_VULKAN_TYPES_TEXTURE_H

//
// Created by petr on 9/27/20.
//

#ifndef REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_IMAGE_H
#define REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_IMAGE_H

#include "../concepts/PtrConstructable.h"
#include "fwd.h"
#include "VulkanObject.h"
#include <unordered_set>
#include <vulkan/vulkan.hpp>

namespace pf::vulkan {
class ImageView;
struct ImageConfig {
  std::shared_ptr<LogicalDevice> logicalDevice;
  vk::ImageType imageType;
  vk::Format format;
  vk::Extent3D extent;
  uint32_t mipLevels;
  uint32_t arrayLayers;
  vk::SampleCountFlagBits sampleCount;
  vk::ImageTiling tiling;
  vk::ImageUsageFlagBits usage;
  std::unordered_set<uint32_t> sharingQueues;
  vk::ImageLayout layout;
};

class Image : public VulkanObject {
 public:
  [[nodiscard]] virtual const vk::Image &getImage() const = 0;

  const vk::Image &operator*() const;
  vk::Image const *operator->() const;
};

class ImageRef : public Image, public PtrConstructable<ImageRef> {
 public:
  using Image::operator*;
  using Image::operator->;
  explicit ImageRef(vk::Image img);
  [[nodiscard]] const vk::Image &getImage() const override;
  [[nodiscard]] std::string info() const override;

 private:
  vk::Image vkImage;
};

class ImageUnique : public Image, public PtrConstructable<ImageUnique> {
 public:
  using Image::operator*;
  using Image::operator->;
  explicit ImageUnique(const ImageConfig &config);

  ImageUnique(const ImageUnique &other) = delete;
  ImageUnique &operator=(const ImageUnique &other) = delete;

  [[nodiscard]] std::shared_ptr<ImageView>
  createImageView(SwapChain &swapChain, vk::ColorSpaceKHR colorSpace, vk::ImageViewType viewType,
                  const vk::ImageSubresourceRange& subResourceRange);

  [[nodiscard]] const vk::Image &getImage() const override;
  [[nodiscard]] std::string info() const override;

  [[nodiscard]] vk::ImageType getImageType() const;
  [[nodiscard]] vk::Format getFormat() const;
  [[nodiscard]] const vk::Extent3D &getExtent() const;
  [[nodiscard]] uint32_t getMipLevels() const;
  [[nodiscard]] uint32_t getArrayLayers() const;
  [[nodiscard]] vk::SampleCountFlagBits getSampleCount() const;
  [[nodiscard]] vk::ImageTiling getTiling() const;
  [[nodiscard]] vk::ImageLayout getLayout() const;

 private:
  vk::UniqueImage vkImage;
  vk::ImageType imageType;
  vk::Format format;
  vk::Extent3D extent;
  uint32_t mipLevels;
  uint32_t arrayLayers;
  vk::SampleCountFlagBits sampleCount;
  vk::ImageTiling tiling;
  vk::ImageLayout layout;
  std::shared_ptr<LogicalDevice> logicalDevice;
};

}// namespace pf::vulkan

#endif//REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_IMAGE_H

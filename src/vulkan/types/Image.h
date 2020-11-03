//
// Created by petr on 9/27/20.
//

#ifndef VOXEL_RENDER_IMAGE_H
#define VOXEL_RENDER_IMAGE_H

#include "concepts/PtrConstructible.h"
#include "VulkanObject.h"
#include "fwd.h"
#include <unordered_set>
#include <vulkan/vulkan.hpp>

namespace pf::vulkan {
class ImageView;
struct ImageConfig {
  vk::ImageType imageType;
  vk::Format format;
  vk::Extent3D extent;
  uint32_t mipLevels;
  uint32_t arrayLayers;
  vk::SampleCountFlagBits sampleCount;
  vk::ImageTiling tiling;
  vk::ImageUsageFlags usage;
  std::unordered_set<uint32_t> sharingQueues;
  vk::ImageLayout layout;
};

class Image : public VulkanObject, public std::enable_shared_from_this<Image> {
 public:
  explicit Image(std::shared_ptr<LogicalDevice> device, ImageConfig &&config);
  [[nodiscard]] virtual const vk::Image &getVkImage() const = 0;

  const vk::Image &operator*() const;
  vk::Image const *operator->() const;

  [[nodiscard]] std::shared_ptr<ImageView>
  createImageView(vk::ColorSpaceKHR colorSpace, vk::ImageViewType viewType,
                  const vk::ImageSubresourceRange &subResourceRange);
  [[nodiscard]] vk::ImageType getImageType() const;
  [[nodiscard]] vk::Format getFormat() const;
  [[nodiscard]] const vk::Extent3D &getExtent() const;
  [[nodiscard]] uint32_t getMipLevels() const;
  [[nodiscard]] uint32_t getArrayLayers() const;
  [[nodiscard]] vk::SampleCountFlagBits getSampleCount() const;
  [[nodiscard]] vk::ImageTiling getTiling() const;
  [[nodiscard]] vk::ImageLayout getLayout() const;

  [[nodiscard]] LogicalDevice &getLogicalDevice();

  void transitionLayout(CommandPool &cmdPool, vk::ImageLayout newLayout,
                        const vk::ImageSubresourceRange& subresourceRange);

 protected:
  vk::ImageType imageType;
  vk::Format format;
  vk::Extent3D extent;
  uint32_t mipLevels{};
  uint32_t arrayLayers{};
  vk::SampleCountFlagBits sampleCount;
  vk::ImageTiling tiling;
  vk::ImageLayout layout;
  vk::ImageUsageFlags usage;
  std::unordered_set<uint32_t> sharingQueues;
  std::shared_ptr<LogicalDevice> logicalDevice;
};

class ImageRef : public Image, public PtrConstructible<ImageRef> {
 public:
  using Image::operator*;
  using Image::operator->;
  explicit ImageRef(std::shared_ptr<LogicalDevice> device, ImageConfig &&config, vk::Image img);
  [[nodiscard]] const vk::Image &getVkImage() const override;
  [[nodiscard]] std::string info() const override;

 private:
  vk::Image vkImage;
};

class ImageUnique : public Image,
                    public PtrConstructible<ImageUnique>,
                    public std::enable_shared_from_this<ImageUnique> {
 public:
  using Image::operator*;
  using Image::operator->;
  explicit ImageUnique(std::shared_ptr<LogicalDevice> device, ImageConfig &&config);

  ImageUnique(const ImageUnique &other) = delete;
  ImageUnique &operator=(const ImageUnique &other) = delete;

  [[nodiscard]] const vk::Image &getVkImage() const override;
  [[nodiscard]] std::string info() const override;

 private:
  uint32_t findMemoryType(uint32_t memoryTypeBits);

  vk::UniqueImage vkImage;
  vk::UniqueDeviceMemory vkMemory;
};

}// namespace pf::vulkan

#endif//VOXEL_RENDER_IMAGE_H

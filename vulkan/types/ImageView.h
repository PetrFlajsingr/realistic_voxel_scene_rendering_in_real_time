//
// Created by petr on 9/27/20.
//

#ifndef REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_IMAGEVIEW_H
#define REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_IMAGEVIEW_H

#include "../concepts/PtrConstructable.h"
#include "fwd.h"
#include "VulkanObject.h"
#include <vulkan/vulkan.hpp>

namespace pf::vulkan {

class SwapChain;

struct ImageViewConfig {
  LogicalDevice &logicalDevice;
  SwapChain &swapChain;
  Image &image;
  vk::Format format;
  vk::ColorSpaceKHR colorSpace;
  vk::ImageViewType viewType;
  vk::ImageSubresourceRange subResourceRange;
};

class ImageView : public VulkanObject, public PtrConstructable<ImageView> {
 public:
  explicit ImageView(const ImageViewConfig &config);

  ImageView(const ImageView &other) = delete;
  ImageView &operator=(const ImageView &other) = delete;

  [[nodiscard]] const vk::ImageView &getImageView();

  const vk::ImageView &operator*() const;
  vk::ImageView const *operator->() const;

  [[nodiscard]] vk::Format getFormat() const;
  [[nodiscard]] vk::ColorSpaceKHR getColorSpace() const;
  [[nodiscard]] vk::ImageViewType getViewType() const;

  [[nodiscard]] std::string info() const override;

 private:
  vk::UniqueImageView vkImageView;
  vk::Format format;
  vk::ColorSpaceKHR colorSpace;
  vk::ImageViewType viewType;
};

}// namespace pf::vulkan

#endif//REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_IMAGEVIEW_H

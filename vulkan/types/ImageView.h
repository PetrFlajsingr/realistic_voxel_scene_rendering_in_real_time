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
  vk::Format format;
  vk::ColorSpaceKHR colorSpace;
  vk::ImageViewType viewType;
  vk::ImageSubresourceRange subResourceRange;
};

class ImageView : public VulkanObject, public PtrConstructable<ImageView> {
 public:
  explicit ImageView(std::shared_ptr<Image> img, const ImageViewConfig &config);

  ImageView(const ImageView &other) = delete;
  ImageView &operator=(const ImageView &other) = delete;

  [[nodiscard]] const vk::ImageView &getImageView();

  const vk::ImageView &operator*() const;
  vk::ImageView const *operator->() const;

  [[nodiscard]] vk::Format getFormat() const;
  [[nodiscard]] vk::ColorSpaceKHR getColorSpace() const;
  [[nodiscard]] vk::ImageViewType getViewType() const;
  [[nodiscard]] const vk::ImageSubresourceRange &getSubresourceRange() const;

  [[nodiscard]] std::string info() const override;

 private:
  std::shared_ptr<Image> image;
  vk::UniqueImageView vkImageView;
  vk::Format format;
  vk::ColorSpaceKHR colorSpace;
  vk::ImageViewType viewType;
  vk::ImageSubresourceRange subresourceRange;
};

}// namespace pf::vulkan

#endif//REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_IMAGEVIEW_H

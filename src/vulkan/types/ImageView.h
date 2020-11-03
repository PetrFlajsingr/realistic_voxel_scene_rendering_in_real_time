//
// Created by petr on 9/27/20.
//

#ifndef VOXEL_RENDER_IMAGEVIEW_H
#define VOXEL_RENDER_IMAGEVIEW_H

#include <pf_common/concepts/PtrConstructible.h>
#include "VulkanObject.h"
#include "fwd.h"
#include <vulkan/vulkan.hpp>

namespace pf::vulkan {

class SwapChain;

struct ImageViewConfig {
  vk::Format format;
  vk::ColorSpaceKHR colorSpace;
  vk::ImageViewType viewType;
  vk::ImageSubresourceRange subResourceRange;
};

class ImageView : public VulkanObject, public PtrConstructible<ImageView> {
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

#endif//VOXEL_RENDER_IMAGEVIEW_H

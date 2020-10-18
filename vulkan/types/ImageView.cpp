//
// Created by petr on 9/27/20.
//

#include "ImageView.h"
#include "Image.h"
#include "PhysicalDevice.h"

namespace pf::vulkan {
ImageView::ImageView(std::shared_ptr<Image> img, const ImageViewConfig &config)
    : image(std::move(img)) {
  auto createInfo = vk::ImageViewCreateInfo{};
  createInfo.setImage(**image)
      .setViewType(config.viewType)
      .setFormat(config.format)
      .setSubresourceRange(config.subResourceRange);
  vkImageView = image->getLogicalDevice().getVkLogicalDevice().createImageViewUnique(createInfo);
  format = config.format;
  colorSpace = config.colorSpace;
  viewType = config.viewType;
  subresourceRange = config.subResourceRange;
}

const vk::ImageView &ImageView::getImageView() { return vkImageView.get(); }

std::string ImageView::info() const { return "Vulkan image view unique"; }

const vk::ImageView &ImageView::operator*() const { return *vkImageView; }

vk::ImageView const *ImageView::operator->() const { return &*vkImageView; }

vk::Format ImageView::getFormat() const { return format; }

vk::ColorSpaceKHR ImageView::getColorSpace() const { return colorSpace; }

vk::ImageViewType ImageView::getViewType() const { return viewType; }

const vk::ImageSubresourceRange &ImageView::getSubresourceRange() const { return subresourceRange; }

}// namespace pf::vulkan

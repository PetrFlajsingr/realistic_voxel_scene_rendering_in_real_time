//
// Created by petr on 9/27/20.
//

#include "ImageView.h"
#include "Image.h"
#include "Device.h"

pf::vulkan::ImageView::ImageView(const ImageViewConfig &config) {
  auto createInfo = vk::ImageViewCreateInfo{};
  createInfo.setImage(config.image.getImage())
      .setViewType(config.viewType)
      .setFormat(config.format)
      .setSubresourceRange(config.subResourceRange);
  vkImageView = config.logicalDevice.getVkLogicalDevice().createImageViewUnique(createInfo);
  format = config.format;
  colorSpace = config.colorSpace;
  viewType = config.viewType;
}

const vk::ImageView &pf::vulkan::ImageView::getImageView() { return vkImageView.get(); }

std::string pf::vulkan::ImageView::info() const { return "Vulkan image view unique"; }

const vk::ImageView &pf::vulkan::ImageView::operator*() const { return *vkImageView; }

vk::ImageView const *pf::vulkan::ImageView::operator->() const { return &*vkImageView; }

vk::Format pf::vulkan::ImageView::getFormat() const { return format; }

vk::ColorSpaceKHR pf::vulkan::ImageView::getColorSpace() const { return colorSpace; }

vk::ImageViewType pf::vulkan::ImageView::getViewType() const { return viewType; }

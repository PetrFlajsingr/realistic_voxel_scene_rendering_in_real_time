//
// Created by petr on 11/1/20.
//

#include "TextureSampler.h"
#include "LogicalDevice.h"


namespace pf::vulkan {

std::string TextureSampler::info() const { return "Texture sampler"; }
TextureSampler::TextureSampler(std::shared_ptr<LogicalDevice> device,
                               TextureSamplerConfig &&config) : logicalDevice(std::move(device)) {
  auto createInfo = vk::SamplerCreateInfo();
  createInfo.magFilter = config.magFilter;
  createInfo.minFilter = config.minFilter;
  createInfo.addressModeU = config.addresssMode.u;
  createInfo.addressModeV = config.addresssMode.v;
  createInfo.addressModeW = config.addresssMode.w;
  createInfo.anisotropyEnable = config.maxAnisotropy.has_value();
  createInfo.maxAnisotropy = config.maxAnisotropy.value_or(0.f);
  createInfo.borderColor = config.borderColor;
  createInfo.unnormalizedCoordinates = config.unnormalizedCoordinates;
  createInfo.compareEnable = config.compareOp.has_value();
  createInfo.compareOp = config.compareOp.value_or(vk::CompareOp::eAlways);
  createInfo.mipmapMode = config.mip.mode;
  createInfo.mipLodBias = config.mip.lodBias;
  createInfo.minLod = config.mip.minLod;
  createInfo.maxLod = config.mip.maxLod;

  vkSampler = logicalDevice->getVkLogicalDevice().createSamplerUnique(createInfo);
}

}
//
// Created by petr on 9/28/20.
//

#include "DescriptorSetLayout.h"

pf::vulkan::DescriptorSetLayout::DescriptorSetLayout(const DescriptorSetLayoutConfig &config) {
  auto bindings = std::vector<vk::DescriptorSetLayoutBinding>();
  for (const auto &bindingConfig : config.bindings) {
    auto binding = vk::DescriptorSetLayoutBinding();
    binding.setBinding(bindingConfig.binding)
        .setDescriptorType(bindingConfig.type)
        .setDescriptorCount(bindingConfig.count)
        .setStageFlags(bindingConfig.stageFlags);
    bindings.emplace_back(std::move(binding));
  }
  auto create_info = vk::DescriptorSetLayoutCreateInfo();
  create_info.setBindings(bindings);
  vkSet = config.logicalDevice.getVkLogicalDevice().createDescriptorSetLayoutUnique(create_info);
}

const vk::DescriptorSetLayout &pf::vulkan::DescriptorSetLayout::getLayout() const {
  return vkSet.get();
}

std::string pf::vulkan::DescriptorSetLayout::info() const {
  return "Vulkan descriptor set layout unique";
}

const vk::DescriptorSetLayout &pf::vulkan::DescriptorSetLayout::operator*() const { return *vkSet; }

vk::DescriptorSetLayout const *pf::vulkan::DescriptorSetLayout::operator->() const {
  return &*vkSet;
}
//
// Created by petr on 9/28/20.
//

#include "DescriptorSetLayout.h"
#include "PhysicalDevice.h"

namespace pf::vulkan {
DescriptorSetLayout::DescriptorSetLayout(std::shared_ptr<LogicalDevice> device,
                                         DescriptorSetLayoutConfig &&config)
    : logicalDevice(std::move(device)) {
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
  vkSet = logicalDevice->getVkLogicalDevice().createDescriptorSetLayoutUnique(create_info);
}

const vk::DescriptorSetLayout &DescriptorSetLayout::getLayout() const { return vkSet.get(); }

std::string DescriptorSetLayout::info() const { return "Vulkan descriptor set layout unique"; }

const vk::DescriptorSetLayout &DescriptorSetLayout::operator*() const { return *vkSet; }

vk::DescriptorSetLayout const *DescriptorSetLayout::operator->() const { return &*vkSet; }

LogicalDevice &DescriptorSetLayout::getDevice() { return *logicalDevice; }

}// namespace pf::vulkan
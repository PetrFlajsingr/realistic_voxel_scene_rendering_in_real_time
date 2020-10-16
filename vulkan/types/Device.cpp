//
// Created by petr on 9/26/20.
//

#include "Device.h"
#include "../VulkanException.h"
#include <utility>

const vk::PhysicalDevice &pf::vulkan::Device::getPhysicalDevice() { return vkDevice; }

pf::vulkan::LogicalDevice &
pf::vulkan::Device::getLogicalDevice(const pf::vulkan::LogicalDeviceId &id) {
  return *logicalDevices[id].get();
}

std::string pf::vulkan::Device::info() const { return "Vulkan physical device unique"; }

const vk::PhysicalDevice &pf::vulkan::Device::operator*() const { return vkDevice; }

vk::PhysicalDevice const *pf::vulkan::Device::operator->() const { return &vkDevice; }

pf::vulkan::LogicalDevice::LogicalDevice(
    const std::shared_ptr<Device> &device, vk::UniqueDevice &&vkLogicalDevice,
    std::unordered_map<vk::QueueFlagBits, uint32_t> queueIndices,
    const std::optional<uint32_t> &presentQueueIndex)
    : physDevice(device), vkLogicalDevice(std::move(vkLogicalDevice)),
      queueIndices(std::move(queueIndices)), presentQueueIndex(presentQueueIndex) {}

const vk::Device &pf::vulkan::LogicalDevice::getVkLogicalDevice() const {
  return vkLogicalDevice.get();
}

std::unordered_map<vk::QueueFlagBits, uint32_t> &pf::vulkan::LogicalDevice::getQueueIndices() {
  return queueIndices;
}

const std::optional<uint32_t> &pf::vulkan::LogicalDevice::getPresentQueueIndex() const {
  return presentQueueIndex;
}

std::string pf::vulkan::LogicalDevice::info() const { return "Vulkan logical device unique"; }

const vk::Device &pf::vulkan::LogicalDevice::operator*() const { return *vkLogicalDevice; }

vk::Device const *pf::vulkan::LogicalDevice::operator->() const { return &*vkLogicalDevice; }

pf::vulkan::Device &pf::vulkan::LogicalDevice::getPhysicalDevice() const {
  auto ptr = physDevice.lock();
  return *ptr;
}

vk::Queue pf::vulkan::LogicalDevice::getQueue(vk::QueueFlagBits type) {
  return vkLogicalDevice->getQueue(queueIndices[type], 0);
}

std::unordered_map<vk::QueueFlagBits, uint32_t>
pf::vulkan::details::getQueueFamilyIndices(vk::PhysicalDevice &physicalDevice,
                                           std::unordered_set<vk::QueueFlagBits> queueTypes) {
  log(spdlog::level::info, VK_TAG, "Getting queue family indices.");
  const auto queue_family_properties_list = physicalDevice.getQueueFamilyProperties();
  auto result = std::unordered_map<vk::QueueFlagBits, uint32_t>{};
  for (const auto &[idx, queue_family_properties] :
       ranges::views::enumerate(queue_family_properties_list)) {

    const auto iter = std::ranges::find_if(queueTypes, [&](auto flag) {
      return (queue_family_properties.queueCount > 0
              && (queue_family_properties.queueFlags & flag));
    });
    if (iter != queueTypes.end()) {
      result[*iter] = idx;
      queueTypes.erase(iter);
    }
  }
  if (!queueTypes.empty()) {
    throw VulkanException("get_queue_family_indices not all queue types found.");
  }
  return result;
}

uint32_t pf::vulkan::details::getPresentQueueFamilyIndex(vk::PhysicalDevice &physicalDevice,
                                                         const vk::SurfaceKHR &surface) {
  using namespace ranges;
  const auto queue_family_properties_list = physicalDevice.getQueueFamilyProperties();
  for (const auto &[idx, queue_family_properties] :
       views::enumerate(queue_family_properties_list)) {
    if (physicalDevice.getSurfaceSupportKHR(idx, surface)) { return idx; }
  }
  throw VulkanException("get_present_queue_family_index present queue not found.");
}

std::vector<vk::DeviceQueueCreateInfo>
pf::vulkan::details::buildQueueCreateInfo(const std::unordered_set<uint32_t> &queueIndices,
                                          std::vector<float> &queuePriorities) {
  if (queueIndices.size() != queuePriorities.size()) {
    throw VulkanException("build_queue_create_info priorities size doesn't match queues.");
  }
  auto result = std::vector<vk::DeviceQueueCreateInfo>{};
  for (auto idx : queueIndices) {
    auto create_info = vk::DeviceQueueCreateInfo{};
    create_info.setQueueFamilyIndex(idx).setQueueCount(1).setQueuePriorities(queuePriorities);
    result.push_back(create_info);
  }
  return result;
}
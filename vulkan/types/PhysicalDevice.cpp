//
// Created by petr on 9/26/20.
//

#include "PhysicalDevice.h"
#include "../VulkanException.h"
#include "Instance.h"
#include "LogicalDevice.h"
#include "Surface.h"
#include "SwapChain.h"
#include <utility>

namespace pf::vulkan {

const vk::PhysicalDevice &PhysicalDevice::getPhysicalDevice() { return vkDevice; }

LogicalDevice &PhysicalDevice::getLogicalDevice(const LogicalDeviceId &id) {
  return *logicalDevices[id].get();
}

std::string PhysicalDevice::info() const { return "Vulkan physical device unique"; }

const vk::PhysicalDevice &PhysicalDevice::operator*() const { return vkDevice; }

vk::PhysicalDevice const *PhysicalDevice::operator->() const { return &vkDevice; }

std::vector<vk::PhysicalDevice> PhysicalDevice::getPhysicalDevices() {
  return instance->getInstance().enumeratePhysicalDevices();
}

std::shared_ptr<LogicalDevice> &PhysicalDevice::createLogicalDevice(LogicalDeviceConfig config) {
  logFmt(spdlog::level::info, VK_TAG, "Creating logical device with id: {}.", config.id);
  using namespace ranges;
  const auto queueFamilyIndices =
      details::getQueueFamilyIndices(vkDevice, std::move(config.queueTypes));

  auto queuePriorities = std::vector(queueFamilyIndices.size(), 1.0f);

  const auto queueIndicesView = queueFamilyIndices | views::values;
  auto queueIndices = std::unordered_set(queueIndicesView.begin(), queueIndicesView.end());
  auto presentIndex = std::optional<uint32_t>(std::nullopt);
  if (config.presentQueueEnabled) {
    const auto presentIdx =
        details::getPresentQueueFamilyIndex(vkDevice, config.surface.getSurface());
    queueIndices.emplace(presentIdx);
    presentIndex = presentIdx;
  }

  auto queueCreateInfos = details::buildQueueCreateInfo(queueIndices, queuePriorities);

  const auto requiredExtensionsCStr = config.requiredDeviceExtensions
      | views::transform([](auto &str) { return str.c_str(); }) | to_vector;
  const auto validationLayersCStr =
      config.validationLayers | views::transform([](auto &str) { return str.c_str(); }) | to_vector;

  auto createInfo = vk::DeviceCreateInfo{};
  createInfo.setQueueCreateInfos(queueCreateInfos)
      .setPEnabledFeatures(&config.deviceFeatures)
      .setPEnabledExtensionNames(requiredExtensionsCStr)
      .setPEnabledLayerNames(validationLayersCStr);

  logicalDevices[config.id] =
      LogicalDevice::CreateShared(shared_from_this(), vkDevice.createDeviceUnique(createInfo),
                                  queueFamilyIndices, presentIndex);
  return logicalDevices[config.id];
}

std::unordered_map<vk::QueueFlagBits, uint32_t>
details::getQueueFamilyIndices(vk::PhysicalDevice &physicalDevice,
                               std::unordered_set<vk::QueueFlagBits> queueTypes) {
  log(spdlog::level::info, VK_TAG, "Getting queue family indices.");
  const auto queueFamilyPropertiesList = physicalDevice.getQueueFamilyProperties();
  auto result = std::unordered_map<vk::QueueFlagBits, uint32_t>{};
  for (const auto &[idx, queueFamilyProperties] :
       ranges::views::enumerate(queueFamilyPropertiesList)) {

    const auto iter = std::ranges::find_if(queueTypes, [&](auto flag) {
      return (queueFamilyProperties.queueCount > 0 && (queueFamilyProperties.queueFlags & flag));
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

uint32_t details::getPresentQueueFamilyIndex(vk::PhysicalDevice &physicalDevice,
                                             const vk::SurfaceKHR &surface) {
  using namespace ranges;
  const auto queueFamilyPropertiesList = physicalDevice.getQueueFamilyProperties();
  for (const auto &[idx, queueFamilyProperties] : views::enumerate(queueFamilyPropertiesList)) {
    if (physicalDevice.getSurfaceSupportKHR(idx, surface)) { return idx; }
  }
  throw VulkanException("get_present_queue_family_index present queue not found.");
}

std::vector<vk::DeviceQueueCreateInfo>
details::buildQueueCreateInfo(const std::unordered_set<uint32_t> &queueIndices,
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
}// namespace pf::vulkan
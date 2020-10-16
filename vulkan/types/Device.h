//
// Created by petr on 9/26/20.
//

#ifndef REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_DEVICE_H
#define REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_DEVICE_H

#include "../DefaultDeviceSuitabilityScorer.h"
#include "../VulkanException.h"
#include "../concepts/PtrConstructable.h"
#include "../logging/loggers.h"
#include "fwd.h"
#include "Instance.h"
#include "../concepts/Window.h"
#include "VulkanObject.h"
#include <range/v3/action.hpp>
#include <range/v3/view.hpp>
#include <unordered_set>
#include <vulkan/vulkan.hpp>

namespace pf::vulkan {

using LogicalDeviceId = std::string;

template<window::Window Window>
struct LogicalDeviceConfig {
  LogicalDeviceId id;
  vk::PhysicalDeviceFeatures deviceFeatures;
  std::unordered_set<vk::QueueFlagBits> queueTypes;
  bool presentQueueEnabled{};
  std::unordered_set<std::string> requiredDeviceExtensions;
  std::unordered_set<std::string> validationLayers;
  Surface &surface;
};

class Device;

class LogicalDevice : public VulkanObject, public PtrConstructable<LogicalDevice> {
 public:
  LogicalDevice(const std::shared_ptr<Device>& device, vk::UniqueDevice &&vkLogicalDevice,
                std::unordered_map<vk::QueueFlagBits, uint32_t> queueIndices,
                const std::optional<uint32_t> &presentQueueIndex);

  LogicalDevice(const LogicalDevice &other) = delete;
  LogicalDevice &operator=(const LogicalDevice &other) = delete;

  [[nodiscard]] const vk::Device &getVkLogicalDevice() const;
  [[nodiscard]] std::unordered_map<vk::QueueFlagBits, uint32_t> &getQueueIndices();
  [[nodiscard]] vk::Queue getQueue(vk::QueueFlagBits type);
  [[nodiscard]] const std::optional<uint32_t> &getPresentQueueIndex() const;

  const vk::Device &operator*() const;
  vk::Device const *operator->() const;

  [[nodiscard]] Device &getPhysicalDevice() const;

  std::string info() const override;

 private:
  std::weak_ptr<Device> physDevice;
  vk::UniqueDevice vkLogicalDevice;
  std::unordered_map<vk::QueueFlagBits, uint32_t> queueIndices;
  std::optional<uint32_t> presentQueueIndex;
};

namespace details {
std::unordered_map<vk::QueueFlagBits, uint32_t>
getQueueFamilyIndices(vk::PhysicalDevice &physicalDevice,
                      std::unordered_set<vk::QueueFlagBits> queueTypes);

uint32_t getPresentQueueFamilyIndex(vk::PhysicalDevice &physicalDevice,
                                    const vk::SurfaceKHR &surface);
// TODO: counts
std::vector<vk::DeviceQueueCreateInfo>
buildQueueCreateInfo(const std::unordered_set<uint32_t> &queueIndices,
                     std::vector<float> &queuePriorities);
}// namespace details

struct DeviceConfig {
  Instance &instance;
};

class Device : public VulkanObject,
               public PtrConstructable<Device>,
               public std::enable_shared_from_this<Device> {
 public:
  template<DeviceSuitabilityScorer DeviceScorer>
  explicit Device(const DeviceConfig &config, DeviceScorer &&scorer)
      : vkDevice(selectPhysicalDevice(config, scorer)) {}

  Device(const Device &) = delete;
  Device &operator=(const Device &) = delete;

  [[nodiscard]] const vk::PhysicalDevice &getPhysicalDevice();

  const vk::PhysicalDevice &operator*() const;
  vk::PhysicalDevice const *operator->() const;

  [[nodiscard]] LogicalDevice &getLogicalDevice(const LogicalDeviceId &id);

  std::string info() const override;

  template<window::Window Window>
  std::shared_ptr<LogicalDevice> &createLogicalDevice(LogicalDeviceConfig<Window> config) {
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
    const auto validationLayersCStr = config.validationLayers
        | views::transform([](auto &str) { return str.c_str(); }) | to_vector;

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

 private:
  template<DeviceSuitabilityScorer DeviceScorer>
  static vk::PhysicalDevice selectPhysicalDevice(const DeviceConfig &config,
                                                 DeviceScorer &&device_scorer) {
    using namespace ranges;
    log(spdlog::level::info, VK_TAG, "Selecting physical device.");
    const auto physicalDevices = config.instance.getInstance().enumeratePhysicalDevices();
    const auto suitableDevices = physicalDevices | views::transform([&](const auto &device) {
                                   const auto deviceName = device.getProperties().deviceName;
                                   const auto score = device_scorer(device);
                                   logFmt(spdlog::level::info, VK_TAG, "Device name: {}, score: {}",
                                          deviceName, score.has_value() ? *score : -1);
                                   return std::make_pair(score, device);
                                 })
        | views::filter([](const auto &scored_device) { return scored_device.first.has_value(); })
        | to_vector | actions::sort([](const auto &dev_a, const auto &dev_b) {
                                   return dev_a.first.value() > dev_b.first.value();
                                 });
    if (suitableDevices.empty()) {
      throw VulkanException("No suitable physical device was found.");
    }
    const auto selectedDevice = suitableDevices.front().second;
    const auto deviceName = selectedDevice.getProperties().deviceName;
    const auto score = suitableDevices.front().first.value();
    logFmt(spdlog::level::info, VK_TAG, "Selected device: Device name: {}, score: {}", deviceName,
           score);
    return selectedDevice;
  }

  vk::PhysicalDevice vkDevice;
  std::unordered_map<LogicalDeviceId, std::shared_ptr<LogicalDevice>> logicalDevices;
};

}// namespace pf::vulkan

#endif//REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_DEVICE_H

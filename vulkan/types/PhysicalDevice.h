//
// Created by petr on 9/26/20.
//

#ifndef VOXEL_RENDER_PHYSICALDEVICE_H
#define VOXEL_RENDER_PHYSICALDEVICE_H

#include "../DefaultDeviceSuitabilityScorer.h"
#include "../VulkanException.h"
#include "../concepts/PtrConstructible.h"
#include "../concepts/Window.h"
#include "../logging/loggers.h"
#include "LogicalDevice.h"
#include "VulkanObject.h"
#include "fwd.h"
#include <range/v3/action.hpp>
#include <range/v3/view.hpp>
#include <unordered_set>
#include <vulkan/vulkan.hpp>

namespace pf::vulkan {

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

class PhysicalDevice : public VulkanObject,
                       public PtrConstructible<PhysicalDevice>,
                       public std::enable_shared_from_this<PhysicalDevice> {
 public:
  template<DeviceSuitabilityScorer DeviceScorer>
  explicit PhysicalDevice(std::shared_ptr<Instance> instance, DeviceScorer &&scorer)
      : instance(std::move(instance)),
        vkDevice(selectPhysicalDevice(getPhysicalDevices(), scorer)) {}

  PhysicalDevice(const PhysicalDevice &) = delete;
  PhysicalDevice &operator=(const PhysicalDevice &) = delete;

  [[nodiscard]] const vk::PhysicalDevice &getPhysicalDevice();

  const vk::PhysicalDevice &operator*() const;
  vk::PhysicalDevice const *operator->() const;

  [[nodiscard]] LogicalDevice &getLogicalDevice(const LogicalDeviceId &id);

  std::string info() const override;

  [[nodiscard]] std::shared_ptr<LogicalDevice> &createLogicalDevice(LogicalDeviceConfig config);

  [[nodiscard]] Instance &getInstance() const;

 private:
  std::vector<vk::PhysicalDevice> getPhysicalDevices();

  template<DeviceSuitabilityScorer DeviceScorer>
  static vk::PhysicalDevice
  selectPhysicalDevice(const std::vector<vk::PhysicalDevice> &physicalDevices,
                       DeviceScorer &&device_scorer);

  std::shared_ptr<Instance> instance;
  vk::PhysicalDevice vkDevice;
  std::unordered_map<LogicalDeviceId, std::shared_ptr<LogicalDevice>> logicalDevices;
};

template<DeviceSuitabilityScorer DeviceScorer>
vk::PhysicalDevice
PhysicalDevice::selectPhysicalDevice(const std::vector<vk::PhysicalDevice> &physicalDevices,
                                     DeviceScorer &&device_scorer) {
  using namespace ranges;
  log(spdlog::level::info, VK_TAG, "Selecting physical device.");
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
  if (suitableDevices.empty()) { throw VulkanException("No suitable physical device was found."); }
  const auto selectedDevice = suitableDevices.front().second;
  const auto deviceName = selectedDevice.getProperties().deviceName;
  const auto score = suitableDevices.front().first.value();
  logFmt(spdlog::level::info, VK_TAG, "Selected device: Device name: {}, score: {}", deviceName,
         score);
  return selectedDevice;
}

}// namespace pf::vulkan

#endif//VOXEL_RENDER_PHYSICALDEVICE_H

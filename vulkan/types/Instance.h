//
// Created by petr on 9/26/20.
//

#ifndef REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_INSTANCE_H
#define REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_INSTANCE_H

#include "../concepts/PtrConstructable.h"
#include "VulkanCommon.h"
#include "VulkanObject.h"
#include "PhysicalDevice.h"
#include "../DefaultDeviceSuitabilityScorer.h"
#include <unordered_set>
#include <vulkan/vulkan.hpp>

namespace pf::vulkan {
struct InstanceConfig {
  std::string appName;
  Version appVersion;
  Version vkVersion;
  EngineInfo engineInfo;
  std::unordered_set<std::string> requiredWindowExtensions;
  std::unordered_set<std::string> validationLayers;

  std::optional<VulkanDebugCallback> callback;
};

class Instance : public VulkanObject,
                 public PtrConstructable<Instance>,
                 public std::enable_shared_from_this<Instance> {
 public:
  explicit Instance(InstanceConfig config);

  Instance(const Instance &other) = delete;
  Instance &operator=(const Instance &other) = delete;

  void setDebugCallback(const VulkanDebugCallback &callback);
  void setVkInstance(vk::UniqueInstance &&instance);
  void setDebugMessenger(DynamicUniqueDebugUtilsMessengerEXT &&debugMessenger);

  const vk::Instance &operator*() const;
  vk::Instance const *operator->() const;

  [[nodiscard]] const vk::Instance &getInstance();
  [[nodiscard]] std::optional<std::reference_wrapper<const vk::DebugUtilsMessengerEXT>>
  getDebugMessenger();

  template<DeviceSuitabilityScorer DeviceScorer>
  [[nodiscard]] std::shared_ptr<PhysicalDevice> selectDevice(DeviceScorer &&scorer);

  ~Instance() override;

  static VKAPI_ATTR VkBool32 VKAPI_CALL cVulkanDebugCallback(
      VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT msgTypeFlags,
      const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *user_data);

  [[nodiscard]] std::string info() const override;

 private:
  VulkanDebugCallback debugCallback;
  vk::UniqueInstance vkInstance;
  std::optional<DynamicUniqueDebugUtilsMessengerEXT> debugMessenger = std::nullopt;
};

template<DeviceSuitabilityScorer DeviceScorer>
std::shared_ptr<PhysicalDevice> Instance::selectDevice(DeviceScorer &&scorer) {
  return PhysicalDevice::CreateShared(shared_from_this(), std::forward<DeviceScorer>(scorer));
}

}// namespace pf::vulkan
#endif//REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_INSTANCE_H
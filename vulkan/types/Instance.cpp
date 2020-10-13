//
// Created by petr on 9/26/20.
//

#include "Instance.h"
#include "../logging/loggers.h"
#include <range/v3/view.hpp>
#include <utility>

pf::vulkan::Instance::Instance(pf::vulkan::InstanceConfig config) {
  log(spdlog::level::info, VK_TAG, "Creating vulkan instance.");
  const auto app_info = vk::ApplicationInfo(
      config.appName.c_str(), versionToUint32(config.appVersion), config.engineInfo.name.c_str(),
      versionToUint32(config.engineInfo.engineVersion), versionToUint32(config.vkVersion));
  logFmt(spdlog::level::info, VK_TAG,
         "App name: {}\nversion: {}\nengine name: {}\nengine version: {}\nvulkan version: {}.",
         config.appName, config.appVersion, config.engineInfo.name, config.engineInfo.engineVersion,
         config.vkVersion);

  const auto message_severity_flags = vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose
      | vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo
      | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning
      | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
  const auto message_type_flags = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral
      | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance
      | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation;
  const auto debug_create_info = vk::DebugUtilsMessengerCreateInfoEXT(
      {}, message_severity_flags, message_type_flags, cVulkanDebugCallback, this);
  using namespace ranges;

  const auto validation_layers_enabled =
      !config.validationLayers.empty() && config.callback.has_value();
  if (validation_layers_enabled) {
    log(spdlog::level::info, VK_TAG, "Validation layers are enabled.");
    config.requiredWindowExtensions.emplace(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    debugCallback = config.callback.value();
  }

  const auto ext_c_str = config.requiredWindowExtensions
      | views::transform([](auto &str) { return str.c_str(); }) | to_vector;
  const auto layer_c_str = config.validationLayers
      | ranges::views::transform([](auto &str) { return str.c_str(); }) | to_vector;

  auto create_info = vk::InstanceCreateInfo();
  create_info.setPApplicationInfo(&app_info)
      .setPEnabledExtensionNames(ext_c_str)
      .setPEnabledLayerNames(layer_c_str)
      .setPNext(validation_layers_enabled ? &debug_create_info : nullptr);

  vkInstance = vk::createInstanceUnique(create_info);

  if (validation_layers_enabled) {
    debugMessenger = vkInstance->createDebugUtilsMessengerEXTUnique(
        debug_create_info, nullptr, vk::DispatchLoaderDynamic{*vkInstance, vkGetInstanceProcAddr});
  }
  log(spdlog::level::info, VK_TAG, "Vulkan instance created.");
}

const vk::Instance &pf::vulkan::Instance::getInstance() { return vkInstance.get(); }

std::optional<std::reference_wrapper<const vk::DebugUtilsMessengerEXT>>
pf::vulkan::Instance::getDebugMessenger() {
  if (debugMessenger.has_value()) { return debugMessenger->get(); }
  return std::nullopt;
}

pf::vulkan::Instance::~Instance() {
  if (debugMessenger.has_value()) { debugMessenger->release(); }
  vkInstance.release();
}

void pf::vulkan::Instance::setDebugCallback(const pf::vulkan::VulkanDebugCallback &callback) {
  debugCallback = callback;
}

void pf::vulkan::Instance::setVkInstance(vk::UniqueInstance &&instance) {
  vkInstance = std::move(instance);
}

void pf::vulkan::Instance::setDebugMessenger(DynamicUniqueDebugUtilsMessengerEXT &&messenger) {
  debugMessenger = std::move(messenger);
}

VkBool32 pf::vulkan::Instance::cVulkanDebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT msgTypeFlags,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *user_data) {
  auto self = reinterpret_cast<Instance *>(user_data);
  return self->debugCallback(DebugCallbackData::fromVk(*pCallbackData),
                             static_cast<vk::DebugUtilsMessageSeverityFlagBitsEXT>(severity),
                             static_cast<vk::DebugUtilsMessageTypeFlagsEXT>(msgTypeFlags))
      ? VK_TRUE
      : VK_FALSE;
}
std::string pf::vulkan::Instance::info() const { return "Vulkan instance unique"; }

const vk::Instance &pf::vulkan::Instance::operator*() const { return *vkInstance; }

vk::Instance const *pf::vulkan::Instance::operator->() const { return &*vkInstance; }

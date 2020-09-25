//
// Created by petr on 9/25/20.
//

#include "utils.h"
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <range/v3/view/transform.hpp>

uint32_t pf::vulkan::version_to_uint32(const pf::vulkan::version &version) {
  return VK_MAKE_VERSION(version.major, version.minor, version.patch);
}

pf::vulkan::instance_create_result
pf::vulkan::create_instance(pf::vulkan::instance_config config,
                            const pf::vulkan::details::debug_config &debug_config) {
  const auto app_info = vk::ApplicationInfo(
      config.app_name.c_str(), version_to_uint32(config.app_version), config.e_info.name.c_str(),
      version_to_uint32(config.e_info.engine_version), version_to_uint32(config.vk_version));

  const auto message_severity_flags = vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose
      | vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo
      | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning
      | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
  const auto message_type_flags = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral
      | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance
      | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation;
  const auto debug_create_info =
      vk::DebugUtilsMessengerCreateInfoEXT({}, message_severity_flags, message_type_flags,
                                           debug_config.callback, debug_config.callback_data);

  const auto validation_layers_enabled = config.validation_layers.has_value();
  if (validation_layers_enabled) {
    config.required_extensions.emplace(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }

  auto create_info = vk::InstanceCreateInfo();
  create_info.pApplicationInfo = &app_info;
  create_info.enabledExtensionCount = config.required_extensions.size();

  const auto ext_c_str_view =
      config.required_extensions | ranges::views::transform([](auto &str) { return str.c_str(); });
  const auto ext_cstr = std::vector<const char *>(ext_c_str_view.begin(), ext_c_str_view.end());
  create_info.ppEnabledExtensionNames = ext_cstr.data();

  const auto layer_c_str_view = config.validation_layers.value()
      | ranges::views::transform([](auto &str) { return str.c_str(); });
  const auto layer_cstr =
      std::vector<const char *>(layer_c_str_view.begin(), layer_c_str_view.end());
  if (validation_layers_enabled) {
    create_info.enabledLayerCount = layer_cstr.size();
    create_info.ppEnabledLayerNames = layer_cstr.data();
    create_info.pNext = &debug_create_info;
  } else {
    create_info.ppEnabledLayerNames = nullptr;
    create_info.pNext = nullptr;
    create_info.enabledLayerCount = 0;
  }

  auto result = instance_create_result();
  result.instance = vk::createInstanceUnique(create_info);

  if (validation_layers_enabled) {
    result.debug_messenger = result.instance->createDebugUtilsMessengerEXTUnique(
        debug_create_info, nullptr,
        vk::DispatchLoaderDynamic{*result.instance, vkGetInstanceProcAddr});
  }
  return result;
}

std::ostream &operator<<(std::ostream &os, const pf::vulkan::version &self) {
  fmt::format_to(std::ostream_iterator<char>(os), "{}.{}.{}", self.major, self.minor, self.patch);
  return os;
}

pf::vulkan::debug_callback_data
pf::vulkan::debug_callback_data::from_vk(const VkDebugUtilsMessengerCallbackDataEXT &src) {
  return {src.messageIdNumber, std::string(src.pMessage)};
}

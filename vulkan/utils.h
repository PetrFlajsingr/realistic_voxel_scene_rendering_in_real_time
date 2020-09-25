//
// Created by petr on 9/25/20.
//

#ifndef REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_UTILS_H
#define REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_UTILS_H

#include <optional>
#include <string>
#include <unordered_set>
#include <vulkan/vulkan.hpp>

namespace pf::vulkan {

struct version {
  uint32_t major;
  uint32_t minor;
  uint32_t patch;

  friend std::ostream &operator<<(std::ostream &os, const version &self);
};

struct engine_info {
  std::string name;
  version engine_version;
};

struct debug_callback_data {
  int32_t message_id;
  std::string message;
  static debug_callback_data from_vk(const VkDebugUtilsMessengerCallbackDataEXT &src);
};

using vulkan_debug_callback =
    std::function<bool(void *, debug_callback_data, vk::DebugUtilsMessageSeverityFlagBitsEXT,
                       vk::DebugUtilsMessageTypeFlagsEXT)>;
struct debug_config {
  void *user_data;
  vulkan_debug_callback callback;
};

struct instance_config {
  std::string app_name;
  version app_version;
  version vk_version;
  engine_info e_info;

  std::unordered_set<std::string> required_extensions;
  std::optional<std::unordered_set<std::string>> validation_layers;

  debug_config debug;
};

using DynamicUniqueDebugUtilsMessengerEXT =
    vk::UniqueHandle<vk::DebugUtilsMessengerEXT, vk::DispatchLoaderDynamic>;

struct instance_create_result {
  vk::UniqueInstance instance;
  std::optional<DynamicUniqueDebugUtilsMessengerEXT> debug_messenger = std::nullopt;
};

namespace details {
struct debug_config {
  void *callback_data;
  PFN_vkDebugUtilsMessengerCallbackEXT callback;
};
}// namespace details

uint32_t version_to_uint32(const version &version);
instance_create_result create_instance(instance_config config,
                                       const details::debug_config &debug_config);

}// namespace pf::vulkan

#endif//REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_UTILS_H

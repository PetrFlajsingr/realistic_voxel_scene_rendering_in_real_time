//
// Created by petr on 9/25/20.
//

#ifndef REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_VULKAN_H
#define REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_VULKAN_H

#include "../concepts/window.h"
#include "default_device_suitability_scorer.h"
#include "utils.h"
#include <vulkan/vulkan.hpp>

namespace pf::vulkan {

template<window::window Window, device_suitability_scorer DeviceScorer>
class vulkan {
 public:
  explicit vulkan(const instance_config &i_config, std::shared_ptr<Window> window,
                  DeviceScorer &&device_scorer)
      : window(std::move(window)), device_scorer(device_scorer) {
    user_data = i_config.debug.user_data;
    debug_callback = i_config.debug.callback;
    auto create_data = create_instance(i_config, {this, c_vulkan_debug_callback});
    instance = std::move(create_data.instance);
    debug_messenger = std::move(create_data.debug_messenger);
    surface = std::move(this->window->create_vulkan_surface(*instance));
  }

 private:
  static VKAPI_ATTR VkBool32 VKAPI_CALL c_vulkan_debug_callback(
      VkDebugUtilsMessageSeverityFlagBitsEXT severity,
      VkDebugUtilsMessageTypeFlagsEXT msg_type_flags,
      const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *user_data) {
    auto self = reinterpret_cast<vulkan *>(user_data);
    return self->debug_callback(self->user_data, debug_callback_data::from_vk(*pCallbackData),
                                static_cast<vk::DebugUtilsMessageSeverityFlagBitsEXT>(severity),
                                static_cast<vk::DebugUtilsMessageTypeFlagsEXT>(msg_type_flags))
        ? VK_TRUE
        : VK_FALSE;
  }

  vulkan_debug_callback debug_callback;
  void *user_data;
  vk::UniqueInstance instance;
  vk::UniqueSurfaceKHR surface;
  std::optional<DynamicUniqueDebugUtilsMessengerEXT> debug_messenger;
  std::shared_ptr<Window> window;
  device_suitability_scorer_fnc device_scorer;
};

}// namespace pf::vulkan
#endif//REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_VULKAN_H

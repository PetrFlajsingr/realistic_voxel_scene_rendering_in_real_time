//
// Created by petr on 9/25/20.
//

#ifndef REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_VULKAN_INTERFACE_H
#define REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_VULKAN_INTERFACE_H

#include "../concepts/window.h"
#include "default_device_suitability_scorer.h"
#include "utils.h"
#include "vulkan_exception.h"
#include <range/v3/action.hpp>
#include <range/v3/view.hpp>
#include <vulkan/vulkan.hpp>

namespace pf::vulkan {

template<window::window Window, device_suitability_scorer DeviceScorer>
class vulkan_interface {
 public:
  explicit vulkan_interface(const instance_config &i_config, std::shared_ptr<Window> window,
                            DeviceScorer &&device_scorer)
      : window(std::move(window)), device_scorer(device_scorer) {
    user_data = i_config.debug.user_data;
    debug_callback = i_config.debug.callback;
    auto create_data = create_instance(i_config, {this, c_vulkan_debug_callback});
    instance = std::move(create_data.instance);
    debug_messenger = std::move(create_data.debug_messenger);
    surface = std::move(this->window->create_vulkan_surface(*instance));
    selected_device = select_physical_device();
  }

  // proper destruction order
  ~vulkan_interface() {
    if (debug_messenger.has_value()) {
      debug_messenger->release();
    }
    surface.release();
    instance.release();
  }

 private:
  static VKAPI_ATTR VkBool32 VKAPI_CALL c_vulkan_debug_callback(
      VkDebugUtilsMessageSeverityFlagBitsEXT severity,
      VkDebugUtilsMessageTypeFlagsEXT msg_type_flags,
      const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *user_data) {
    auto self = reinterpret_cast<vulkan_interface *>(user_data);
    return self->debug_callback(self->user_data, debug_callback_data::from_vk(*pCallbackData),
                                static_cast<vk::DebugUtilsMessageSeverityFlagBitsEXT>(severity),
                                static_cast<vk::DebugUtilsMessageTypeFlagsEXT>(msg_type_flags))
        ? VK_TRUE
        : VK_FALSE;
  }

  vk::PhysicalDevice select_physical_device() {
    using namespace ranges;
    const auto physical_devices = instance->enumeratePhysicalDevices();
    const auto suitable_devices = physical_devices | views::transform([this](const auto &device) {
                                    return std::make_pair(device_scorer(device), device);
                                  })
        | views::filter([](const auto &scored_device) { return scored_device.first.has_value(); })
        | to_vector | actions::sort([](const auto &dev_a, const auto &dev_b) {
                                    return dev_a.first.value() > dev_b.first.value();
                                  });
    if (suitable_devices.empty()) {
      throw vulkan_exception("No suitable physical device was found.");
    }
    return suitable_devices.front().second;
  }

  vulkan_debug_callback debug_callback;
  void *user_data;
  std::optional<DynamicUniqueDebugUtilsMessengerEXT> debug_messenger = std::nullopt;
  vk::UniqueInstance instance;
  vk::UniqueSurfaceKHR surface;
  std::shared_ptr<Window> window;
  vk::PhysicalDevice selected_device;
  device_suitability_scorer_fnc device_scorer;
};

}// namespace pf::vulkan
#endif//REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_VULKAN_INTERFACE_H

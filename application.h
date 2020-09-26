//
// Created by petr on 9/24/20.
//

#ifndef REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_APPLICATION_H
#define REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_APPLICATION_H

#include "concepts/window.h"
#include "exceptions/stacktrace_exception.h"
#include "vulkan/vulkan_interface.h"
#include <iostream>
#include <memory>
#include <spdlog/spdlog.h>

namespace pf {
struct application_settings {
  bool debug{};
  window::window_settings window_settings;
};

template<window::window Window>
class application {
 public:
  explicit application(const application_settings &settings)
      : logger(spdlog::get("application")),
        window(std::make_shared<Window>(settings.window_settings)) {
    logger->info("Creating application.");
    if (settings.debug) { logger->debug("Debug is active."); }

    logger->info("Initialising window.");
    if (auto init_res = window->init(); init_res.has_value()) {
      throw stacktrace_exception(fmt::format("Window creation failed: {}", init_res.value()));
    }
    logger->info("Window initialised\n{}", *window);

    auto extensions = window->required_vulkan_extensions();
    auto validation_layers = std::unordered_set<std::string>{"VK_LAYER_KHRONOS_validation"};
    const auto vulkan_config = vulkan::instance_config{
        .app_name = "Realistic voxel rendering in real time",
        .app_version = {0, 1, 0},
        .vk_version = {1, 2, 0},
        .e_info = vulkan::engine_info{.name = "<unnamed>", .engine_version = {0, 1, 0}},
        .required_extensions = std::move(extensions),
        .validation_layers = std::move(validation_layers),
        .debug{.user_data = this, .callback = debug_callback}};

    vul = std::make_unique<vulkan::vulkan_interface<Window, vulkan::default_device_suitability_scorer>>(
        vulkan_config, window,
        vulkan::default_device_suitability_scorer(vulkan_config.required_extensions, {},
                                                  [](const auto &) { return 0; }));
  }

  void run() {

    window->main_loop();
    logger->info("Main loop ended.");
  }

 private:
  static bool debug_callback(void *user_data, vulkan::debug_callback_data data,
                             vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
                             vk::DebugUtilsMessageTypeFlagsEXT) {
    auto self = reinterpret_cast<application *>(user_data);
    auto log_level = spdlog::level::trace;
    switch (severity) {
      case vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose:
        log_level = spdlog::level::trace;
        break;
      case vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo: log_level = spdlog::level::info; break;
      case vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning:
        log_level = spdlog::level::warn;
        break;
      case vk::DebugUtilsMessageSeverityFlagBitsEXT::eError: log_level = spdlog::level::err; break;
    }
    // TODO: change to vulkan logger
    self->logger->log(log_level, "Validation layer: {} message id: {}", data.message,
                      data.message_id);
    return false;
  }

  std::shared_ptr<spdlog::logger> logger;
  std::shared_ptr<Window> window;
  std::unique_ptr<vulkan::vulkan_interface<Window, vulkan::default_device_suitability_scorer>> vul;
};
}// namespace pf
#endif//REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_APPLICATION_H

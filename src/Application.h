//
// Created by petr on 9/24/20.
//

#ifndef VOXEL_RENDER_APPLICATION_H
#define VOXEL_RENDER_APPLICATION_H

#include "concepts/Renderer.h"
#include "logging/loggers.h"
#include <iostream>
#include <memory>
#include <pf_common/exceptions/StackTraceException.h>
#include <pf_glfw_vulkan/concepts/Window.h>
#include <pf_glfw_vulkan/vulkan/types.h>
#include <range/v3/view.hpp>

namespace pf {
struct application_settings {
  bool debug{};
  ui::WindowSettings window_settings;
};

template<ui::Window Window, Renderer<Window> Renderer>
class Application {
 public:
  explicit Application(Renderer &&renderer, const application_settings &settings)
      : window(std::make_shared<Window>(settings.window_settings)), renderer(std::move(renderer)) {
    log(spdlog::level::info, APP_TAG, "Creating application.");
    if (settings.debug) { log(spdlog::level::debug, APP_TAG, "Debug is active."); }
    init_window();
  }

  void run() {
    window->mainLoop();
    renderer.stop();
    log(spdlog::level::info, APP_TAG, "Main loop ended.");
  }

 private:
  void init_window() {
    log(spdlog::level::info, APP_TAG, "Initialising window.");
    if (auto init_res = window->init(); init_res.has_value()) {
      throw StackTraceException("Window creation failed: {}", init_res.value());
    }
    logFmt(spdlog::level::info, APP_TAG, "Window initialised\n{}", *window);
    renderer.init(*window);
    window->setMainLoopCallback([this] { renderer.render(); });
  }

  std::shared_ptr<Window> window;
  Renderer renderer;
};
}// namespace pf
#endif//VOXEL_RENDER_APPLICATION_H

/**
 * @file Application.h
 * @brief And object to prepare a window and run a renderer.
 * @author Petr Flajšingr
 * @date 24.9.20
 */

#ifndef VOXEL_RENDER_APPLICATION_H
#define VOXEL_RENDER_APPLICATION_H

#include "concepts/Renderer.h"
#include "logging/loggers.h"
#include <iostream>
#include <memory>
#include <pf_common/exceptions/StackTraceException.h>
#include <pf_glfw_vulkan/ui/Window.h>
#include <pf_glfw_vulkan/vulkan/types.h>
#include <range/v3/view.hpp>

namespace pf {
/**
 * @brief Init settings for application.
 */
struct ApplicationSettings {
  bool debug{};
  ui::WindowSettings window_settings;
};

/**
 * @brief An object which sets up the given window and runs the given renderer.
 * @tparam Renderer type of renderer
 * @tparam Window type of window
 */
template<std::derived_from<ui::Window> Window, Renderer Renderer>
class Application {
 public:
  /**
   * Construct Application. Initialises the window and render loop.
   * @param renderer renderer instance
   * @param settings app settings
   */
  explicit Application(Renderer &&renderer, const ApplicationSettings &settings)
      : window(std::make_shared<Window>(settings.window_settings)), renderer(std::move(renderer)) {
    logi(APP_TAG, "Creating application.");
    if (settings.debug) { logd(APP_TAG, "Debug is active."); }
    init_window();
  }

  /**
   * Run window mainloop.
   */
  void run() {
    window->run();
    renderer.stop();
    logi(APP_TAG, "Main loop ended.");
  }

 private:
  void init_window() {
    logi(APP_TAG, "Initialising window.");
    if (auto init_res = window->init(); init_res.has_value()) {
      throw StackTraceException("Window creation failed: {}", init_res.value());
    }
    logi(APP_TAG, "Window initialised");
    renderer.init(window);
    window->setMainLoopCallback([this] { renderer.render(); });
  }

  std::shared_ptr<Window> window;
  Renderer renderer;
};
}// namespace pf
#endif//VOXEL_RENDER_APPLICATION_H

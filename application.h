//
// Created by petr on 9/24/20.
//

#ifndef REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_APPLICATION_H
#define REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_APPLICATION_H

#include "concepts/window.h"
#include "exceptions/stacktrace_exception.h"
#include <iostream>
#include <memory>
#include <spdlog/spdlog.h>

namespace pf {
struct application_settings {
  bool debug;
  window::window_settings window_settings;
};

template<window::window Window>
class application {
 public:
  explicit application(const application_settings &settings)
      : logger(spdlog::get("application")), window(settings.window_settings) {
    logger->info("Creating application.");
    if (settings.debug) { logger->debug("Debug is active."); }
  }

  void run() {
    logger->info("Initialising window.");
    if (auto init_res = window.init(); init_res.has_value()) {
      throw stacktrace_exception(fmt::format("Window creation failed: {}", init_res.value()));
    }
    logger->info("Window initialised\n{}", window);

    window.main_loop();
    logger->info("Main loop ended.");
  }

 private:
  std::shared_ptr<spdlog::logger> logger;
  Window window;
};
}// namespace pf
#endif//REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_APPLICATION_H

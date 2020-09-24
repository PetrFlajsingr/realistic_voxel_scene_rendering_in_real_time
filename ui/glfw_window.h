//
// Created by petr on 9/24/20.
//

#ifndef REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_GLFW_WINDOW_H
#define REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_GLFW_WINDOW_H

#include "../concepts/window.h"
#include <GLFW/glfw3.h>

class glfw_window final : public window::window_data {
 public:
  explicit glfw_window(const window::window_settings &settings);
  virtual ~glfw_window();
  [[nodiscard]] std::optional<std::string> init();

  void set_main_loop_callback(std::invocable auto &&callback) { main_loop_fnc = callback; }

  void main_loop();

  friend std::ostream &operator<<(std::ostream &os, const glfw_window &window);

 private:
  GLFWwindow *handle{};
  std::function<void()> main_loop_fnc = [] {};
};

static_assert(window::window<glfw_window>);

#endif//REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_GLFW_WINDOW_H

//
// Created by petr on 9/24/20.
//

#ifndef REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_GLFW_WINDOW_H
#define REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_GLFW_WINDOW_H

#include "../concepts/window.h"
#include "../coroutines/sequence.h"
#include "events/common.h"
#include "events/event_dispatch_impl.h"
#include "events/subscription.h"
#include <GLFW/glfw3.h>
#include <array>
#include <unordered_map>

namespace pf {
class glfw_window final : public window::window_data, public events::event_dispatch_impl {
 public:
  explicit glfw_window(const window::window_settings &settings);
  virtual ~glfw_window();
  [[nodiscard]] std::optional<std::string> init();

  void set_main_loop_callback(std::invocable auto &&callback) { main_loop_fnc = callback; }
  void main_loop();

 private:

  static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
  static void mouse_position_callback(GLFWwindow* window, double xpos, double ypos);
  static void mouse_wheel_callback(GLFWwindow* window, double xpos, double ypos);
  static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
  GLFWwindow *handle{};
  std::function<void()> main_loop_fnc = [] {};
  std::pair<double, double> cursor_position{0, 0};

};

std::optional<events::mouse_button> glfw_button_to_events(int button);
std::optional<events::key_event_type> glfw_key_event_to_events(int key_event);

std::ostream &operator<<(std::ostream &os, const glfw_window &window);
static_assert(window::window<glfw_window>);
}// namespace pf
#endif//REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_GLFW_WINDOW_H

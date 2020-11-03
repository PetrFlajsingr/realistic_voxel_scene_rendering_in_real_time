//
// Created by petr on 9/24/20.
//

#ifndef VOXEL_RENDER_GLFWWINDOW_H
#define VOXEL_RENDER_GLFWWINDOW_H

#include "concepts/Window.h"
#include <pf_common/coroutines/Sequence.h>
#include "events/EventDispatchImpl.h"
#include <pf_common/Subscription.h>
#include "events/common.h"
#include <GLFW/glfw3.h>
#include <array>
#include <concepts>
#include <unordered_map>

namespace pf::ui {
class GlfwWindow final : public WindowData, public events::EventDispatchImpl {
 public:
  explicit GlfwWindow(const WindowSettings &settings);
  virtual ~GlfwWindow();
  [[nodiscard]] std::optional<std::string> init();

  void setMainLoopCallback(std::invocable auto &&callback) { mainLoopFnc = callback; }
  void setResizeCallback(std::invocable<Resolution> auto &&callback) { resizeFnc = callback; }
  void mainLoop();

  vk::UniqueSurfaceKHR createVulkanSurface(const vk::Instance &instance);
  static std::unordered_set<std::string> requiredVulkanExtensions();

  GLFWwindow *getHandle() const;

 private:
  static void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods);
  static void mousePositionCallback(GLFWwindow *window, double xpos, double ypos);
  static void mouseWheelCallback(GLFWwindow *window, double xpos, double ypos);
  static void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
  static void resizeCallback(GLFWwindow *window, int width, int height);
  GLFWwindow *handle{};
  std::function<void()> mainLoopFnc = [] {};
  std::function<void(Resolution)> resizeFnc = [](Resolution) {};
  std::pair<double, double> cursorPosition{0, 0};
};

std::optional<events::MouseButton> glfwButtonToEvents(int button);
std::optional<events::KeyEventType> glfwKeyEventToEvents(int key_event);

std::ostream &operator<<(std::ostream &os, const GlfwWindow &window);
static_assert(Window<GlfwWindow>);
}// namespace pf::ui
#endif//VOXEL_RENDER_GLFWWINDOW_H

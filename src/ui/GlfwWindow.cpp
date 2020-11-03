//
// Created by petr on 9/24/20.
//

#include "GlfwWindow.h"
#include <pf_common/exceptions/StackTraceException.h>
#include <fmt/format.h>
#include <magic_enum.hpp>

namespace pf::ui {
GlfwWindow::GlfwWindow(const WindowSettings &settings) : WindowData(settings) {}

std::optional<std::string> GlfwWindow::init() {
  if (glfwInit() == GLFW_FALSE) { return "glfwInit failed"; }
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

  handle = glfwCreateWindow(resolution.width, resolution.height, title.c_str(), nullptr, nullptr);
  glfwSetWindowUserPointer(handle, this);
  glfwSetFramebufferSizeCallback(handle, resizeCallback);
  glfwSetMouseButtonCallback(handle, mouseButtonCallback);
  glfwSetCursorPosCallback(handle, mousePositionCallback);
  glfwSetScrollCallback(handle, mouseWheelCallback);
  glfwSetKeyCallback(handle, keyCallback);
  return std::nullopt;
}

void GlfwWindow::mainLoop() {
  while (!glfwWindowShouldClose(handle)) {
    glfwPollEvents();
    onFrame();
    mainLoopFnc();
  }
}

std::ostream &operator<<(std::ostream &os, const GlfwWindow &window) {
  os << fmt::format("GLFW window\n\ttitle: {}\n\tresolution: {}\n\tmode: {}", window.getTitle(),
                    window.getResolution(), magic_enum::enum_name(window.getMode()));
  return os;
}

GlfwWindow::~GlfwWindow() {
  glfwDestroyWindow(handle);
  glfwTerminate();
}
void GlfwWindow::mouseButtonCallback(GLFWwindow *window, int button, int action, int) {
  auto self = reinterpret_cast<GlfwWindow *>(glfwGetWindowUserPointer(window));
  const auto mouseButton = glfwButtonToEvents(button);
  if (!mouseButton.has_value()) { return; }
  const auto eventType =
      action == GLFW_PRESS ? events::MouseEventType::Down : events::MouseEventType::Up;
  auto cursorPosition = std::pair<double, double>();
  glfwGetCursorPos(self->handle, &cursorPosition.first, &cursorPosition.second);

  self->notifyMouse(eventType, mouseButton.value(), cursorPosition, std::make_pair(0.0, 0.0));
}

void GlfwWindow::mousePositionCallback(GLFWwindow *window, double xpos, double ypos) {
  auto self = reinterpret_cast<GlfwWindow *>(glfwGetWindowUserPointer(window));

  const auto cursorPosition = std::pair<double, double>(xpos, ypos);
  self->notifyMouse(events::MouseEventType::Move, events::MouseButton::None, cursorPosition,
                    std::make_pair(cursorPosition.first - self->cursorPosition.first,
                                   cursorPosition.second - self->cursorPosition.second));
  self->cursorPosition = cursorPosition;
}

void GlfwWindow::mouseWheelCallback(GLFWwindow *window, double xpos, double ypos) {
  auto self = reinterpret_cast<GlfwWindow *>(glfwGetWindowUserPointer(window));

  const auto cursorPosition = std::pair<double, double>(xpos, ypos);
  self->notifyMouse(events::MouseEventType::Wheel, events::MouseButton::None, cursorPosition,
                    std::make_pair(xpos, ypos));
  self->cursorPosition = cursorPosition;
}

void GlfwWindow::keyCallback(GLFWwindow *window, int key, int, int action, int) {
  auto self = reinterpret_cast<GlfwWindow *>(glfwGetWindowUserPointer(window));
  const auto eventType = glfwKeyEventToEvents(action);
  if (!eventType.has_value()) { return; }
  const auto keyChar = static_cast<char>(key);
  self->notifyKey(eventType.value(), keyChar);
}
vk::UniqueSurfaceKHR GlfwWindow::createVulkanSurface(const vk::Instance &instance) {
  auto surface = VkSurfaceKHR{};
  if (const auto res = glfwCreateWindowSurface(instance, handle, nullptr, &surface);
      res != VK_SUCCESS) {
    const auto resEnum = static_cast<vk::Result>(res);
    throw StackTraceException::fmt("Window surface creation failed: {} {}",
                                   magic_enum::enum_name(resEnum), vk::to_string(resEnum));
  }
  auto surfaceDeleter =
      vk::ObjectDestroy<vk::Instance, VULKAN_HPP_DEFAULT_DISPATCHER_TYPE>(instance);
  return vk::UniqueSurfaceKHR(surface, surfaceDeleter);
}
std::unordered_set<std::string> GlfwWindow::requiredVulkanExtensions() {
  auto extensionCount = uint32_t{};
  const char **extensions;
  extensions = glfwGetRequiredInstanceExtensions(&extensionCount);
  return std::unordered_set<std::string>(extensions, extensions + extensionCount);
}

void GlfwWindow::resizeCallback(GLFWwindow *window, int width, int height) {
  auto self = reinterpret_cast<GlfwWindow *>(glfwGetWindowUserPointer(window));
  self->resizeFnc({static_cast<std::size_t>(width), static_cast<std::size_t>(height)});
}
GLFWwindow *GlfwWindow::getHandle() const { return handle; }

std::optional<events::MouseButton> glfwButtonToEvents(int button) {
  switch (button) {
    case GLFW_MOUSE_BUTTON_LEFT: return events::MouseButton::Left;
    case GLFW_MOUSE_BUTTON_MIDDLE: return events::MouseButton::Middle;
    case GLFW_MOUSE_BUTTON_RIGHT: return events::MouseButton::Right;
    default: return std::nullopt;
  }
}
std::optional<events::KeyEventType> glfwKeyEventToEvents(int key_event) {
  switch (key_event) {
    case GLFW_PRESS: return events::KeyEventType::Pressed;
    case GLFW_REPEAT: return events::KeyEventType::Repeat;
    case GLFW_RELEASE: return events::KeyEventType::Up;
    default: return std::nullopt;
  }
}

}// namespace pf::ui
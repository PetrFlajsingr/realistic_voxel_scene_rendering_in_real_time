//
// Created by petr on 9/24/20.
//

#include "glfw_window.h"
#include <fmt/format.h>
#include <magic_enum.hpp>

pf::glfw_window::glfw_window(const window::window_settings &settings)
    : window::window_data(settings) {}

std::optional<std::string> pf::glfw_window::init() {
  if (glfwInit() == GLFW_FALSE) { return "glfwInit failed"; }
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

  handle = glfwCreateWindow(resolution.width, resolution.height, title.c_str(), nullptr, nullptr);
  glfwSetWindowUserPointer(handle, this);
  // TODO: resizing
  //glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
  glfwSetMouseButtonCallback(handle, mouse_button_callback);
  glfwSetCursorPosCallback(handle, mouse_position_callback);
  glfwSetScrollCallback(handle, mouse_wheel_callback);
  glfwSetKeyCallback(handle, key_callback);
  return std::nullopt;
}

void pf::glfw_window::main_loop() {
  while (!glfwWindowShouldClose(handle)) {
    glfwPollEvents();
    on_frame();
    main_loop_fnc();
  }
}

std::ostream &pf::operator<<(std::ostream &os, const pf::glfw_window &window) {
  os << fmt::format("GLFW window\n\ttitle: {}\n\tresolution: {}\n\tmode: {}", window.get_title(),
                    window.get_resolution(), magic_enum::enum_name(window.get_mode()));
  return os;
}

pf::glfw_window::~glfw_window() {
  glfwDestroyWindow(handle);
  glfwTerminate();
}
void pf::glfw_window::mouse_button_callback(GLFWwindow *window, int button, int action, int) {
  auto self = reinterpret_cast<glfw_window *>(glfwGetWindowUserPointer(window));
  const auto mouse_button = glfw_button_to_events(button);
  if (!mouse_button.has_value()) { return; }
  const auto event_type =
      action == GLFW_PRESS ? events::mouse_event_type::down : events::mouse_event_type::up;
  auto cursor_position = std::pair<double, double>();
  glfwGetCursorPos(self->handle, &cursor_position.first, &cursor_position.second);

  self->notify_mouse(event_type, mouse_button.value(), cursor_position, std::make_pair(0.0, 0.0));
}

void pf::glfw_window::mouse_position_callback(GLFWwindow *window, double xpos, double ypos) {
  auto self = reinterpret_cast<glfw_window *>(glfwGetWindowUserPointer(window));

  const auto cursor_position = std::pair<double, double>(xpos, ypos);
  self->notify_mouse(events::mouse_event_type::move, events::mouse_button::none, cursor_position,
                     std::make_pair(cursor_position.first - self->cursor_position.first,
                                    cursor_position.second - self->cursor_position.second));
  self->cursor_position = cursor_position;
}

void pf::glfw_window::mouse_wheel_callback(GLFWwindow *window, double xpos, double ypos) {
  auto self = reinterpret_cast<glfw_window *>(glfwGetWindowUserPointer(window));

  const auto cursor_position = std::pair<double, double>(xpos, ypos);
  self->notify_mouse(events::mouse_event_type::wheel, events::mouse_button::none, cursor_position,
                     std::make_pair(xpos, ypos));
  self->cursor_position = cursor_position;
}

void pf::glfw_window::key_callback(GLFWwindow *window, int key, int, int action, int) {
  auto self = reinterpret_cast<glfw_window *>(glfwGetWindowUserPointer(window));
  const auto event_type = glfw_key_event_to_events(action);
  if (!event_type.has_value()) { return; }
  const auto key_char = static_cast<char>(key);
  self->notify_key(event_type.value(), key_char);
}

std::optional<pf::events::mouse_button> pf::glfw_button_to_events(int button) {
  switch (button) {
    case GLFW_MOUSE_BUTTON_LEFT: return events::mouse_button::left;
    case GLFW_MOUSE_BUTTON_MIDDLE: return events::mouse_button::middle;
    case GLFW_MOUSE_BUTTON_RIGHT: return events::mouse_button::right;
    default: return std::nullopt;
  }
}
std::optional<pf::events::key_event_type> pf::glfw_key_event_to_events(int key_event) {
  switch (key_event) {
    case GLFW_PRESS: return events::key_event_type::pressed;
    case GLFW_REPEAT: return events::key_event_type::repeat;
    case GLFW_RELEASE: return events::key_event_type::up;
    default: return std::nullopt;
  }
}

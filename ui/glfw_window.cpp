//
// Created by petr on 9/24/20.
//

#include "glfw_window.h"
#include <fmt/format.h>
#include <magic_enum.hpp>

glfw_window::glfw_window(const window::window_settings &settings) : window::window_data(settings) {}

std::optional<std::string> glfw_window::init() {
  if (glfwInit() == GLFW_FALSE) { return "glfwInit failed"; }
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

  handle = glfwCreateWindow(resolution.width, resolution.height, title.c_str(), nullptr, nullptr);
  glfwSetWindowUserPointer(handle, this);
  //glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
  //glfwSetMouseButtonCallback(window, mouseButtonCallback);
  //glfwSetCursorPosCallback(window, mousePositionCallback);
  return std::nullopt;
}

void glfw_window::main_loop() {
  while (!glfwWindowShouldClose(handle)) {
    glfwPollEvents();
    main_loop_fnc();
  }
}

std::ostream &operator<<(std::ostream &os, const glfw_window &window) {
  os << fmt::format("GLFW window\n\ttitle: {}\n\tresolution: {}\n\tmode: {}", window.title,
                    window.resolution, magic_enum::enum_name(window.mode));
  return os;
}
glfw_window::~glfw_window() {
  glfwDestroyWindow(handle);
  glfwTerminate();
}

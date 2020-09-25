//
// Created by petr on 9/25/20.
//

#include "vulkan_exception.h"
#include "fmt/format.h"

vulkan_exception::vulkan_exception(const std::string_view &message)
    : stacktrace_exception(message) {}

vulkan_exception vulkan_exception::fmt(std::string_view fmt, auto &&... args) {
  return vulkan_exception(fmt::format(fmt, args...));
}

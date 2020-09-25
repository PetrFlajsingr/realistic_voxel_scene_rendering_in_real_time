//
// Created by petr on 9/23/20.
//

#ifndef REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_STACKTRACE_EXCEPTION_H
#define REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_STACKTRACE_EXCEPTION_H

#include "fmt/format.h"
#include <exception>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

class stacktrace_exception : public std::exception {
 public:
  explicit stacktrace_exception(std::string_view message);
  static stacktrace_exception fmt(std::string_view fmt, auto &&... args) {
    return stacktrace_exception(fmt::format(fmt, args...));
  }

  [[nodiscard]] const char *what() const noexcept override;

 private:
  std::string what_stacktrace;
};

struct trace_data {
  std::string filename;
  std::string location;
  uint32_t line_n;
  trace_data(std::string filename, std::string location, uint32_t line_n);
};

class invalid_argument_exception : public stacktrace_exception {
 public:
  explicit invalid_argument_exception(const std::string_view &message);
};

std::vector<trace_data> get_trace(std::size_t skip_n = 0);

#endif//REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_STACKTRACE_EXCEPTION_H

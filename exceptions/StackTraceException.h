//
// Created by petr on 9/23/20.
//

#ifndef REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_STACKTRACEEXCEPTION_H
#define REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_STACKTRACEEXCEPTION_H

#include "fmt/format.h"
#include <exception>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

class StackTraceException : public std::exception {
 public:
  explicit StackTraceException(std::string_view message);
  static StackTraceException fmt(std::string_view fmt, auto &&... args) {
    return StackTraceException(fmt::format(fmt, args...));
  }

  [[nodiscard]] const char *what() const noexcept override;

 private:
  std::string whatStacktrace;
};

struct TraceData {
  std::string filename;
  std::string location;
  uint32_t lineN;
  TraceData(std::string filename, std::string location, uint32_t lineN);
};

class InvalidArgumentException : public StackTraceException {
 public:
  explicit InvalidArgumentException(std::string_view message);
};

class NotImplementedException : public StackTraceException {
 public:
  explicit NotImplementedException(std::string_view message);
};

std::vector<TraceData> getTrace(std::size_t skipN = 0);

#endif//REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_STACKTRACEEXCEPTION_H

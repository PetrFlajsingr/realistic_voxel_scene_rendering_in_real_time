//
// Created by petr on 9/25/20.
//

#ifndef REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_VULKAN_EXCEPTION_H
#define REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_VULKAN_EXCEPTION_H

#include "../exceptions/stacktrace_exception.h"

class vulkan_exception : public stacktrace_exception {
 public:
  explicit vulkan_exception(const std::string_view &message);
  static vulkan_exception fmt(std::string_view fmt, auto &&... args);
};

#endif//REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_VULKAN_EXCEPTION_H

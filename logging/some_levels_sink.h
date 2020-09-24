//
// Created by petr on 9/24/20.
//

#ifndef REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_SOME_LEVELS_SINK_H
#define REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_SOME_LEVELS_SINK_H

#include "../meta.h"
#include "../utils.h"
#include "spdlog/sinks/sink.h"

template<std::derived_from<spdlog::sinks::sink> Sink>
class some_levels_sink : public Sink {
 public:
  template<typename... Args>
  explicit some_levels_sink(const iterable auto &levels, Args &&... args)
      : Sink(std::forward<Args>(args)...), allowed_levels(std::begin(levels), std::end(levels)) {}

  void log(const spdlog::details::log_msg &msg) override {
    if (is_one_of(msg.level, allowed_levels)) { Sink::log(msg); }
  }

  void set_allowed(const iterable auto &levels) {
    allowed_levels.clear();
    allowed_levels.insert(allowed_levels.begin(), std::begin(levels), std::end(levels));
  }

 private:
  std::vector<spdlog::level::level_enum> allowed_levels;
};

#endif//REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_SOME_LEVELS_SINK_H

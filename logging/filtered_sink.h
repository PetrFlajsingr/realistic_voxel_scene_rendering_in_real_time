//
// Created by petr on 9/23/20.
//

#ifndef REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_FILTERED_SINK_H
#define REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_FILTERED_SINK_H

#include "spdlog/sinks/sink.h"

template<std::derived_from<spdlog::sinks::sink> Sink>
class filtered_sink : public Sink {
 public:
  using filter_fnc = std::function<bool(spdlog::level::level_enum, spdlog::string_view_t)>;
  void log(const spdlog::details::log_msg &msg) override {
    if (filter(msg.level, msg.payload)) { Sink::log(msg); }
  }

  template<std::invocable<spdlog::level::level_enum, spdlog::string_view_t> F>
  void set_filter(F &&f) {
    filter = f;
  }

 private:
  filter_fnc filter = [](spdlog::level::level_enum, spdlog::string_view_t) { return true; };
};

#endif//REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_FILTERED_SINK_H

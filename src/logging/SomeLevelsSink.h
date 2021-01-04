//
// Created by petr on 9/24/20.
//

#ifndef VOXEL_RENDER_SOMELEVELSSINK_H
#define VOXEL_RENDER_SOMELEVELSSINK_H

#include <pf_common/algorithms.h>
#include <pf_common/concepts/Iterable.h>
#include <spdlog/sinks/sink.h>

template<std::derived_from<spdlog::sinks::sink> Sink>
class SomeLevelsSink : public Sink {
 public:
  template<typename... Args>
  explicit SomeLevelsSink(const pf::Iterable auto &levels, Args &&...args)
      : Sink(std::forward<Args>(args)...), allowed_levels(std::begin(levels), std::end(levels)) {}

  void log(const spdlog::details::log_msg &msg) override {
    if (pf::isIn(msg.level, allowed_levels)) { Sink::log(msg); }
  }

  void set_allowed(const pf::Iterable auto &levels) {
    allowed_levels.clear();
    allowed_levels.insert(allowed_levels.begin(), std::begin(levels), std::end(levels));
  }

 private:
  std::vector<spdlog::level::level_enum> allowed_levels;
};

#endif//VOXEL_RENDER_SOMELEVELSSINK_H

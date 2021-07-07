/**
 * @file SomeLevelsSink.h
 * @brief An spdlog sink which lets only logs of certain level pass.
 * @author Petr Flajšingr
 * @date 24.9.20
 */

#ifndef VOXEL_RENDER_SOMELEVELSSINK_H
#define VOXEL_RENDER_SOMELEVELSSINK_H

#include <pf_common/algorithms.h>
#include <ranges>
#include <spdlog/sinks/sink.h>

/**
 * @brief A CRTP decorator for spdlog sinks. Filters messages based on log level and sends the ones that fulfil the predicate to Sink.
 * @tparam Sink sink for data which pass
 */
template<std::derived_from<spdlog::sinks::sink> Sink>
class SomeLevelsSink : public Sink {
 public:
  /**
   * Construct SomeLevelsSink.
   * @tparam Args argument types to pass to Sink's constructor
   * @param levels allowed levels
   * @param args arguments to pass to Sink's constructor
   */
  template<typename... Args>
  explicit SomeLevelsSink(const std::ranges::range auto &levels, Args &&...args) requires(
      std::same_as<spdlog::level::level_enum, std::ranges::range_value_t<decltype(levels)>>)
      : Sink(std::forward<Args>(args)...), allowed_levels(std::begin(levels), std::end(levels)) {}

  void log(const spdlog::details::log_msg &msg) override {
    if (pf::isIn(msg.level, allowed_levels)) { Sink::log(msg); }
  }

  /**
   * Set new allowed levels.
   * @param levels log levels which pass into Sink
   */
  void set_allowed(const std::ranges::range auto &levels) requires(
      std::same_as<spdlog::level::level_enum, std::ranges::range_value_t<decltype(levels)>>) {
    allowed_levels.clear();
    allowed_levels.insert(allowed_levels.begin(), std::begin(levels), std::end(levels));
  }

 private : std::vector<spdlog::level::level_enum> allowed_levels;
};

#endif//VOXEL_RENDER_SOMELEVELSSINK_H

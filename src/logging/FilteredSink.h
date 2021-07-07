/**
 * @file FilteredSink.h
 * @brief An spdlog sink which filters messages.
 * @author Petr Flajšingr
 * @date 23.9.20
 */

#ifndef VOXEL_RENDER_FILTERED_SINK_H
#define VOXEL_RENDER_FILTERED_SINK_H

#include <spdlog/sinks/sink.h>

/**
 * @brief A CRTP decorator for spdlog sinks. Filters messages and sends the ones that fulfil a predicate to Sink.
 * @tparam Sink sink for data which pass
 */
template<std::derived_from<spdlog::sinks::sink> Sink>
class FilteredSink : public Sink {
 public:
  using FilterFnc = std::function<bool(spdlog::level::level_enum, spdlog::string_view_t)>;
  void log(const spdlog::details::log_msg &msg) override {
    if (filter(msg.level, msg.payload)) { Sink::log(msg); }
  }

  /**
   * Set filter for logs to be passed to Sink.
   * @param f filtering function
   */
  void setFilter(std::invocable<spdlog::level::level_enum, spdlog::string_view_t> auto &&f) requires(
      std::convertible_to<bool, std::invoke_result_t<decltype(f), spdlog::level::level_enum, spdlog::string_view_t>>) {
    filter = f;
  }

 private : FilterFnc filter = [](spdlog::level::level_enum, spdlog::string_view_t) { return true; };
};

#endif//VOXEL_RENDER_FILTERED_SINK_H

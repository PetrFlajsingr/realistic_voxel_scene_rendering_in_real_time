/**
 * @file CallbackSink.h
 * @brief An spdlog sink which calls a function on log.
 * @author Petr Flajšingr
 * @date 2.11.20
 */

#ifndef REALISTIC_VOXEL_RENDERING_LOGGING_CALLBACKSINK_H
#define REALISTIC_VOXEL_RENDERING_LOGGING_CALLBACKSINK_H

#include <spdlog/sinks/sink.h>

/**
 * @brief Calls a function on log with log string.
 */
class CallbackSink : public spdlog::sinks::sink {
 public:
  /**
   * Construct CallbackSink.
   * @param fnc callback for log string
   */
  explicit CallbackSink(std::invocable<std::string_view> auto fnc) : callback(fnc) {}
  void log(const spdlog::details::log_msg &msg) override;
  void flush() override;
  void set_pattern(const std::string &pattern) override;
  void set_formatter(std::unique_ptr<spdlog::formatter> sink_formatter) override;

 private:
  std::function<void(std::string_view)> callback;
  std::unique_ptr<spdlog::formatter> formatter = nullptr;
};

#endif//REALISTIC_VOXEL_RENDERING_LOGGING_CALLBACKSINK_H

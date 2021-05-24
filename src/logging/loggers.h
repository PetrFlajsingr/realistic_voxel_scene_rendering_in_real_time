//
// Created by petr on 9/23/20.
//

#ifndef VOXEL_RENDER_LOGGERS_H
#define VOXEL_RENDER_LOGGERS_H

#include <experimental/source_location>
#include <filesystem>
#include <optional>
#include <pf_common/ILogger.h>
#include <pf_common/Subscription.h>
#include <pf_common/coroutines/Sequence.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <vector>

struct GlobalLoggerSettings {
  bool verbose;
  bool console;
  bool debug;
  std::filesystem::path logDir;
};

class GlobalLoggerInterface : public pf::ILogger {
 public:
  explicit GlobalLoggerInterface(const std::string &loggerName);
  void log(pf::LogLevel level, std::string_view tag, std::string_view msg) override;
};

std::vector<std::shared_ptr<spdlog::sinks::sink>> createConsoleLogSinks(const GlobalLoggerSettings &settings,
                                                                        std::string_view tag);

void createLoggerForTag(const GlobalLoggerSettings &settings, std::string_view tag);

namespace pf {

namespace details {
inline std::unordered_map<int, std::function<void(std::string_view)>> logListeners;
inline std::unordered_map<int, std::function<void(std::string_view)>> logErrListeners;
inline std::optional<GlobalLoggerSettings> settings = std::nullopt;

inline cppcoro::generator<int> idGenerator = iota<int>();
}// namespace details

const auto GLOBAL_LOGGER_NAME = "pf";
const auto TAG_FORMAT = "[{}] {}";
const auto VK_TAG = "vulkan";
const auto APP_TAG = "app";
const auto MAIN_TAG = "main";
inline std::shared_ptr<spdlog::logger> globalLogger = nullptr;

void initGlobalLogger(const GlobalLoggerSettings &settings);

void log(spdlog::level::level_enum level, std::string_view tag, std::string_view msg, const auto &...args) {
  globalLogger->log(level, fmt::format(TAG_FORMAT, tag, fmt::format(msg, args...)));
};
void logt(std::string_view tag, std::string_view msg, const auto &...args) {
  log(spdlog::level::trace, tag, msg, args...);
}
void logi(std::string_view tag, std::string_view msg, const auto &...args) {
  log(spdlog::level::info, tag, msg, args...);
}
void logd(std::string_view tag, std::string_view msg, const auto &...args) {
  log(spdlog::level::debug, tag, msg, args...);
}
void logw(std::string_view tag, std::string_view msg, const auto &...args) {
  log(spdlog::level::warn, tag, msg, args...);
}
void logc(std::string_view tag, std::string_view msg, const auto &...args) {
  log(spdlog::level::critical, tag, msg, args...);
}
void loge(std::string_view tag, std::string_view msg, const auto &...args) {
  log(spdlog::level::err, tag, msg, args...);
}

Subscription addLogListener(std::invocable<std::string_view> auto listener, bool err = false) {
  if (!details::settings.has_value()) { throw std::exception(); }
  const auto id = getNext(details::idGenerator);
  if (!err) {
    details::logListeners[id] = listener;
  } else {
    details::logErrListeners[id] = listener;
  }
  initGlobalLogger(*details::settings);
  return Subscription([id] {
    details::logListeners.erase(id);
    details::logErrListeners.erase(id);
  });
}

void logSrc(spdlog::level::level_enum level, std::string_view tag, std::string_view msg,
            std::experimental::source_location src_loc = std::experimental::source_location::current());

}// namespace pf

#endif//VOXEL_RENDER_LOGGERS_H

/**
 * @file loggers.h
 * @brief Global functions to manage loggers and for logging.
 * @author Petr Flajšingr
 * @date 23.9.20
 */

#ifndef VOXEL_RENDER_LOGGERS_H
#define VOXEL_RENDER_LOGGERS_H

#include <experimental/source_location>
#include <filesystem>
#include <optional>
#include <pf_common/ILogger.h>
#include <pf_common/Subscription.h>
#include <pf_common/coroutines/Sequence.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <vector>

/**
 * @brief Settings for creation of a global logger.
 */
struct GlobalLoggerSettings {
  bool verbose;                 /**< Allow for logging of trace */
  bool console;                 /**< Log to console */
  bool debug;                   /**< Allow logging of debug */
  std::filesystem::path logDir; /**< Directory to save log file into */
};

/**
 * @brief Implementation of logger interface for global logging.
 */
class GlobalLoggerInterface : public pf::ILogger {
 public:
  /**
   * Construct GlobalLoggerInterface.
   * @param loggerName name of the logger, has to be unique within the app
   */
  explicit GlobalLoggerInterface(const std::string &loggerName);
  void log(pf::LogLevel level, std::string_view tag, std::string_view msg) override;
};
/**
 * Create sinks for console log output.
 * @param settings settings
 * @param tag tag show in the log message
 * @return newly created sinks
 */
std::vector<std::shared_ptr<spdlog::sinks::sink>> createConsoleLogSinks(const GlobalLoggerSettings &settings,
                                                                        std::string_view tag);
/**
 * Create a new global logger with given tag.
 * @param settings settings
 * @param tag tag shown in the log message
 */
void createLoggerForTag(const GlobalLoggerSettings &settings, std::string_view tag);

namespace pf {

namespace details {
inline std::unordered_map<int, std::function<void(std::string_view)>>
    logListeners; /**< Listeners for all levels but error */
inline std::unordered_map<int, std::function<void(std::string_view)>> logErrListeners; /**< Listener for error level */
inline std::optional<GlobalLoggerSettings> settings = std::nullopt;

inline cppcoro::generator<int> idGenerator = iota<int>(); /**< An id generator for log listeners */
}// namespace details

const auto GLOBAL_LOGGER_NAME = "pf";
const auto TAG_FORMAT = "[{}] {}";
const auto VK_TAG = "vulkan";
const auto APP_TAG = "app";
const auto MAIN_TAG = "main";
inline std::shared_ptr<spdlog::logger> globalLogger = nullptr;

/**
 * Initialise global logger with given settings.
 * @param settings
 */
void initGlobalLogger(const GlobalLoggerSettings &settings);

/**
 *
 * @param level log level
 * @param tag tag added to message
 * @param msg log message
 * @param args format arguments
 */
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

/**
 * Add a listener to either logger without error or with.
 * @param listener listener function
 * @param err if true then the listener listens to error only
 * @return instance of Subscription with which the listener can be canceled
 */
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
    initGlobalLogger(*details::settings);
  });
}

}// namespace pf

#endif//VOXEL_RENDER_LOGGERS_H

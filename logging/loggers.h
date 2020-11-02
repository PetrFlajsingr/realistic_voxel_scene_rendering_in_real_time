//
// Created by petr on 9/23/20.
//

#ifndef VOXEL_RENDER_LOGGERS_H
#define VOXEL_RENDER_LOGGERS_H

#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"
#include <experimental/source_location>
#include <filesystem>
#include <vector>

struct GlobalLoggerSettings {
  bool verbose;
  bool console;
  bool debug;
  std::filesystem::path logDir;
};

std::vector<std::shared_ptr<spdlog::sinks::sink>>
createConsoleLogSinks(const GlobalLoggerSettings &settings, std::string_view tag);

void createLoggerForTag(const GlobalLoggerSettings &settings, std::string_view tag);

namespace pf {

namespace details {
inline std::vector<std::function<void(std::string_view)>> logListeners;
inline std::vector<std::function<void(std::string_view)>> logErrListeners;
inline std::optional<GlobalLoggerSettings> settings = std::nullopt;
}// namespace details

const auto GLOBAL_LOGGER_NAME = "pf";
const auto TAG_FORMAT = "[{}] {}";
const auto VK_TAG = "vulkan";
const auto APP_TAG = "app";
const auto MAIN_TAG = "main";
inline std::shared_ptr<spdlog::logger> globalLogger = nullptr;

void initGlobalLogger(const GlobalLoggerSettings &settings);

void log(spdlog::level::level_enum level, std::string_view tag, std::string_view msg);

void addLogListener(std::invocable<std::string_view> auto listener, bool err = false) {
  if (!details::settings.has_value()) {
    throw std::exception();
  }
  if (!err) {
    details::logListeners.emplace_back(listener);
  } else {
    details::logErrListeners.emplace_back(listener);
  }
  initGlobalLogger(*details::settings);
}

void logFmt(spdlog::level::level_enum level, std::string_view tag, std::string_view msg,
            const auto &... args) {
  globalLogger->log(level, fmt::format(TAG_FORMAT, tag, msg), args...);
}

void logSrc(
    spdlog::level::level_enum level, std::string_view tag, std::string_view msg,
    std::experimental::source_location src_loc = std::experimental::source_location::current());

}// namespace pf

#endif//VOXEL_RENDER_LOGGERS_H

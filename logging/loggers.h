//
// Created by petr on 9/23/20.
//

#ifndef REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_LOGGERS_H
#define REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_LOGGERS_H

#include "../utils.h"
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

const auto GLOBAL_LOGGER_NAME = "pf";
const auto TAG_FORMAT = "[{}] {}";
const auto VK_TAG = "vulkan";
const auto APP_TAG = "app";
const auto MAIN_TAG = "main";
inline std::shared_ptr<spdlog::logger> globalLogger;

void initGlobalLogger(const GlobalLoggerSettings &settings);

void log(spdlog::level::level_enum level, std::string_view tag, std::string_view msg);

void logFmt(spdlog::level::level_enum level, std::string_view tag, std::string_view msg,
            const auto &... args) {
  globalLogger->log(level, fmt::format(TAG_FORMAT, tag, msg), args...);
}

void logSrc(
    spdlog::level::level_enum level, std::string_view tag, std::string_view msg,
    std::experimental::source_location src_loc = std::experimental::source_location::current());

}// namespace pf

#endif//REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_LOGGERS_H

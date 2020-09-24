//
// Created by petr on 9/23/20.
//

#ifndef REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_LOGGERS_H
#define REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_LOGGERS_H

#include "../utils.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"
#include <filesystem>
#include <vector>

struct global_logger_settings {
  bool verbose;
  bool console;
  bool debug;
  std::filesystem::path log_dir;
};

std::vector<std::shared_ptr<spdlog::sinks::sink>>
create_console_log_sinks(const global_logger_settings &settings);

void create_global_logger(const global_logger_settings &settings);

void create_logger_for_tag(const global_logger_settings &settings, std::string_view tag);

#endif//REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_LOGGERS_H

//
// Created by petr on 9/23/20.
//

#ifndef REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_GLOBAL_LOGGER_H
#define REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_GLOBAL_LOGGER_H

#include "../utils.h"
#include "filtered_sink.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include <filesystem>
#include <vector>

struct global_logger_settings {
  bool verbose;
  bool console;
  bool debug;
  std::filesystem::path log_dir;
};

std::vector<std::shared_ptr<spdlog::sinks::sink>> create_console_log_sinks(const global_logger_settings &settings);

void create_global_logger(const global_logger_settings &settings);

#endif//REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_GLOBAL_LOGGER_H

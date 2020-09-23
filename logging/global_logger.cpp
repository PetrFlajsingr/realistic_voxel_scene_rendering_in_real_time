//
// Created by petr on 9/23/20.
//

#include "global_logger.h"


std::vector<std::shared_ptr<spdlog::sinks::sink>> create_console_log_sinks(const global_logger_settings &settings) {
  auto sinks = std::vector<std::shared_ptr<spdlog::sinks::sink>>{};
  auto console_out = std::make_shared<filtered_sink<spdlog::sinks::stdout_color_sink_mt>>();
  console_out->set_filter([settings](spdlog::level::level_enum level, spdlog::string_view_t) {
    if (is_one_of(level, {spdlog::level::err, spdlog::level::critical})) { return false; }
    if (!settings.debug && level == spdlog::level::debug) { return false; }
    return true;
  });
  console_out->set_level(settings.verbose ? spdlog::level::trace
                                          : settings.debug ? spdlog::level::debug
                                                           : spdlog::level::info);
  console_out->set_pattern("[general] [%^%l%$] %v");

  auto console_err = std::make_shared<spdlog::sinks::stderr_color_sink_mt>();
  console_err->set_level(spdlog::level::err);
  console_err->set_pattern("[general] [%^%l%$] %v");

  sinks.emplace_back(console_out);
  sinks.emplace_back(console_err);
  return sinks;
}

void create_global_logger(const global_logger_settings &settings) {
  auto sinks = std::vector<std::shared_ptr<spdlog::sinks::sink>>{};
  if (settings.console) { sinks = create_console_log_sinks(settings); }
  auto log_file_path = settings.log_dir;
  log_file_path.append("log.log");
  sinks.emplace_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>(log_file_path.string()));

  auto logger = std::make_shared<spdlog::logger>("general", sinks.begin(), sinks.end());
  logger->set_level(spdlog::level::trace);
  spdlog::register_logger(logger);
}

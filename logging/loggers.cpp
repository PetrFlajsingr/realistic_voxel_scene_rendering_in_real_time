//
// Created by petr on 9/23/20.
//

#include "loggers.h"
#include "some_levels_sink.h"

std::vector<std::shared_ptr<spdlog::sinks::sink>>
create_console_log_sinks(const global_logger_settings &settings, std::string_view tag) {
  auto sinks = std::vector<std::shared_ptr<spdlog::sinks::sink>>{};

  auto allowed_levels_stdout = std::vector{spdlog::level::info, spdlog::level::warn};
  if (settings.debug) { allowed_levels_stdout.emplace_back(spdlog::level::debug); }
  if (settings.verbose) { allowed_levels_stdout.emplace_back(spdlog::level::trace); }

  auto console_out = std::make_shared<some_levels_sink<spdlog::sinks::stdout_color_sink_mt>>(
      std::move(allowed_levels_stdout));
  console_out->set_level(spdlog::level::trace);
  console_out->set_pattern(fmt::format("[{}] [%^%l%$] %v", tag));

  auto console_err = std::make_shared<spdlog::sinks::stderr_color_sink_mt>();
  console_err->set_level(spdlog::level::err);
  console_err->set_pattern(fmt::format("[{}_err] [%^%l%$] %v", tag));

  sinks.emplace_back(console_out);
  sinks.emplace_back(console_err);
  return sinks;
}

void create_logger_for_tag(const global_logger_settings &settings, std::string_view tag) {
  auto sinks = std::vector<std::shared_ptr<spdlog::sinks::sink>>{};
  if (settings.console) { sinks = create_console_log_sinks(settings, tag); }
  auto log_file_path = settings.log_dir;
  log_file_path.append("log.log");
  sinks.emplace_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>(log_file_path.string()));

  auto logger = std::make_shared<spdlog::logger>(std::string(tag), sinks.begin(), sinks.end());
  const auto logger_level = settings.verbose ? spdlog::level::trace
  : settings.debug                           ? spdlog::level::debug
                                             : spdlog::level::info;
  logger->set_level(logger_level);
  spdlog::register_logger(logger);
}

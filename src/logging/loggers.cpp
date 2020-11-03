//
// Created by petr on 9/23/20.
//

#include "loggers.h"

#include "CallbackSink.h"
#include "SomeLevelsSink.h"
#include <utility>

std::vector<std::shared_ptr<spdlog::sinks::sink>>
createConsoleLogSinks(const GlobalLoggerSettings &settings, std::string_view tag) {
  auto sinks = std::vector<std::shared_ptr<spdlog::sinks::sink>>{};

  auto allowed_levels_stdout = std::vector{spdlog::level::info, spdlog::level::warn};
  if (settings.debug) { allowed_levels_stdout.emplace_back(spdlog::level::debug); }
  if (settings.verbose) { allowed_levels_stdout.emplace_back(spdlog::level::trace); }

  auto console_out = std::make_shared<SomeLevelsSink<spdlog::sinks::stdout_color_sink_mt>>(
      std::move(allowed_levels_stdout));
  console_out->set_level(spdlog::level::trace);
  console_out->set_pattern(fmt::format("[{}] %+", tag));

  for (auto &listener : pf::details::logListeners) {
    auto sink = std::make_shared<CallbackSink>(listener);
    sink->set_level(spdlog::level::trace);
    sink->set_pattern(fmt::format("[{}] %+", tag));
    sinks.emplace_back(sink);
  }

  auto console_err = std::make_shared<spdlog::sinks::stderr_color_sink_mt>();
  console_err->set_level(spdlog::level::err);
  console_err->set_pattern(fmt::format("[{}_err] %+", tag));

  for (auto &listener : pf::details::logErrListeners) {
    auto sink = std::make_shared<CallbackSink>(listener);
    sink->set_level(spdlog::level::err);
    sink->set_pattern(fmt::format("[{}_err] %+", tag));
    sinks.emplace_back(sink);
  }

  sinks.emplace_back(console_out);
  sinks.emplace_back(console_err);
  return sinks;
}

void createLoggerForTag(const GlobalLoggerSettings &settings, std::string_view tag) {
  auto sinks = std::vector<std::shared_ptr<spdlog::sinks::sink>>{};
  if (settings.console) { sinks = createConsoleLogSinks(settings, tag); }
  auto log_file_path = settings.logDir;
  log_file_path.append("log.log");
  sinks.emplace_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>(log_file_path.string()));

  auto logger = std::make_shared<spdlog::logger>(std::string(tag), sinks.begin(), sinks.end());
  const auto logger_level = settings.verbose ? spdlog::level::trace
  : settings.debug                           ? spdlog::level::debug
                                             : spdlog::level::info;
  logger->set_level(logger_level);
  spdlog::register_logger(logger);
}

void pf::initGlobalLogger(const GlobalLoggerSettings &settings) {
  if (globalLogger != nullptr) { spdlog::drop(GLOBAL_LOGGER_NAME); }
  details::settings = settings;
  createLoggerForTag(settings, GLOBAL_LOGGER_NAME);
  globalLogger = spdlog::get(GLOBAL_LOGGER_NAME);
}

void pf::log(spdlog::level::level_enum level, std::string_view tag, std::string_view msg) {
  globalLogger->log(level, fmt::format(TAG_FORMAT, tag, msg));
}

void pf::logSrc(spdlog::level::level_enum level, std::string_view tag, std::string_view msg,
                std::experimental::source_location src_loc) {
  const auto format = "[{}:{}] {}";
  log(level, tag, fmt::format(format, src_loc.file_name(), src_loc.function_name(), msg));
}
void pf::logt(std::string_view tag, std::string_view msg) { log(spdlog::level::trace, tag, msg); }
void pf::logi(std::string_view tag, std::string_view msg) { log(spdlog::level::info, tag, msg); }
void pf::logd(std::string_view tag, std::string_view msg) { log(spdlog::level::debug, tag, msg); }
void pf::logw(std::string_view tag, std::string_view msg) { log(spdlog::level::warn, tag, msg); }
void pf::logc(std::string_view tag, std::string_view msg) {
  log(spdlog::level::critical, tag, msg);
}
void pf::loge(std::string_view tag, std::string_view msg) { log(spdlog::level::err, tag, msg); }

GlobalLoggerInterface::GlobalLoggerInterface(const std::string &loggerName) : ILogger(loggerName) {}
void GlobalLoggerInterface::log(pf::LogLevel level, std::string_view tag, std::string_view msg) {
  auto spdlogLevel = spdlog::level::trace;
  switch (level) {
    case pf::LogLevel::Trace: spdlogLevel = spdlog::level::trace; break;
    case pf::LogLevel::Debug: spdlogLevel = spdlog::level::debug; break;
    case pf::LogLevel::Info: spdlogLevel = spdlog::level::info; break;
    case pf::LogLevel::Warn: spdlogLevel = spdlog::level::warn; break;
    case pf::LogLevel::Err: spdlogLevel = spdlog::level::err; break;
    case pf::LogLevel::Critical: spdlogLevel = spdlog::level::critical; break;
    case pf::LogLevel::Off: spdlogLevel = spdlog::level::off; break;
  }
  pf::log(spdlogLevel, tag, msg);
}

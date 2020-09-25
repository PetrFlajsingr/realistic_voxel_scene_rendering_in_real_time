#include "application.h"
#include "argparse.hpp"
#include "args/valid_path_check_action.h"
#include "coroutines/sequence.h"
#include "logging/loggers.h"
#include "ui/glfw_window.h"
#include <filesystem>
#include <toml.hpp>

argparse::ArgumentParser create_argument_parser() {
  auto argument_parser = argparse::ArgumentParser("Realistic voxel scene rendering in real time");
  argument_parser.add_argument("-v", "--verbose")
      .help("Verbose logging")
      .default_value(false)
      .implicit_value(true);
  argument_parser.add_argument("-l", "--log")
      .help("Enable console logging.")
      .default_value(false)
      .implicit_value(true);
  argument_parser.add_argument("-d", "--debug")
      .help("Enable debug logging.")
      .default_value(false)
      .implicit_value(true);
  argument_parser.add_argument("--log_dir")
      .help("Custom directory for log files.")
      .default_value(std::filesystem::current_path())
      .action(valid_path_check_action{path_type::directory});
  argument_parser.add_argument("--config")
      .help("Custom TOML config file.")
      .default_value(std::filesystem::current_path().append("config.toml"))
      .action(valid_path_check_action{path_type::file});
  return argument_parser;
}

void create_loggers(argparse::ArgumentParser &argument_parser) {
  const auto logger_settings =
      global_logger_settings{.verbose = argument_parser.get<bool>("-v"),
                             .console = argument_parser.get<bool>("-l"),
                             .debug = argument_parser.get<bool>("-d"),
                             .log_dir = argument_parser.get<std::filesystem::path>("--log_dir")};
  create_logger_for_tag(logger_settings, "general");
  create_logger_for_tag(logger_settings, "application");
}

int main(int argc, char *argv[]) {
  using namespace pf;

  auto argument_parser = create_argument_parser();

  try {
    argument_parser.parse_args(argc, argv);
  } catch (const std::runtime_error &err) {
    std::cerr << err.what() << std::endl;
    std::cout << argument_parser;
    return 0;
  }

  try {
    auto config = toml::parse(argument_parser.get<std::filesystem::path>("--config"));

    create_loggers(argument_parser);
    const auto window_settings = window::window_settings{.resolution = {800, 600},
                                                         .title = "test",
                                                         .mode = window::mode_t::windowed};
    auto app = application<glfw_window>(application_settings{.debug = argument_parser.get<bool>("-d"),
                                                      .window_settings = window_settings});
    app.run();
  } catch (const std::exception &exception) {
    auto logger = spdlog::get("general");
    logger->critical("Application crash:");
    logger->critical(exception.what());
    return -1;
  } catch (...) {
    auto logger = spdlog::get("general");
    logger->critical("Unknown application crash.");
    return -1;
  }
  return 0;
}

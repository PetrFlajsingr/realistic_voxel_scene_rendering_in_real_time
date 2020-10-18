#include "application.h"
#include "argparse.hpp"
#include "args/ValidPathCheckAction.h"
#include "coroutines/Sequence.h"
#include "logging/loggers.h"
#include "rendering/TriangleRenderer.h"
#include "ui/GlfwWindow.h"
#include <filesystem>
#include <toml.hpp>

argparse::ArgumentParser createArgumentParser() {
  auto argumentParser = argparse::ArgumentParser("Realistic voxel scene rendering in real time");
  argumentParser.add_argument("-v", "--verbose")
      .help("Verbose logging")
      .default_value(false)
      .implicit_value(true);
  argumentParser.add_argument("-l", "--log")
      .help("Enable console logging.")
      .default_value(false)
      .implicit_value(true);
  argumentParser.add_argument("-d", "--debug")
      .help("Enable debug logging.")
      .default_value(false)
      .implicit_value(true);
  argumentParser.add_argument("--log_dir")
      .help("Custom directory for log files.")
      .default_value(std::filesystem::current_path())
      .action(ValidPathCheckAction{PathType::Directory});
  argumentParser.add_argument("--config")
      .help("Custom TOML config file.")
      .default_value(std::filesystem::current_path().append("config.toml"))
      .action(ValidPathCheckAction{PathType::File});
  return argumentParser;
}

void createLogger(argparse::ArgumentParser &argument_parser) {
  const auto loggerSettings =
      GlobalLoggerSettings{.verbose = argument_parser.get<bool>("-v"),
                             .console = argument_parser.get<bool>("-l"),
                             .debug = argument_parser.get<bool>("-d"),
                             .logDir = argument_parser.get<std::filesystem::path>("--log_dir")};
  pf::initGlobalLogger(loggerSettings);
}


int main(int argc, char *argv[]) {
  using namespace pf;
  auto argumentParser = createArgumentParser();

  try {
    argumentParser.parse_args(argc, argv);
  } catch (const std::runtime_error &err) {
    std::cerr << err.what() << std::endl;
    std::cout << argumentParser;
    return 0;
  }

  //try {
    auto config = toml::parse(argumentParser.get<std::filesystem::path>("--config"));

    createLogger(argumentParser);
    auto resolutionConfig = config["ui"]["window"];
    const auto windowSettings = window::WindowSettings{
        .resolution = {static_cast<std::size_t>(resolutionConfig["width"].as_integer()),
                       static_cast<std::size_t>(resolutionConfig["height"].as_integer())},
        .title = "test",
        .mode = window::Mode::Windowed};

    auto app = application<GlfwWindow, pf::TriangleRenderer>(
        pf::TriangleRenderer(),
        application_settings{.debug = argumentParser.get<bool>("-d"),
                             .window_settings = windowSettings});
    app.run();
  //} catch (const std::exception &exception) {
  //  pf::log(spdlog::level::critical, MAIN_TAG, "Application crash:");
  //  pf::log(spdlog::level::critical, MAIN_TAG, exception.what());
  //  throw;
  //  //return -1;
  //} catch (...) {
  //  pf::log(spdlog::level::critical, MAIN_TAG, "Unknown application crash.");
  //  throw;
  //  //return -1;
  //}
  return 0;
}

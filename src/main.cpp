#include "Application.h"
#include "argparse.hpp"
#include "args/ValidPathCheckAction.h"
#include "logging/loggers.h"
#include "rendering/SVORenderer.h"
#include <filesystem>
#include <pf_common/RAII.h>
#include <pf_glfw_vulkan/ui/GlfwWindow.h>
#include <toml++/toml.h>

// TODO: change include guards

argparse::ArgumentParser createArgumentParser() {
  auto argumentParser = argparse::ArgumentParser("Realistic voxel scene rendering in real time");
  argumentParser.add_argument("-v", "--verbose").help("Verbose logging").default_value(false).implicit_value(true);
  argumentParser.add_argument("-l", "--log").help("Enable console logging.").default_value(false).implicit_value(true);
  argumentParser.add_argument("-d", "--debug").help("Enable debug logging.").default_value(false).implicit_value(true);
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
  const auto loggerSettings = GlobalLoggerSettings{.verbose = argument_parser.get<bool>("-v"),
                                                   .console = argument_parser.get<bool>("-l"),
                                                   .debug = argument_parser.get<bool>("-d"),
                                                   .logDir = argument_parser.get<std::filesystem::path>("--log_dir")};
  pf::initGlobalLogger(loggerSettings);
}

void saveConfig(const std::filesystem::path &dst, toml::table &config) {
  auto ofstream = std::ofstream(dst);
  ofstream << config;
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

  auto configPath = argumentParser.get<std::filesystem::path>("--config");
  //try {
  createLogger(argumentParser);

  auto config = toml::parse_file(configPath.string());
  auto saveConfigRAII = pf::RAII([&] { saveConfig(configPath, config); });

  auto resolutionConfig = config["ui"]["window"];
  const auto windowSettings =
      ui::WindowSettings{.resolution = {static_cast<size_t>(resolutionConfig["width"].value_or(800)),
                                        static_cast<size_t>(resolutionConfig["height"].value_or(600))},
                         .title = "test",
                         .mode = ui::Mode::Windowed};

  {
    auto app = Application<ui::GlfwWindow, SVORenderer>(
        SVORenderer(*config.as_table()),
        ApplicationSettings{.debug = argumentParser.get<bool>("-d"), .window_settings = windowSettings});
    app.run();
  }

  //} catch (const std::exception &exception) {
  //  //pf::log(spdlog::level::critical, MAIN_TAG, "Application crash:");
  //  //pf::log(spdlog::level::critical, MAIN_TAG, exception.what());
  //  throw;
  //  //return -1;
  //} catch (...) {
  //  //pf::log(spdlog::level::critical, MAIN_TAG, "Unknown application crash.");
  //  throw;
  //  //return -1;
  //}
  return 0;
}

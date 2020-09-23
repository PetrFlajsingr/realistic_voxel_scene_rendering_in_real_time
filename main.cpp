#include "argparse.hpp"
#include "args/valid_path_check_action.h"
#include "logging/global_logger.h"
#include "spdlog/spdlog.h"
#include "utils.h"
#include <filesystem>

argparse::ArgumentParser create_argument_parser() {
  auto argument_parser = argparse::ArgumentParser("Realistic voxel scene rendering in real time");
  argument_parser.add_argument("-v", "--verbose").help("Verbose logging").default_value(false).implicit_value(true);
  argument_parser.add_argument("-l", "--log").help("Enable console logging.").default_value(false).implicit_value(true);
  argument_parser.add_argument("-d", "--debug").help("Enable debug logging.").default_value(false).implicit_value(true);
  argument_parser.add_argument("--log_dir")
      .help("Custom directory for log files.")
      .default_value(std::filesystem::current_path())
      .action(valid_path_check_action{});
  return argument_parser;
}

int main(int argc, char *argv[]) {
  auto argument_parser = create_argument_parser();

  try {
    argument_parser.parse_args(argc, argv);
    create_global_logger(global_logger_settings{.verbose = argument_parser.get<bool>("-v"),
                                                .console = argument_parser.get<bool>("-l"),
                                                .debug = argument_parser.get<bool>("-d"),
                                                .log_dir = argument_parser.get<std::filesystem::path>("--log_dir")});

  } catch (const std::runtime_error &err) {
    std::cerr << err.what() << std::endl;
    std::cout << argument_parser;
  }
  return 0;
}

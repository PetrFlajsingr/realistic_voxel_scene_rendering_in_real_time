/**
 * @file RegexCheckAction.h
 * @brief An action to match argument value to a regex.
 * @author Petr Flajšingr
 * @date 24.9.20
 */

#ifndef VOXEL_RENDER_REGEXCHECKACTION_H
#define VOXEL_RENDER_REGEXCHECKACTION_H

#include <regex>

/**
 * @brief Matches argument to a regex. For argparse.
 */
class RegexCheckAction {
 public:
  explicit RegexCheckAction(std::regex regex);
  explicit RegexCheckAction(std::string_view regexStr);
  std::string_view operator()(std::string_view arg);

 private:
  std::regex rgx;
};

#endif//VOXEL_RENDER_REGEXCHECKACTION_H

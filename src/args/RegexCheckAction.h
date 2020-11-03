//
// Created by petr on 9/24/20.
//

#ifndef VOXEL_RENDER_REGEXCHECKACTION_H
#define VOXEL_RENDER_REGEXCHECKACTION_H

#include <regex>

class RegexCheckAction {
 public:
  explicit RegexCheckAction(std::regex regex);
  explicit RegexCheckAction(std::string_view regexStr);
  std::string_view operator()(std::string_view arg);

 private:
  std::regex rgx;
};

#endif//VOXEL_RENDER_REGEXCHECKACTION_H

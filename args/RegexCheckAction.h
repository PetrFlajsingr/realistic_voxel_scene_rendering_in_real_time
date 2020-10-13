//
// Created by petr on 9/24/20.
//

#ifndef REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_REGEXCHECKACTION_H
#define REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_REGEXCHECKACTION_H

#include <regex>

class RegexCheckAction {
 public:
  explicit RegexCheckAction(std::regex regex);
  explicit RegexCheckAction(std::string_view regexStr);
  std::string_view operator()(std::string_view arg);

 private:
  std::regex regex;
};

#endif//REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_REGEXCHECKACTION_H

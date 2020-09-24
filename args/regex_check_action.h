//
// Created by petr on 9/24/20.
//

#ifndef REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_REGEX_CHECK_ACTION_H
#define REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_REGEX_CHECK_ACTION_H

#include <regex>

class regex_check_action {
 public:
  explicit regex_check_action(std::regex regex);
  explicit regex_check_action(std::string_view regex_str);
  std::string_view operator()(std::string_view arg);

 private:
  std::regex regex;
};


#endif//REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_REGEX_CHECK_ACTION_H

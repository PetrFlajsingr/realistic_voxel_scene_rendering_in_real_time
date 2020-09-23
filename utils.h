//
// Created by petr on 9/23/20.
//

#ifndef REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_UTILS_H
#define REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_UTILS_H

#include <algorithm>

template<typename T, typename Container = std::initializer_list<T>>
bool is_one_of(const T &val, Container &&vals) {
  return std::ranges::any_of(vals, [&val](const auto &v) { return val == v; });
}

#endif//REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_UTILS_H

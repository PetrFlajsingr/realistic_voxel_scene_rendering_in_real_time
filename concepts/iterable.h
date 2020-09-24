//
// Created by petr on 9/24/20.
//

#ifndef REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_ITERABLE_H
#define REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_ITERABLE_H
#include <iterator>

template<typename T>
concept iterable = requires(T t) {
  { std::begin(t) }
  ->std::forward_iterator;
  { std::end(t) }
  ->std::sentinel_for<decltype(std::begin(t))>;
};

#endif//REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_ITERABLE_H
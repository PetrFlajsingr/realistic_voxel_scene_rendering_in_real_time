//
// Created by petr on 9/24/20.
//

#ifndef REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_META_H
#define REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_META_H

#include <iterator>

template<typename T>
concept iterable = requires(T t) {
  { std::begin(t) }
  ->std::forward_iterator;
  { std::end(t) }
  ->std::sentinel_for<decltype(std::begin(t))>;
};

template<typename T>
concept stream_outputable = requires(T t, std::ostream o) {
  { o << t }
  ->std::convertible_to<std::ostream &>;
};

#endif//REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_META_H

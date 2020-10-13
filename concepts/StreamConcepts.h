//
// Created by petr on 9/24/20.
//

#ifndef REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_STREAMCONCEPTS_H
#define REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_STREAMCONCEPTS_H
#include <istream>
#include <ostream>

template<typename T>
concept StreamInputable = requires(T t, std::ostream o) {
  { o << t }
  ->std::convertible_to<std::ostream &>;
};

template<typename T>
concept StreamOutputable = requires(T t, std::istream i) {
  { t >> i }
  ->std::convertible_to<std::istream &>;
};

template<typename T>
concept Streamable = StreamOutputable<T> &&StreamInputable<T>;
#endif//REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_STREAMCONCEPTS_H

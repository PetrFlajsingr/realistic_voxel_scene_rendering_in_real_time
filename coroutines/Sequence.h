//
// Created by petr on 9/24/20.
//

#ifndef REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_SEQUENCE_H
#define REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_SEQUENCE_H

#include "../concepts/Incrementable.h"
#include <cppcoro/generator.hpp>

template<Incrementable T>
cppcoro::generator<T> iota(T start = T(0)) {
  while (true) {
    co_yield start;
    ++start;
  }
}

template<Incrementable T>
cppcoro::generator<T> range(T start, T end, T step = T(1)) {
  for (Incrementable auto i = start; i < end; i += step) { co_yield i; }
}
template<Incrementable T>
cppcoro::generator<T> range(T end) {
  return range(T(0), end);
}

template<typename T>
T getNext(cppcoro::generator<T> &generator) {
  auto iter = generator.begin();
  return *iter;
}

#endif//REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_SEQUENCE_H

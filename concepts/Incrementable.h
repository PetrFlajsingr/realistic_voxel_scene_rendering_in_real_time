//
// Created by petr on 9/24/20.
//

#ifndef REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_INCREMENTABLE_H
#define REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_INCREMENTABLE_H
#include <concepts>

template<typename T>
concept Incrementable = requires(T t) {
  {++t};
  {t += t};
  {t + t};
};

#endif//REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_INCREMENTABLE_H

//
// Created by petr on 10/19/20.
//

#ifndef REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_ONEOF_H
#define REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_ONEOF_H

#include <concepts>

template <typename T, typename ...Args>
concept OneOf = (std::same_as<T, Args> || ...);

#endif//REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_ONEOF_H

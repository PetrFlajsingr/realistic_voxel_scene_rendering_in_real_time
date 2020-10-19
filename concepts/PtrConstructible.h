//
// Created by petr on 10/12/20.
//

#ifndef REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_PTRCONSTRUCTIBLE_H
#define REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_PTRCONSTRUCTIBLE_H

#include <memory>

template<typename T>
struct PtrConstructible {
  template<typename... Args>
  static std::shared_ptr<T>
  CreateShared(Args &&... args) requires std::constructible_from<T, Args...> {
    return std::make_shared<T>(std::forward<Args>(args)...);
  }
  template<typename... Args>
  static std::unique_ptr<T>
  CreateUnique(Args &&... args) requires std::constructible_from<T, Args...> {
    return std::make_unique<T>(std::forward<Args>(args)...);
  }
};

#endif//REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_PTRCONSTRUCTIBLE_H

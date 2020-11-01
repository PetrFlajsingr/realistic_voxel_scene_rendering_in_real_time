//
// Created by petr on 11/1/20.
//

#ifndef REALISTIC_VOXEL_RENDERING_UTILS_RAII_H
#define REALISTIC_VOXEL_RENDERING_UTILS_RAII_H

#include <concepts>
#include <functional>
#include <utility>

class RAII {
 public:
  inline explicit RAII(std::invocable auto &&callable) : callable(std::forward(callable)) {}

  inline ~RAII() { std::invoke(callable); }

 private:
  std::function<void()> callable;
};

#endif//REALISTIC_VOXEL_RENDERING_UTILS_RAII_H
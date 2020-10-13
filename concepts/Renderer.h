//
// Created by petr on 9/27/20.
//

#ifndef REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_RENDERER_H
#define REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_RENDERER_H

#include "Window.h"
#include <concepts>

namespace pf {
template<typename T, typename Window>
concept Renderer = window::Window<Window> &&requires(T t, Window window) {
  {t.init(window)};
  {t.render()};
};

}// namespace pf
#endif//REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_RENDERER_H

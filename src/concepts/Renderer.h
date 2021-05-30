//
// Created by petr on 9/27/20.
//

#ifndef VOXEL_RENDER_RENDERER_H
#define VOXEL_RENDER_RENDERER_H

#include <concepts>
#include <pf_glfw_vulkan/ui/Window.h>

namespace pf {
template<typename T>
concept Renderer = requires(T t, ui::Window &window) {
  {t.init(window)};
  {t.render()};
  {t.stop()};
} && std::move_constructible<T>;

}// namespace pf
#endif//VOXEL_RENDER_RENDERER_H

//
// Created by petr on 9/27/20.
//

#ifndef VOXEL_RENDER_RENDERER_H
#define VOXEL_RENDER_RENDERER_H

#include <concepts>
#include <pf_glfw_vulkan/concepts/Window.h>

namespace pf {
template<typename T, typename Window>
concept Renderer = ui::Window<Window> &&requires(T t, Window window) {
  {t.init(window)};
  {t.render()};
}
&&std::move_constructible<T>;

}// namespace pf
#endif//VOXEL_RENDER_RENDERER_H

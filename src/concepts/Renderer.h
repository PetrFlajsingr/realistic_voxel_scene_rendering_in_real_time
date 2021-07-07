/**
 * @file Renderer.h
 * @brief A concept for valid renderer.
 * @author Petr Flajšingr
 * @date 27.9.20
 */

#ifndef VOXEL_RENDER_RENDERER_H
#define VOXEL_RENDER_RENDERER_H

#include <concepts>
#include <pf_glfw_vulkan/ui/Window.h>

namespace pf {
/**
 * @brief Checks for renderer validity.
 * @tparam T renderer type
 */
template<typename T>
concept Renderer = requires(T t, std::shared_ptr<ui::Window> window) {
  {t.init(window)};
  {t.render()};
  {t.stop()};
}
&&std::move_constructible<T>;

}// namespace pf
#endif//VOXEL_RENDER_RENDERER_H

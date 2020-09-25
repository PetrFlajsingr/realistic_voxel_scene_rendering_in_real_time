//
// Created by petr on 9/24/20.
//

#ifndef REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_WINDOW_H
#define REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_WINDOW_H

#include "../ui/events/common.h"
#include "../ui/events/subscription.h"
#include "fmt/format.h"
#include "streams.h"
#include <concepts>
#include <fmt/ostream.h>
#include <functional>
#include <string>
#include <unordered_set>
#include <vulkan/vulkan.hpp>

namespace pf::window {
struct resolution_t {
  std::size_t width;
  std::size_t height;
  [[nodiscard]] double aspect_ratio() const;
};
std::ostream &operator<<(std::ostream &os, const resolution_t &res);

enum class mode_t { windowed, borderless, fullscreen, fullscreen_borderless };

struct window_settings {
  resolution_t resolution;
  std::string title;
  mode_t mode;
};

class window_data {
 public:
  explicit window_data(const window_settings &settings);
  [[nodiscard]] const resolution_t &get_resolution() const;
  void set_resolution(const resolution_t &res);
  [[nodiscard]] const std::string &get_title() const;
  void set_title(const std::string &tit);
  [[nodiscard]] mode_t get_mode() const;
  void set_mode(mode_t mod);

 protected:
  resolution_t resolution;
  std::string title;
  mode_t mode;
};

template<typename T>
concept window = std::constructible_from<T, window_settings> &&requires(
    T t, std::function<void()> callback, resolution_t res, mode_t mod, std::string title) {
  { t.init() }
  ->std::same_as<std::optional<std::string>>;
  {t.set_main_loop_callback(callback)};
  {t.main_loop()};
  {t.set_resolution(res)};
  {t.set_mode(mod)};
  {t.set_title(title)};
  { t.get_resolution() }
  ->std::convertible_to<resolution_t>;
  { t.get_mode() }
  ->std::convertible_to<mode_t>;
  { t.get_title() }
  ->std::convertible_to<std::string>;
}
&&requires(T t, events::mouse_event_type m_type, events::key_event_type k_type,
           events::details::mouse_event_fnc m_fnc, events::details::key_event_fnc k_fnc) {
  { t.add_mouse_listener(m_type, m_fnc) }
  ->std::same_as<events::subscription>;
  { t.add_key_listener(k_type, k_fnc) }
  ->std::same_as<events::subscription>;
}
&&requires(T t, const vk::Instance &instance) {
  { t.create_vulkan_surface(instance) }
  ->std::same_as<vk::UniqueSurfaceKHR>;
  { t.required_vulkan_extensions() }
  ->std::same_as<std::unordered_set<std::string>>;
}
&&stream_outputable<T>;

}// namespace pf::window

#endif//REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_WINDOW_H

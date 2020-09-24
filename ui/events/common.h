//
// Created by petr on 9/24/20.
//

#ifndef REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_COMMON_H
#define REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_COMMON_H

#include <functional>
#include <magic_enum.hpp>
#include <utility>

namespace pf::events {

enum class mouse_button { left, right, middle, none };
constexpr unsigned int mouse_button_count = 3;
enum class mouse_button_state { pressed, released };

enum class mouse_event_type { down, up, click, dbl_click, move, wheel };
constexpr std::size_t mouse_event_type_count = magic_enum::enum_count<mouse_event_type>();

enum class key_event_type { up, pressed, repeat };
constexpr std::size_t keyboard_event_type_count = magic_enum::enum_count<key_event_type>();

struct key_event {
  key_event_type type;
  char key;
};

struct mouse_event {
  mouse_event_type type;
  mouse_button button;
  std::pair<double, double> location;
  std::pair<double, double> delta;
};

namespace details {
using mouse_event_fnc = std::function<bool(mouse_event)>;
using key_event_fnc = std::function<bool(key_event)>;
}// namespace details

template<typename F>
concept mouse_event_listener =
    std::invocable<F, mouse_event> &&std::same_as<std::invoke_result_t<F, mouse_event>, bool>;
template<typename F>
concept key_event_listener =
    std::invocable<F, key_event> &&std::same_as<std::invoke_result_t<F, key_event>, bool>;

}// namespace pf::events

#endif//REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_COMMON_H

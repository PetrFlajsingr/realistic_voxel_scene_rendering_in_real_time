//
// Created by petr on 9/24/20.
//

#ifndef REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_EVENT_DISPATCH_IMPL_H
#define REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_EVENT_DISPATCH_IMPL_H

#include "../../coroutines/sequence.h"
#include "common.h"
#include "subscription.h"
#include <array>
#include <chrono>
#include <cppcoro/generator.hpp>
#include <iostream>
#include <queue>
#include <unordered_map>
#include <utility>

namespace pf::events {
class event_dispatch_impl {
 public:
  events::subscription add_mouse_listener(mouse_event_type type,
                                          mouse_event_listener auto listener) {
    const auto id = generate_listener_id();
    mouse_listeners[magic_enum::enum_integer(type)][id] = listener;
    return subscription(
        [id, type, this] { mouse_listeners[magic_enum::enum_integer(type)].erase(id); });
  }

  subscription add_key_listener(key_event_type type, key_event_listener auto listener) {
    const auto id = generate_listener_id();
    key_listeners[magic_enum::enum_integer(type)][id] = listener;
    return subscription(
        [id, type, this] { key_listeners[magic_enum::enum_integer(type)].erase(id); });
  }

  subscription add_text_listener(text_event_listener auto listener) {
    const auto id = generate_listener_id();
    text_listeners[id] = listener;
    return subscription([id, this] { text_listeners.erase(id); });
  }

  [[nodiscard]] bool is_mouse_down() const { return is_mouse_down_; }

 protected:
  using listener_id = uint32_t;

  void notify_mouse(mouse_event_type type, mouse_button button, std::pair<double, double> location,
                    std::pair<double, double> delta) {
    auto mouse_clicked = false;
    if (type == mouse_event_type::down) {
      is_mouse_down_ = true;
    } else if (type == mouse_event_type::up) {
      mouse_clicked = is_mouse_down_;
      is_mouse_down_ = false;
    }
    auto &listeners = mouse_listeners[magic_enum::enum_integer(type)];
    const auto event =
        mouse_event{.type = type, .button = button, .location = location, .delta = delta};
    for (auto &[id, listener] : listeners) {
      if (listener(event)) { break; }
    }
    if (mouse_clicked) {// TODO: delayed event for click OR dbl click
      using namespace std::chrono_literals;
      if (is_dbl_click()) {
        last_click_time = std::chrono::steady_clock::now();
        notify_mouse(mouse_event_type::dbl_click, event.button, event.location, event.delta);
      } else {
        last_click_time = std::chrono::steady_clock::now();
        enqueue(
            [this, event] {
              if (std::chrono::steady_clock::now() - last_click_time < DBL_CLICK_LIMIT) { return; }
              notify_mouse(mouse_event_type::click, event.button, event.location, event.delta);
            },
            DBL_CLICK_LIMIT);
      }
    }
  }

  void notify_key(key_event_type type, char key) {
    auto &listeners = key_listeners[magic_enum::enum_integer(type)];
    const auto event = key_event{.type = type, .key = key};
    for (auto &[id, listener] : listeners) {
      if (listener(event)) { break; }
    }
  }

  void notify_text(std::string text) {
    const auto event = text_event{.text = std::move(text)};
    for (auto &[id, listener] : text_listeners) {
      if (listener(event)) { break; }
    }
  }

  void on_frame() {
    const auto curr_time = std::chrono::steady_clock::now();
    while (!event_queue.empty() && event_queue.top().exec_time <= curr_time) {
      event_queue.top()();
      event_queue.pop();
    }
  }

  void enqueue(std::invocable auto &&fnc,
               std::chrono::milliseconds delay = std::chrono::milliseconds(0)) {
    const auto exec_time = std::chrono::steady_clock::now() + delay;
    event_queue.emplace(fnc, exec_time);
  }

 private:
  listener_id generate_listener_id() { return get_next(id_generator); }

  bool is_dbl_click() {
    const auto curr_time = std::chrono::steady_clock::now();
    const auto difference = curr_time - last_click_time;
    return difference < DBL_CLICK_LIMIT;
  }

  cppcoro::generator<listener_id> id_generator = iota<listener_id>();

  std::array<std::unordered_map<listener_id, details::mouse_event_fnc>, mouse_event_type_count>
      mouse_listeners;
  std::array<std::unordered_map<listener_id, details::key_event_fnc>, keyboard_event_type_count>
      key_listeners;
  std::unordered_map<listener_id, details::text_event_fnc> text_listeners;

  bool is_mouse_down_ = false;
  std::chrono::steady_clock::time_point last_click_time{};
  const std::chrono::milliseconds DBL_CLICK_LIMIT{1500};

  struct delay_event {
    std::function<void()> fnc;
    std::chrono::steady_clock::time_point exec_time;
    bool operator<(const delay_event &rhs) const { return exec_time < rhs.exec_time; }

    void operator()() const { fnc(); }
  };
  std::priority_queue<delay_event, std::vector<delay_event>, std::less<>> event_queue;
};
}// namespace pf::events

#endif//REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_EVENT_DISPATCH_IMPL_H

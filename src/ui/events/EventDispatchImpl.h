//
// Created by petr on 9/24/20.
//

#ifndef VOXEL_RENDER_EVENTDISPATCHIMPL_H
#define VOXEL_RENDER_EVENTDISPATCHIMPL_H

#include <pf_common/coroutines/Sequence.h>
#include <pf_common/Subscription.h>
#include "common.h"
#include <array>
#include <chrono>
#include <iostream>
#include <queue>
#include <unordered_map>
#include <utility>
#include <concepts>

namespace pf::events {
class EventDispatchImpl {
 public:
  Subscription addMouseListener(MouseEventType type, MouseEventListener auto listener);

  Subscription addKeyListener(KeyEventType type, KeyEventListener auto listener);

  Subscription addTextListener(TextEventListener auto listener);

  [[nodiscard]] bool isMouseDown() const;

 protected:
  using ListenerId = uint32_t;

  void notifyMouse(MouseEventType type, MouseButton button, std::pair<double, double> location,
                   std::pair<double, double> delta);

  void notifyKey(KeyEventType type, char key);

  void notifyText(std::string text);

  void onFrame();

  void enqueue(std::invocable auto &&fnc,
               std::chrono::milliseconds delay = std::chrono::milliseconds(0));

 private:
  ListenerId generateListenerId();

  bool isDblClick();

  cppcoro::generator<ListenerId> idGenerator = iota<ListenerId>();

  std::array<std::unordered_map<ListenerId, details::MouseEventFnc>, MouseEventTypeCount>
      mouseListeners;
  std::array<std::unordered_map<ListenerId, details::KeyEventFnc>, KeyboardEventTypeCount>
      keyListeners;
  std::unordered_map<ListenerId, details::TextEventFnc> textListeners;

  bool isMouseDown_ = false;
  std::chrono::steady_clock::time_point lastClickTime{};
  const std::chrono::milliseconds DBL_CLICK_LIMIT{1500};

  struct DelayEvent {
    std::function<void()> fnc;
    std::chrono::steady_clock::time_point execTime;
    bool operator<(const DelayEvent &rhs) const;

    void operator()() const;
  };
  std::priority_queue<DelayEvent, std::vector<DelayEvent>, std::less<>> eventQueue;
};
}// namespace pf::events

#endif//VOXEL_RENDER_EVENTDISPATCHIMPL_H

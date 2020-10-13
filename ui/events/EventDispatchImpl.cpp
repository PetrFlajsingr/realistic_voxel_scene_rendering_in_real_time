//
// Created by petr on 10/12/20.
//
#include "EventDispatchImpl.h"

pf::events::Subscription
pf::events::EventDispatchImpl::addMouseListener(pf::events::MouseEventType type,
                                                MouseEventListener auto listener) {
  const auto id = generateListenerId();
  mouseListeners[magic_enum::enum_integer(type)][id] = listener;
  return Subscription(
      [id, type, this] { mouseListeners[magic_enum::enum_integer(type)].erase(id); });
}

pf::events::Subscription
pf::events::EventDispatchImpl::addKeyListener(pf::events::KeyEventType type,
                                              KeyEventListener auto listener) {
  const auto id = generateListenerId();
  keyListeners[magic_enum::enum_integer(type)][id] = listener;
  return Subscription([id, type, this] { keyListeners[magic_enum::enum_integer(type)].erase(id); });
}

pf::events::Subscription
pf::events::EventDispatchImpl::addTextListener(TextEventListener auto listener) {
  const auto id = generateListenerId();
  textListeners[id] = listener;
  return Subscription([id, this] { textListeners.erase(id); });
}

bool pf::events::EventDispatchImpl::isMouseDown() const { return isMouseDown_; }
void pf::events::EventDispatchImpl::notifyMouse(pf::events::MouseEventType type,
                                                pf::events::MouseButton button,
                                                std::pair<double, double> location,
                                                std::pair<double, double> delta) {
  auto mouseClicked = false;
  if (type == MouseEventType::Down) {
    isMouseDown_ = true;
  } else if (type == MouseEventType::Up) {
    mouseClicked = isMouseDown_;
    isMouseDown_ = false;
  }
  auto &listeners = mouseListeners[magic_enum::enum_integer(type)];
  const auto event =
      MouseEvent{.type = type, .button = button, .location = location, .delta = delta};
  for (auto &[id, listener] : listeners) {
    if (listener(event)) { break; }
  }
  if (mouseClicked) {
    using namespace std::chrono_literals;
    if (isDblClick()) {
      lastClickTime = std::chrono::steady_clock::now();
      notifyMouse(MouseEventType::DblClick, event.button, event.location, event.delta);
    } else {
      lastClickTime = std::chrono::steady_clock::now();
      enqueue(
          [this, event] {
            if (std::chrono::steady_clock::now() - lastClickTime < DBL_CLICK_LIMIT) { return; }
            notifyMouse(MouseEventType::Click, event.button, event.location, event.delta);
          },
          DBL_CLICK_LIMIT);
    }
  }
}
void pf::events::EventDispatchImpl::notifyKey(pf::events::KeyEventType type, char key) {
  auto &listeners = keyListeners[magic_enum::enum_integer(type)];
  const auto event = KeyEvent{.type = type, .key = key};
  for (auto &[id, listener] : listeners) {
    if (listener(event)) { break; }
  }
}
void pf::events::EventDispatchImpl::notifyText(std::string text) {
  const auto event = TextEvent{.text = std::move(text)};
  for (auto &[id, listener] : textListeners) {
    if (listener(event)) { break; }
  }
}
void pf::events::EventDispatchImpl::onFrame() {
  const auto currTime = std::chrono::steady_clock::now();
  while (!eventQueue.empty() && eventQueue.top().execTime <= currTime) {
    eventQueue.top()();
    eventQueue.pop();
  }
}
void pf::events::EventDispatchImpl::enqueue(std::invocable auto &&fnc,
                                            std::chrono::milliseconds delay) {
  const auto execTime = std::chrono::steady_clock::now() + delay;
  eventQueue.emplace(fnc, execTime);
}
pf::events::EventDispatchImpl::ListenerId pf::events::EventDispatchImpl::generateListenerId() {
  return getNext(idGenerator);
}
bool pf::events::EventDispatchImpl::isDblClick() {
  const auto currTime = std::chrono::steady_clock::now();
  const auto difference = currTime - lastClickTime;
  return difference < DBL_CLICK_LIMIT;
}
bool pf::events::EventDispatchImpl::DelayEvent::operator<(
    const pf::events::EventDispatchImpl::DelayEvent &rhs) const {
  return execTime < rhs.execTime;
}
void pf::events::EventDispatchImpl::DelayEvent::operator()() const { fnc(); }

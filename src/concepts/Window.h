//
// Created by petr on 9/24/20.
//

#ifndef VOXEL_RENDER_WINDOW_H
#define VOXEL_RENDER_WINDOW_H

#include <pf_common/Subscription.h>
#include "ui/events/common.h"
#include <pf_common/concepts/StreamConcepts.h>
#include <fmt/format.h>
#include <concepts>
#include <fmt/ostream.h>
#include <functional>
#include <string>
#include <unordered_set>
#include <vulkan/vulkan.hpp>

namespace pf::ui {
struct Resolution {
  std::size_t width;
  std::size_t height;
  [[nodiscard]] double aspectRatio() const;
};
std::ostream &operator<<(std::ostream &os, const Resolution &res);

enum class Mode { Windowed, Borderless, Fullscreen, FullscreenBorderless };

struct WindowSettings {
  Resolution resolution;
  std::string title;
  Mode mode;
};

class WindowData {
 public:
  explicit WindowData(const WindowSettings &settings);
  [[nodiscard]] const Resolution &getResolution() const;
  void setResolution(const Resolution &res);
  [[nodiscard]] const std::string &getTitle() const;
  void setTitle(const std::string &windowTitle);
  [[nodiscard]] Mode getMode() const;
  void setMode(Mode windowMode);

 protected:
  Resolution resolution;
  std::string title;
  Mode mode;
};

template<typename T>
concept Window = std::constructible_from<T, WindowSettings> &&requires(
    T t, std::function<void()> callback, Resolution res, Mode mod, std::string title,
    std::function<void(Resolution)> resizeCallback) {
  { t.init() }
  ->std::same_as<std::optional<std::string>>;
  {t.setMainLoopCallback(callback)};
  {t.setResizeCallback(resizeCallback)};
  {t.mainLoop()};
  {t.setResolution(res)};
  {t.setMode(mod)};
  {t.setTitle(title)};
  { t.getResolution() }
  ->std::convertible_to<Resolution>;
  { t.getMode() }
  ->std::convertible_to<Mode>;
  { t.getTitle() }
  ->std::convertible_to<std::string>;
}
&&requires(T t, events::MouseEventType m_type, events::KeyEventType k_type,
           events::details::MouseEventFnc m_fnc, events::details::KeyEventFnc k_fnc) {
  { t.addMouseListener(m_type, m_fnc) }
  ->std::same_as<Subscription>;
  { t.addKeyListener(k_type, k_fnc) }
  ->std::same_as<Subscription>;
}
&&requires(T t, const vk::Instance &instance) {
  { t.createVulkanSurface(instance) }
  ->std::same_as<vk::UniqueSurfaceKHR>;
  { t.requiredVulkanExtensions() }
  ->std::same_as<std::unordered_set<std::string>>;
}
&&StreamInputable<T>;

}// namespace pf::ui

#endif//VOXEL_RENDER_WINDOW_H

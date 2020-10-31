//
// Created by petr on 9/24/20.
//
#include "Window.h"
#include <fmt/format.h>

double pf::ui::Resolution::aspectRatio() const { return static_cast<double>(width) / height; }

std::ostream &pf::ui::operator<<(std::ostream &os, const pf::ui::Resolution &res) {
  os << fmt::format("{}x{}", res.width, res.height);
  return os;
}

pf::ui::WindowData::WindowData(const pf::ui::WindowSettings &settings)
    : resolution(settings.resolution), title(settings.title), mode(settings.mode) {}

const pf::ui::Resolution &pf::ui::WindowData::getResolution() const {
  return WindowData::resolution;
}

void pf::ui::WindowData::setResolution(const pf::ui::Resolution &res) {
  WindowData::resolution = res;
}

const std::string &pf::ui::WindowData::getTitle() const { return title; }

void pf::ui::WindowData::setTitle(const std::string &windowTitle) { title = windowTitle; }

pf::ui::Mode pf::ui::WindowData::getMode() const { return mode; }

void pf::ui::WindowData::setMode(ui::Mode windowMode) { mode = windowMode; }

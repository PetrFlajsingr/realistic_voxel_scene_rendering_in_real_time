//
// Created by petr on 9/24/20.
//
#include "Window.h"
#include <fmt/format.h>

double pf::window::Resolution::aspectRatio() const { return static_cast<double>(width) / height; }

std::ostream &pf::window::operator<<(std::ostream &os, const pf::window::Resolution &res) {
  os << fmt::format("{}x{}", res.width, res.height);
  return os;
}

pf::window::WindowData::WindowData(const pf::window::WindowSettings &settings)
    : resolution(settings.resolution), title(settings.title), mode(settings.mode) {}

const pf::window::Resolution &pf::window::WindowData::getResolution() const {
  return WindowData::resolution;
}

void pf::window::WindowData::setResolution(const pf::window::Resolution &res) {
  WindowData::resolution = res;
}

const std::string &pf::window::WindowData::getTitle() const { return title; }

void pf::window::WindowData::setTitle(const std::string &tit) { WindowData::title = tit; }

pf::window::Mode pf::window::WindowData::getMode() const { return mode; }

void pf::window::WindowData::setMode(window::Mode mode) { WindowData::mode = mode; }

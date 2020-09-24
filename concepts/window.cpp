//
// Created by petr on 9/24/20.
//
#include "window.h"
#include <fmt/format.h>

double window::resolution_t::aspect_ratio() const { return static_cast<double>(width) / height; }

std::ostream &window::operator<<(std::ostream &os, const window::resolution_t &res) {
  os << fmt::format("{}x{}", res.width, res.height);
  return os;
}

window::window_data::window_data(const window::window_settings &settings)
    : resolution(settings.resolution), title(settings.title), mode(settings.mode) {}

const window::resolution_t &window::window_data::get_resolution() const {
  return window_data::resolution;
}

void window::window_data::set_resolution(const window::resolution_t &res) {
  window_data::resolution = res;
}

const std::string &window::window_data::get_title() const { return title; }

void window::window_data::set_title(const std::string &tit) { window_data::title = tit; }

window::mode_t window::window_data::get_mode() const { return mode; }

void window::window_data::set_mode(window::mode_t mod) { window_data::mode = mod; }

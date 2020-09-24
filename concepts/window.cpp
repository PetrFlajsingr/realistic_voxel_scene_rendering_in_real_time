//
// Created by petr on 9/24/20.
//
#include "window.h"
#include <fmt/format.h>

double pf::window::resolution_t::aspect_ratio() const { return static_cast<double>(width) / height; }

std::ostream &pf::window::operator<<(std::ostream &os, const pf::window::resolution_t &res) {
  os << fmt::format("{}x{}", res.width, res.height);
  return os;
}

pf::window::window_data::window_data(const pf::window::window_settings &settings)
    : resolution(settings.resolution), title(settings.title), mode(settings.mode) {}

const pf::window::resolution_t &pf::window::window_data::get_resolution() const {
  return window_data::resolution;
}

void pf::window::window_data::set_resolution(const pf::window::resolution_t &res) {
  window_data::resolution = res;
}

const std::string &pf::window::window_data::get_title() const { return title; }

void pf::window::window_data::set_title(const std::string &tit) { window_data::title = tit; }

pf::window::mode_t pf::window::window_data::get_mode() const { return mode; }

void pf::window::window_data::set_mode(window::mode_t mod) { window_data::mode = mod; }

//
// Created by petr on 9/24/20.
//

#include "regex_check_action.h"
#include <fmt/format.h>

regex_check_action::regex_check_action(std::regex regex) : regex(std::move(regex)) {}

regex_check_action::regex_check_action(std::string_view regex_str)
    : regex(std::string(regex_str)) {}

std::string_view regex_check_action::operator()(std::string_view arg) {
  if (!std::regex_match(std::string(arg), regex)) {
    throw std::runtime_error(fmt::format("Argument '{}' doesn't match regex.", arg));
  }
  return arg;
}

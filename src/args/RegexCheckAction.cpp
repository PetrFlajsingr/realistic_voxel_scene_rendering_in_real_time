//
// Created by petr on 9/24/20.
//

#include "RegexCheckAction.h"
#include <fmt/format.h>

RegexCheckAction::RegexCheckAction(std::regex regex) : rgx(std::move(regex)) {}

RegexCheckAction::RegexCheckAction(std::string_view regexStr) : rgx(std::string(regexStr)) {}

std::string_view RegexCheckAction::operator()(std::string_view arg) {
  if (!std::regex_match(std::string(arg), rgx)) {
    throw std::runtime_error(fmt::format("Argument '{}' doesn't match regex.", arg));
  }
  return arg;
}

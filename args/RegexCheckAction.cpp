//
// Created by petr on 9/24/20.
//

#include "RegexCheckAction.h"
#include <fmt/format.h>

RegexCheckAction::RegexCheckAction(std::regex regex) : regex(std::move(regex)) {}

RegexCheckAction::RegexCheckAction(std::string_view regexStr) : regex(std::string(regexStr)) {}

std::string_view RegexCheckAction::operator()(std::string_view arg) {
  if (!std::regex_match(std::string(arg), regex)) {
    throw std::runtime_error(fmt::format("Argument '{}' doesn't match regex.", arg));
  }
  return arg;
}

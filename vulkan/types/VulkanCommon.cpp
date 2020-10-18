//
// Created by petr on 10/18/20.
//

#include "VulkanCommon.h"
#include <range/v3/numeric/accumulate.hpp>
#include <range/v3/view.hpp>
#include <regex>

using namespace ranges::views;
using namespace std::string_literals;

template<ranges::bidirectional_range T>
requires std::same_as<ranges::range_value_t<T>, char> uint32_t charViewToUint(T &&range) {
  auto d = 1u;
  const auto charToUintOrThrow = [](auto ch) {
    if (!std::isdigit(ch)) { throw std::runtime_error{"Invalid char in uint conversion: "s + ch}; }
    return ch - '0';
  };
  const auto accumulateDigits = [&d](auto a, auto b) {
    const auto result = a + b * d;
    d *= 10;
    return result;
  };
  return ranges::accumulate(range | transform(charToUintOrThrow) | reverse, 0u, accumulateDigits);
}

namespace pf::vulkan {
std::vector<uint32_t> splitVersionString(const char *verStr) {
  const auto versionRegex = std::regex(R"(\d+\.\d+\.\d+)");
  if (!std::regex_match(verStr, versionRegex)) {
    throw std::runtime_error("Invalid version literal: "s + verStr);
  }
  return c_str(verStr) | split('.')
      | transform([](const auto &chars) { return charViewToUint(chars | ranges::to_vector); })
      | ranges::to_vector;
}

Version literals::operator""_v(const char *verStr, std::size_t) {
  auto result = Version();
  const auto parts = splitVersionString(verStr);
  result.major = parts[0];
  result.minor = parts[1];
  result.patch = parts[2];
  return result;
}

}// namespace pf::vulkan
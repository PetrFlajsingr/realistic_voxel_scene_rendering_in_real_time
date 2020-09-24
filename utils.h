//
// Created by petr on 9/23/20.
//

#ifndef REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_UTILS_H
#define REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_UTILS_H

#include <algorithm>
#include <tuple>

template<typename T, typename Container = std::initializer_list<T>>
bool is_one_of(const T &val, Container &&vals) {
  return std::ranges::any_of(vals, [&val](const auto &v) { return val == v; });
}

namespace details {
template<typename F, typename T, std::size_t... Index>
void iterate_tuple_impl(F &&action, T const &tup, std::index_sequence<Index...> const &) {
  [[maybe_unused]] bool ignore[] = {(action(std::get<Index>(tup)), true)...};
}
template<typename F, typename T, std::size_t... Index>
void iterate_tuple_pairs_impl(F &&action, T const &tup, std::index_sequence<Index...> const &) {
  [[maybe_unused]] bool ignore[] = {
      (action(std::get<Index>(tup), std::get<Index + 1>(tup)), true)...};
}
}// namespace details

template<typename F, typename... Obj>
void iterate_tuple(F &&action, std::tuple<Obj...> const &tup) {
  details::iterate_tuple_impl(action, tup, std::make_index_sequence<sizeof...(Obj)>());
}

template<typename F, typename... Obj>
void iterate_tuple_pairs(F &&action, std::tuple<Obj...> const &tup) {
  details::iterate_tuple_pairs_impl(action, tup, std::make_index_sequence<sizeof...(Obj) - 1>());
}

#endif//REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_UTILS_H

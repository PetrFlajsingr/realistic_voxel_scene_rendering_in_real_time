//
// Created by petr on 9/23/20.
//

#ifndef REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_UTILS_H
#define REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_UTILS_H

#include <algorithm>
#include <tuple>

template<typename T, typename Container = std::initializer_list<T>>
bool isIn(const T &val, Container &&vals) {
  return std::ranges::any_of(vals, [&val](const auto &v) { return val == v; });
}

template<typename T, template<class> typename Container,
         template<class> typename Container2>
std::optional<T> findFirstCommon(const Container<T> &vals, const Container2<T> &vals2) {
  for (const auto &val : vals) {
    if (isIn(val, vals2)) { return val; }
  }
  return std::nullopt;
}

namespace details {
template<typename F, typename T, std::size_t... Index>
void iterateTupleImpl(F &&action, T const &tup, std::index_sequence<Index...> const &) {
  [[maybe_unused]] bool ignore[] = {(action(std::get<Index>(tup)), true)...};
}
template<typename F, typename T, std::size_t... Index>
void iterateTuplePairsImpl(F &&action, T const &tup, std::index_sequence<Index...> const &) {
  [[maybe_unused]] bool ignore[] = {
      (action(std::get<Index>(tup), std::get<Index + 1>(tup)), true)...};
}
}// namespace details

template<typename F, typename... Obj>
void iterateTuple(F &&action, std::tuple<Obj...> const &tup) {
  details::iterateTupleImpl(action, tup, std::make_index_sequence<sizeof...(Obj)>());
}

template<typename F, typename... Obj>
void iterateTuplePairs(F &&action, std::tuple<Obj...> const &tup) {
  details::iterateTuplePairsImpl(action, tup, std::make_index_sequence<sizeof...(Obj) - 1>());
}

#endif//REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_UTILS_H

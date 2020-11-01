//
// Created by petr on 10/26/20.
//

#ifndef VOXEL_RENDER_ALGORITHMS_H
#define VOXEL_RENDER_ALGORITHMS_H

#include <algorithm>

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

#endif//VOXEL_RENDER_ALGORITHMS_H
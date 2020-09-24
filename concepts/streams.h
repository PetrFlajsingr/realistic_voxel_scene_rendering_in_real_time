//
// Created by petr on 9/24/20.
//

#ifndef REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_STREAMS_H
#define REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_STREAMS_H
#include <ostream>

template<typename T>
concept stream_outputable = requires(T t, std::ostream o) {
  { o << t }
  ->std::convertible_to<std::ostream &>;
};
#endif//REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_STREAMS_H

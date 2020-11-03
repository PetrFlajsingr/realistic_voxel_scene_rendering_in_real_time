//
// Created by petr on 11/1/20.
//

#ifndef REALISTIC_VOXEL_RENDERING_UTILS_VISITOR_H
#define REALISTIC_VOXEL_RENDERING_UTILS_VISITOR_H

template<typename... Ts>
struct Visitor : Ts... { using Ts::operator()...; };
template<class... Ts>
Visitor(Ts...) -> Visitor<Ts...>;

#endif//REALISTIC_VOXEL_RENDERING_UTILS_VISITOR_H
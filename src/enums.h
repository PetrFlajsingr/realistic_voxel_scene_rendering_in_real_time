//
// Created by petr on 6/15/21.
//

#ifndef REALISTIC_VOXEL_RENDERING_SRC_ENUMS_H
#define REALISTIC_VOXEL_RENDERING_SRC_ENUMS_H

#include <pf_common/enums.h>

namespace pf {
enum class SVOViewType : int { Color = 0, Normals, Iterations, Distance, ChildIndex, TreeLevel };

enum class GBufferViewType : uint { Disabled = 0, Color, Normal, Depth, Shaded };

enum class ProbeVisualisation : uint32_t { Disabled = 0, Color = 1, Normals = 2, Depth = 3, CameraView = 4 };
// TODO: figure out why PF_ENUM_OUT doesn't work
inline std::ostream &operator<<(std::ostream &o, SVOViewType viewType) {
  o << magic_enum::enum_name(viewType);
  return o;
}
inline std::ostream &operator<<(std::ostream &o, ProbeVisualisation viewType) {
  o << magic_enum::enum_name(viewType);
  return o;
}

inline std::ostream &operator<<(std::ostream &o, GBufferViewType viewType) {
  o << magic_enum::enum_name(viewType);
  return o;
}

}

#endif//REALISTIC_VOXEL_RENDERING_SRC_ENUMS_H

/**
 * @file enums.h
 * @brief Enums used in different parts of the application.
 * @author Petr Flajšingr
 * @date 15.6.21
 */

#ifndef REALISTIC_VOXEL_RENDERING_SRC_ENUMS_H
#define REALISTIC_VOXEL_RENDERING_SRC_ENUMS_H

#include <pf_common/enums.h>

namespace pf {
/**
 * @brief Render type for SVO debug.
 */
enum class SVOViewType : int { Color = 0, Normals, Iterations, Distance, ChildIndex, TreeLevel };

/**
 * @brief Render type for GBuffer debug.
 */
enum class GBufferViewType : uint { Disabled = 0, Color, Normal, Depth, Shaded };

/**
 * @brief Render type for probe atlas visualisation.
 */
enum class ProbeVisualisation : uint32_t { Disabled = 0, Color = 1, Normals = 2, Depth = 3, CameraView = 4 };

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

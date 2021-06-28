//
// Created by petr on 6/26/21.
//

#ifndef REALISTIC_VOXEL_RENDERING_SRC_VOXEL_MATERIALS_H
#define REALISTIC_VOXEL_RENDERING_SRC_VOXEL_MATERIALS_H

#include <glm/vec4.hpp>
#include <ogt_vox.h>

namespace pf::vox {
/**
 * Diffuse:
 *  - no params
 * Metal:
 *  - roughness <0, 100>
 *  - index of refraction <1.0, 3.0>
 *  - metallic <0, 100>
 * Emit:
 *  - emission <0, 100>
 *  - power <1, 5> ?
 *  - low dynamic range <0, 100>
 * Glass:
 *  - rougness <0, 100>
 *  - index of refraction <1.0, 3.0>
 *  - transparency <0, 100>
 * Blend:
 *  - roughness <0, 100>
 *  - index of refraction <1.0, 3.0>
 *  - metallic <0, 100>
 *  - transparency <0, 100>
 * Media and cloud unsupported
 */
enum class MatType : std::uint32_t { Diffuse = 0, Metal = 1, Glass = 2, Emit = 3, Blend = 4, Media = 5, Cloud = 6 };

struct MaterialProperties {
  MaterialProperties() = default;
  explicit MaterialProperties(const ogt_vox_matl &source, glm::vec4 col);
  MatType type;
  float metalness;
  float rougness;
  float specular;
  float indexOfRefraction;
  float flux;
  float emission;
  float lowDynamicRange;
  float transparency;
  float alpha;
  float density;
  glm::vec4 color;
};

}// namespace pf::vox

#endif//REALISTIC_VOXEL_RENDERING_SRC_VOXEL_MATERIALS_H

//
// Created by petr on 6/26/21.
//

#include "Materials.h"

pf::vox::MaterialProperties::MaterialProperties(const ogt_vox_matl &source, glm::vec4 col) {
  type = static_cast<MatType>(source.type);
  metalness = source.metal;
  rougness = source.rough;
  specular = source.spec;
  indexOfRefraction = 1.0f + source.ior; // .vox has +1 implicitly
  flux = source.flux;
  emission = source.emit;
  lowDynamicRange = source.ldr;
  transparency = source.trans;
  alpha = source.alpha;
  density = source.d;
  color = col;
}

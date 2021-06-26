//
// Created by petr on 6/26/21.
//

#include "Materials.h"

pf::vox::MaterialProperties::MaterialProperties(const ogt_vox_matl &source) {
  type = static_cast<MatType>(source.type);
  metalness = source.metal;
  rougness = source.rough;
  specular = source.spec;
  indexOfRefraction = source.ior;
  flux = source.flux;
  emission = source.emit;
  lowDynamicRange = source.ldr;
  transparency = source.trans;
  alpha = source.alpha;
  density = source.d;
}

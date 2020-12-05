//
// Created by petr on 12/5/20.
//

#include "RTTriangleRenderer.h"

namespace pf {

RTTriangleRenderer::RTTriangleRenderer(toml::table &tomlConfig)
    : config(tomlConfig), camera({0, 0}) {}

void RTTriangleRenderer::render() {
  throw NotImplementedException("render not implemented");
}

}
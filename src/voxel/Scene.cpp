//
// Created by petr on 12/7/20.
//

#include "Scene.h"

#include <algorithm>
#include <pf_common/exceptions/StackTraceException.h>
#include <utility>

namespace pf::vox {
Scene::Scene(std::string name, std::vector<std::unique_ptr<Model>> models)
    : name(std::move(name)), models(std::move(models)) {}

const std::string &Scene::getName() const { return name; }

const std::vector<std::unique_ptr<Model>> &Scene::getModels() const { return models; }

Model &Scene::getModelByName(std::string_view modelName) {
  const auto modelNamePredicate = [modelName](const auto &model) { return model->getName() == modelName; };
  if (const auto iter = std::ranges::find_if(models, modelNamePredicate); iter != models.end()) { return **iter; }
  throw StackTraceException::fmt("Model not found: {}", modelName);
}

}// namespace pf::vox
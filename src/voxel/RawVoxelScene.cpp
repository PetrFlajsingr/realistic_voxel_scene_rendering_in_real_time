//
// Created by petr on 12/7/20.
//

#include "RawVoxelScene.h"

#include <algorithm>
#include <pf_common/exceptions/StackTraceException.h>
#include <utility>

namespace pf::vox {
RawVoxelScene::RawVoxelScene(std::string name, std::vector<std::unique_ptr<RawVoxelModel>> models)
    : name(std::move(name)), models(std::move(models)) {}

const std::string &RawVoxelScene::getName() const { return name; }

const std::vector<std::unique_ptr<RawVoxelModel>> &RawVoxelScene::getModels() const { return models; }

RawVoxelModel &RawVoxelScene::getModelByName(std::string_view modelName) {
  const auto modelNamePredicate = [modelName](const auto &model) { return model->getName() == modelName; };
  if (const auto iter = std::ranges::find_if(models, modelNamePredicate); iter != models.end()) { return **iter; }
  throw StackTraceException("Model not found: {}", modelName);
}

}// namespace pf::vox
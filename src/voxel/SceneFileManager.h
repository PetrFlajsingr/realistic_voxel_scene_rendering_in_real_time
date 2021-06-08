//
// Created by petr on 6/6/21.
//

#ifndef REALISTIC_VOXEL_RENDERING_SRC_UTILS_SCENEFILEMANAGER_H
#define REALISTIC_VOXEL_RENDERING_SRC_UTILS_SCENEFILEMANAGER_H

#include "GPUModelInfo.h"
#include <filesystem>
#include <optional>
#include <ranges>
#include <string>
#include <vector>

namespace pf::vox {
/**
 * File format:
 *
 *
 *
 *
 */

std::optional<std::string>
saveSceneToFile(std::ranges::range auto &&models, const std::filesystem::path &path) requires(
    std::same_as<std::ranges::range_value_t<decltype(models)>, GPUModelInfo>) {
  auto tomlData = toml::array{};
  for (const auto &model : models) { tomlData.template push_back(model.toToml()); }
  auto ostream = std::ofstream(path);
  auto tomlTable = toml::table{};
  tomlTable.insert("models", tomlData);
  ostream << tomlTable;

  return std::nullopt;
}

[[nodiscard]] inline std::vector<GPUModelInfo> loadSceneFromFile(const std::filesystem::path &path) {
  auto tomlData = toml::parse_file(path.string());
  auto tomlArray = tomlData["models"].as_array();
  auto result = std::vector<GPUModelInfo>{};
  for (const auto &record : *tomlArray) {
    auto recTable = record.as_table();
    result.emplace_back().fromToml(*recTable);
  }
  return result;
}

}// namespace pf::vox
#endif//REALISTIC_VOXEL_RENDERING_SRC_UTILS_SCENEFILEMANAGER_H

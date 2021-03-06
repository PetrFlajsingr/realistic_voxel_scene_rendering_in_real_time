/**
 * @file SceneFileManager.h
 * @brief Functions to save nad load scene files.
 * @author Petr Flajšingr
 * @date 6.6.21
 */

#ifndef REALISTIC_VOXEL_RENDERING_SRC_UTILS_SCENEFILEMANAGER_H
#define REALISTIC_VOXEL_RENDERING_SRC_UTILS_SCENEFILEMANAGER_H

#include "GPUModelInfo.h"
#include <filesystem>
#include <optional>
#include <pf_imgui/serialization.h>
#include <ranges>
#include <string>
#include <vector>

namespace pf::vox {
/**
 * Save model infos as one scene, including their transformations and probe grid information.
 * @param models models to save
 * @param path output path
 * @param probeGridPos probe grid position
 * @param probeGridStep probe grid step
 * @param proximityGridSize proximity grid size
 * @return error string if error occurred
 */
std::optional<std::string> saveSceneToFile(
    std::ranges::range auto &&models, const std::filesystem::path &path, glm::vec3 probeGridPos, float probeGridStep,
    glm::ivec3 proximityGridSize) requires(std::same_as<std::ranges::range_value_t<decltype(models)>, GPUModelInfo>) {
  auto tomlData = toml::array{};
  for (const auto &model : models) { tomlData.template push_back(model.toToml()); }
  auto ostream = std::ofstream(path);
  auto tomlTable = toml::table{};
  tomlTable.insert("models", tomlData);
  tomlTable.insert("probeGridPos", ui::ig::serializeGlmVec(probeGridPos));
  tomlTable.insert("proximityGridSize", ui::ig::serializeGlmVec(proximityGridSize));
  tomlTable.insert("probeGridStep", probeGridStep);
  ostream << tomlTable;

  return std::nullopt;
}
/**
 * @brief Information about a scene loaded from a file.
 */
struct SceneFileInfo {
  std::vector<GPUModelInfo> models;
  glm::vec3 probeGridPos;
  float probeGridStep;
  glm::ivec3 proximityGridSize;
};
/**
 * Load scene information from given file
 * @param path path to scene describing file
 * @return information about the scene
 */
[[nodiscard]] inline SceneFileInfo loadSceneFromFile(const std::filesystem::path &path) {
  auto result = SceneFileInfo{};
  auto tomlData = toml::parse_file(path.string());
  auto tomlArray = tomlData["models"].as_array();
  auto models = std::vector<GPUModelInfo>{};
  for (const auto &record : *tomlArray) {
    auto recTable = record.as_table();
    models.emplace_back().fromToml(*recTable);
  }
  result.models = std::move(models);
  result.probeGridPos = ui::ig::deserializeGlmVec<glm::vec3>(*tomlData["probeGridPos"].as_array());
  result.probeGridStep = *tomlData["probeGridStep"].value<float>();
  result.proximityGridSize = ui::ig::deserializeGlmVec<glm::ivec3>(*tomlData["proximityGridSize"].as_array());
  return result;
}

}// namespace pf::vox
#endif//REALISTIC_VOXEL_RENDERING_SRC_UTILS_SCENEFILEMANAGER_H

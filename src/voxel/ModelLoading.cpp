//
// Created by petr on 12/7/20.
//

#include "ModelLoading.h"
#include "logging/loggers.h"
#include <magic_enum.hpp>
#include <pf_common/RAII.h>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
#define OGT_VOX_IMPLEMENTATION
#include "ogt_vox.h"
#pragma GCC diagnostic pop

#include <algorithm>
#include <fstream>
#include <glm/vec3.hpp>
#include <numeric>
#include <ranges>

namespace pf::vox {

RawVoxelScene loadScene(const std::filesystem::path &srcFile, FileType fileType) {
  if (fileType == FileType::Unknown) {
    const auto detectedFileType = details::detectFileType(srcFile);
    if (!detectedFileType.has_value()) { throw LoadException("Could not detect file type for '{}'", srcFile.string()); }
    fileType = *detectedFileType;
  }
  auto ifstream = std::ifstream(srcFile, std::ios::binary);
  if (!ifstream.is_open()) { throw LoadException("Could not load model '{}', can't open file", srcFile.string()); }
  switch (fileType) {
    case FileType::Vox: return details::loadVoxScene(std::move(ifstream)); break;
    default:
      throw LoadException("Could not load model '{}', unsupported format: {}", srcFile.string(),
                          magic_enum::enum_name(fileType));
  }
}

std::optional<FileType> details::detectFileType(const std::filesystem::path &srcFile) {
  if (srcFile.extension().string() == ".vox") { return FileType::Vox; }
  if (srcFile.extension().string() == ".pf_vox") { return FileType::PfVox; }
  return std::nullopt;
}

RawVoxelScene details::loadVoxScene(std::ifstream &&istream) {
  const auto fileData = std::vector<uint8_t>(std::istreambuf_iterator(istream), {});
  const auto ogtScene = ogt_vox_read_scene(fileData.data(), fileData.size());
  const auto freeScene = RAII{[&] { ogt_vox_destroy_scene(ogtScene); }};

  const auto ogtModels = std::span{ogtScene->models, ogtScene->num_models};
  const auto ogtInstances = std::span{ogtScene->instances, ogtScene->num_instances};

  struct ModelWithTransform {
    const ogt_vox_model *model;
    const char *name;
    glm::vec3 translate;
  };

  auto ogtModelsWithTransform =
      ogtInstances | std::views::transform([&ogtModels](auto instance) {
        return ModelWithTransform{ogtModels[instance.model_index], instance.name,
                                  glm::vec3{instance.transform.m30, instance.transform.m31, instance.transform.m32}};
      })
      | ranges::to_vector;

  [[maybe_unused]] const auto minModelOffset = [&] {
    auto minCoords = glm::vec3{std::numeric_limits<float>::max(), std::numeric_limits<float>::max(),
                               std::numeric_limits<float>::max()};
    std::ranges::for_each(ogtModelsWithTransform, [&minCoords](const auto &modelInfo) {
      const auto translateWithCenterOffset = modelInfo.translate
          - glm::vec3{modelInfo.model->size_x / 2, modelInfo.model->size_y / 2, modelInfo.model->size_z / 2};
      if (minCoords.x > translateWithCenterOffset.x) { minCoords.x = translateWithCenterOffset.x; }
      if (minCoords.y > translateWithCenterOffset.y) { minCoords.y = translateWithCenterOffset.y; }
      if (minCoords.z > translateWithCenterOffset.z) { minCoords.z = translateWithCenterOffset.z; }
    });
    return minCoords;
  }();

  [[maybe_unused]] const auto maxModelOffset = [&] {
    auto maxCoords = glm::vec3{std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest(),
                               std::numeric_limits<float>::lowest()};
    std::ranges::for_each(ogtModelsWithTransform, [&maxCoords](const auto &modelInfo) {
      const auto translateWithCenterOffset = modelInfo.translate
          - glm::vec3{modelInfo.model->size_x / 2, modelInfo.model->size_y / 2, modelInfo.model->size_z / 2};
      if (maxCoords.x < translateWithCenterOffset.x) {
        maxCoords.x = translateWithCenterOffset.x + modelInfo.model->size_x;
      }
      if (maxCoords.y < translateWithCenterOffset.y) {
        maxCoords.y = translateWithCenterOffset.y + modelInfo.model->size_y;
      }
      if (maxCoords.z < translateWithCenterOffset.z) {
        maxCoords.z = translateWithCenterOffset.z + modelInfo.model->size_z;
      }
    });
    return maxCoords;
  }();

  std::ranges::for_each(ogtModelsWithTransform,
                        [minModelOffset](auto &modelInfo) { modelInfo.translate -= minModelOffset; });

  auto models = std::vector<std::unique_ptr<RawVoxelModel>>();
  models.reserve(ogtScene->num_models);

  for (const auto [idx, ogtModel] : ranges::views::enumerate(ogtModelsWithTransform)) {
    const auto volSize = ogtModel.model->size_x * ogtModel.model->size_y * ogtModel.model->size_z;
    const auto modelCenter =
        glm::ivec3{ogtModel.model->size_x / 2, ogtModel.model->size_y / 2, ogtModel.model->size_z / 2};
    const auto ogtVoxels = std::span{ogtModel.model->voxel_data, volSize};

    auto currentPos = glm::vec3{0, 0, 0};

    auto movePos = [&currentPos, ogtModel] {
      ++currentPos.x;
      if (currentPos.x == ogtModel.model->size_x) {
        currentPos.x = 0;
        ++currentPos.z;
        if (currentPos.z == ogtModel.model->size_y) {
          currentPos.z = 0;
          ++currentPos.y;
        }
      }
    };

    auto voxels = std::vector<VoxelInfo>{};
    voxels.reserve(volSize);
    for (const auto ogtVoxel : ogtVoxels) {
      if (ogtVoxel != 0) {
        const auto ogtColor = ogtScene->palette.color[ogtVoxel];
        voxels.emplace_back(
            glm::vec4{currentPos + ogtModel.translate.xzy() - glm::vec3{modelCenter}.xzy(), 0},
            //glm::vec4{currentPos + ogtModel.translate.yxz(), 0},
            glm::vec4{ogtColor.r / 255.0f, ogtColor.g / 255.0f, ogtColor.b / 255.0f, ogtColor.a / 255.0f});
      }
      movePos();
    }
    voxels.shrink_to_fit();
    models.emplace_back(std::make_unique<RawVoxelModel>(
        ogtModel.name == nullptr ? std::to_string(idx) : ogtModel.name, std::move(voxels),
        glm::ivec3{ogtModel.model->size_x, ogtModel.model->size_y, ogtModel.model->size_z}));
  }

  return RawVoxelScene("vox scene", std::move(models), minModelOffset);
}
}// namespace pf::vox
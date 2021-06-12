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
// TODO: load it normally and then just flip y axis for each voxel AND chunk
RawVoxelScene details::loadVoxScene(std::ifstream &&istream) {
  const auto fileData = std::vector<uint8_t>(std::istreambuf_iterator(istream), {});
  const auto ogtScene = ogt_vox_read_scene(fileData.data(), fileData.size());
  const auto freeScene = RAII{[&] { ogt_vox_destroy_scene(ogtScene); }};

  const auto ogtModels = std::span{ogtScene->models, ogtScene->num_models};
  const auto ogtInstances = std::span{ogtScene->instances, ogtScene->num_instances};

  struct ModelWithTransform {
    const ogt_vox_model *model;
    glm::vec3 translate;
  };

  auto ogtModelsWithTransform =
      ogtInstances | std::views::transform([&ogtModels](auto instance) {
        return ModelWithTransform{ogtModels[instance.model_index],
                                  glm::vec3{instance.transform.m30, instance.transform.m31, instance.transform.m32}};
      })
      | ranges::to_vector;

  [[maybe_unused]] const auto minModelOffset = [&] {
    auto minCoords = glm::vec3{std::numeric_limits<float>::max(), std::numeric_limits<float>::max(),
                               std::numeric_limits<float>::max()};
    std::ranges::for_each(ogtModelsWithTransform, [&minCoords](const auto &modelInfo) {
      if (minCoords.x > modelInfo.translate.x) { minCoords.x = modelInfo.translate.x; }
      if (minCoords.y > modelInfo.translate.y) { minCoords.y = modelInfo.translate.y; }
      if (minCoords.z > modelInfo.translate.z) { minCoords.z = modelInfo.translate.z; }
    });
    return minCoords;
  }();

  [[maybe_unused]] const auto maxModelOffset = [&] {
    auto maxCoords = glm::vec3{std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest(),
                               std::numeric_limits<float>::lowest()};
    std::ranges::for_each(ogtModelsWithTransform, [&maxCoords](const auto &modelInfo) {
      if (maxCoords.x < modelInfo.translate.x) { maxCoords.x = modelInfo.translate.x + modelInfo.model->size_x; }
      if (maxCoords.y < modelInfo.translate.y) { maxCoords.y = modelInfo.translate.y + modelInfo.model->size_y; }
      if (maxCoords.z < modelInfo.translate.z) { maxCoords.z = modelInfo.translate.z + modelInfo.model->size_z; }
    });
    return maxCoords;
  }();

  const auto height = maxModelOffset.z - minModelOffset.z;
  std::ranges::for_each(ogtModelsWithTransform, [minModelOffset, height](auto &modelInfo) {
    modelInfo.translate -= minModelOffset;
    modelInfo.translate.z *= -1;
    modelInfo.translate.z += height;
  });

  //const auto height = maxModelOffset.z - minModelOffset.z;
  //std::ranges::for_each(ogtModelsWithTransform, [minModelOffset, height](auto &modelInfo) {
  //  modelInfo.translate -= minModelOffset;
  //  modelInfo.translate.x *= -1;
  //  modelInfo.translate.x += height;
  //});

  auto models = std::vector<std::unique_ptr<RawVoxelModel>>();
  models.reserve(ogtScene->num_models);

  for (const auto [idx, ogtModel] : ranges::views::enumerate(ogtModelsWithTransform)) {
    const auto volSize = ogtModel.model->size_x * ogtModel.model->size_y * ogtModel.model->size_z;
    const auto ogtVoxels = std::span{ogtModel.model->voxel_data, volSize};

    auto currentPos = glm::vec3{0, ogtModel.model->size_z - 1, 0};

    auto movePos = [&currentPos, ogtModel] {
      ++currentPos.x;
      if (currentPos.x == ogtModel.model->size_x) {
        currentPos.x = 0;
        ++currentPos.z;
        if (currentPos.z == ogtModel.model->size_y) {
          currentPos.z = 0;
          --currentPos.y;
        }
      }
    };

    auto voxels = std::vector<VoxelInfo>{};
    voxels.reserve(volSize);
    for (const auto ogtVoxel : ogtVoxels) {
      if (ogtVoxel != 0) {
        const auto ogtColor = ogtScene->palette.color[ogtVoxel];
        voxels.emplace_back(
            glm::vec4{currentPos + ogtModel.translate.xzy(), 0},
            //glm::vec4{currentPos + ogtModel.translate.yxz(), 0},
            glm::vec4{ogtColor.r / 255.0f, ogtColor.g / 255.0f, ogtColor.b / 255.0f, ogtColor.a / 255.0f});
      }
      movePos();
    }
    voxels.shrink_to_fit();
    models.emplace_back(std::make_unique<RawVoxelModel>(std::to_string(idx), std::move(voxels)));
  }

 /* code for exports from FileToVox
  * const auto fileData = std::vector<uint8_t>(std::istreambuf_iterator(istream), {});
  const auto ogtScene = ogt_vox_read_scene(fileData.data(), fileData.size());
  const auto freeScene = RAII{[&] { ogt_vox_destroy_scene(ogtScene); }};

  const auto ogtModels = std::span{ogtScene->models, ogtScene->num_models};
  const auto ogtInstances = std::span{ogtScene->instances, ogtScene->num_instances};

  struct ModelWithTransform {
    const ogt_vox_model *model;
    glm::vec3 translate;
  };

  auto ogtModelsWithTransform =
      ogtInstances | std::views::transform([&ogtModels](auto instance) {
        return ModelWithTransform{ogtModels[instance.model_index],
                                  glm::vec3{instance.transform.m30, instance.transform.m31, instance.transform.m32}};
      })
          | ranges::to_vector;

  [[maybe_unused]] const auto minModelOffset = [&] {
    auto minCoords = glm::vec3{std::numeric_limits<float>::max(), std::numeric_limits<float>::max(),
                               std::numeric_limits<float>::max()};
    std::ranges::for_each(ogtModelsWithTransform, [&minCoords](const auto &modelInfo) {
      if (minCoords.x > modelInfo.translate.x) { minCoords.x = modelInfo.translate.x; }
      if (minCoords.y > modelInfo.translate.y) { minCoords.y = modelInfo.translate.y; }
      if (minCoords.z > modelInfo.translate.z) { minCoords.z = modelInfo.translate.z; }
    });
    return minCoords;
  }();

  [[maybe_unused]] const auto maxModelOffset = [&] {
    auto maxCoords = glm::vec3{std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest(),
                               std::numeric_limits<float>::lowest()};
    std::ranges::for_each(ogtModelsWithTransform, [&maxCoords](const auto &modelInfo) {
      if (maxCoords.x < modelInfo.translate.x) { maxCoords.x = modelInfo.translate.x + modelInfo.model->size_x; }
      if (maxCoords.y < modelInfo.translate.y) { maxCoords.y = modelInfo.translate.y + modelInfo.model->size_y; }
      if (maxCoords.z < modelInfo.translate.z) { maxCoords.z = modelInfo.translate.z + modelInfo.model->size_z; }
    });
    return maxCoords;
  }();

  const auto height = maxModelOffset.z - minModelOffset.z;

  std::ranges::for_each(ogtModelsWithTransform, [minModelOffset, height](auto &modelInfo) {
    modelInfo.translate -= minModelOffset;
    modelInfo.translate.x *= -1;
    modelInfo.translate.x += height;
  });

  auto models = std::vector<std::unique_ptr<Model>>();
  models.reserve(ogtScene->num_models);

  for (const auto [idx, ogtModel] : ranges::views::enumerate(ogtModelsWithTransform)) {
    const auto volSize = ogtModel.model->size_x * ogtModel.model->size_y * ogtModel.model->size_z;
    const auto ogtVoxels = std::span{ogtModel.model->voxel_data, volSize};

    auto currentPos = glm::vec3{0, 0, ogtModel.model->size_z - 1};

    auto movePos = [&currentPos, ogtModel] {
      ++currentPos.x;
      if (currentPos.x == ogtModel.model->size_x) {
        currentPos.x = 0;
        ++currentPos.y;
        if (currentPos.y == ogtModel.model->size_y) {
          currentPos.y = 0;
          --currentPos.z;
        }
      }
    };

    auto voxels = std::vector<Voxel>{};
    voxels.reserve(volSize);
    for (const auto ogtVoxel : ogtVoxels) {
      if (ogtVoxel != 0) {
        const auto ogtColor = ogtScene->palette.color[ogtVoxel];
        voxels.emplace_back(
            glm::vec4{currentPos + glm::vec3{ogtModel.translate.y, ogtModel.translate.z, ogtModel.translate.x}, 0},
            glm::vec4{ogtColor.r / 255.0f, ogtColor.g / 255.0f, ogtColor.b / 255.0f, ogtColor.a / 255.0f});
      }
      movePos();
    }
    voxels.shrink_to_fit();
    models.emplace_back(std::make_unique<Model>(std::to_string(idx), std::move(voxels)));
  }*/

  return RawVoxelScene("vox scene", std::move(models));
}
}// namespace pf::vox
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

#include <glm/vec3.hpp>

namespace pf::vox {
LoadException::LoadException(std::string_view message) : StackTraceException(message) {}

Scene loadScene(const std::filesystem::path &srcFile, FileType fileType) {
  if (fileType == FileType::Unknown) {
    const auto detectedFileType = details::detectFileType(srcFile);
    if (!detectedFileType.has_value()) {
      throw LoadException("Could not detect file type for '{}'", srcFile.string());
    }
    fileType = *detectedFileType;
  }
  auto ifstream = std::ifstream(srcFile, std::ios::binary);
  if (!ifstream.is_open()) {
    throw LoadException("Could not load model '{}', can't open file", srcFile.string());
  }
  switch (fileType) {
    case FileType::Vox: return details::loadVoxScene(std::move(ifstream)); break;
    default:
      throw LoadException("Could not load model '{}', unsupported format: {}", srcFile.string(),
                          magic_enum::enum_name(fileType));
  }
}

std::optional<FileType> details::detectFileType(const std::filesystem::path &srcFile) {
  if (srcFile.extension().string() == ".vox") { return FileType::Vox; }
  return std::nullopt;
}

Scene details::loadVoxScene(std::ifstream &&istream) {
  const auto fileData = std::vector<uint8_t>(std::istreambuf_iterator(istream), {});
  const auto ogtScene = ogt_vox_read_scene(fileData.data(), fileData.size());
  const auto freeScene = RAII{[&] { ogt_vox_destroy_scene(ogtScene); }};

  const auto ogtModels = std::span{ogtScene->models, ogtScene->num_models};
  auto models = std::vector<std::unique_ptr<Model>>();
  models.reserve(ogtScene->num_models);

  for (const auto [idx, ogtModel] : ranges::views::enumerate(ogtModels)) {
    const auto volSize = ogtModel->size_x * ogtModel->size_y * ogtModel->size_z;
    const auto ogtVoxels = std::span{ogtModel->voxel_data, volSize};

    auto currentPos = glm::vec3{0, ogtModel->size_z, 0};

    auto movePos = [&currentPos, ogtModel] {
      ++currentPos.x;
      if (currentPos.x == ogtModel->size_x) {
        currentPos.x = 0;
        ++currentPos.z;
        if (currentPos.z == ogtModel->size_y) {
          currentPos.z = 0;
          --currentPos.y;
        }
      }
    };

    auto voxels = std::vector<Voxel>{};
    voxels.reserve(volSize);
    for (const auto ogtVoxel : ogtVoxels) {
      if (ogtVoxel != 0) {
        const auto ogtColor = ogtScene->palette.color[ogtVoxel];
        voxels.emplace_back(glm::vec4{currentPos, 0},
                            glm::vec4{ogtColor.r / 255.0f, ogtColor.g / 255.0f, ogtColor.b / 255.0f,
                                      ogtColor.a / 255.0f});
      }
      movePos();
    }
    voxels.shrink_to_fit();
    models.emplace_back(std::make_unique<Model>(std::to_string(idx), std::move(voxels)));
  }

  return Scene("vox scene", std::move(models));
}
}// namespace pf::vox
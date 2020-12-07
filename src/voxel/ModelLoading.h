//
// Created by petr on 12/7/20.
//

#ifndef REALISTIC_VOXEL_RENDERING_SRC_VOXEL_MODELLOADER_H
#define REALISTIC_VOXEL_RENDERING_SRC_VOXEL_MODELLOADER_H

#include "Scene.h"
#include <filesystem>
#include <pf_common/exceptions/StackTraceException.h>

namespace pf::vox {

class LoadException : public StackTraceException {
 public:
  explicit LoadException(std::string_view message);
  explicit LoadException(std::string_view fmt, auto &&...args)
      : LoadException(fmt::format(fmt, args...)) {}
};

enum class FileType { Vox, Unknown };

Scene loadScene(const std::filesystem::path &srcFile, FileType fileType = FileType::Unknown);

namespace details {
std::optional<FileType> detectFileType(const std::filesystem::path &srcFile);

Scene loadVoxScene(std::ifstream &&istream);
}// namespace details

}// namespace pf::vox::load
#endif//REALISTIC_VOXEL_RENDERING_SRC_VOXEL_MODELLOADER_H

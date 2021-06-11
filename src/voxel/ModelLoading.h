//
// Created by petr on 12/7/20.
//

#ifndef REALISTIC_VOXEL_RENDERING_SRC_VOXEL_MODELLOADER_H
#define REALISTIC_VOXEL_RENDERING_SRC_VOXEL_MODELLOADER_H

#include "RawVoxelScene.h"
#include <filesystem>
#include <pf_common/exceptions/StackTraceException.h>

namespace pf::vox {

class LoadException : public StackTraceException {
 public:
  explicit LoadException(std::string_view fmt, auto &&...args) : StackTraceException(fmt::format(fmt, args...)) {}
};

enum class FileType { Vox, Unknown };

RawVoxelScene loadScene(const std::filesystem::path &srcFile, FileType fileType = FileType::Unknown);

namespace details {
std::optional<FileType> detectFileType(const std::filesystem::path &srcFile);

RawVoxelScene loadVoxScene(std::ifstream &&istream);
}// namespace details

}// namespace pf::vox
#endif//REALISTIC_VOXEL_RENDERING_SRC_VOXEL_MODELLOADER_H

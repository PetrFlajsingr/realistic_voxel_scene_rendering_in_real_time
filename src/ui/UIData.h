/**
 * @file UIData.h
 * @brief Structs for UI.
 * @author Petr Flajšingr
 * @date 30.6.21
 */

#ifndef REALISTIC_VOXEL_RENDERING_SRC_UI_UIDATA_H
#define REALISTIC_VOXEL_RENDERING_SRC_UI_UIDATA_H

#include <experimental/memory>
#include <filesystem>
#include <pf_common/coroutines/Sequence.h>
#include <pf_glfw_vulkan/vulkan/types/Image.h>
#include <pf_glfw_vulkan/vulkan/types/ImageView.h>
#include <pf_glfw_vulkan/vulkan/types/TextureSampler.h>
#include <string>
#include <voxel/GPUModelInfo.h>

namespace pf {
/**
 * @brief Texture data required for ui::ig::Image.
 */
struct TextureData {
  vulkan::Image &vkImage;
  vulkan::ImageView &vkImageView;
  vulkan::TextureSampler &vkImageSampler;
};
/**
 * @brief Info about model file.
 */
struct ModelFileInfo {
  // TODO: group id and group controls
  ModelFileInfo() = default;
  explicit inline ModelFileInfo(std::filesystem::path path) : path(std::move(path)) {}
  static inline auto IdGenerator = iota<std::size_t>();
  std::experimental::observer_ptr<vox::GPUModelInfo> modelData = nullptr;
  std::size_t id = getNext(IdGenerator);
  std::filesystem::path path{};
  inline bool operator==(const ModelFileInfo &rhs) const { return id == rhs.id; }
  inline bool operator!=(const ModelFileInfo &rhs) const { return !(rhs == *this); }
  inline friend std::ostream &operator<<(std::ostream &os, const ModelFileInfo &info) {
    os << info.path.filename().string();
    if (info.modelData != nullptr) { os << std::to_string(info.id); }
    return os;
  }
};

}// namespace pf

#endif//REALISTIC_VOXEL_RENDERING_SRC_UI_UIDATA_H

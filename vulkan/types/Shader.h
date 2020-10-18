//
// Created by petr on 9/28/20.
//

#ifndef REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_SHADER_H
#define REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_SHADER_H
#include "../concepts/PtrConstructable.h"
#include "fwd.h"
#include "VulkanObject.h"
#include <istream>
#include <vulkan/vulkan.hpp>

namespace pf::vulkan {
// TODO: more shader types
enum class ShaderType { Vertex, Fragment, Compute };

struct ShaderConfigFile {
  std::string name;
  ShaderType type;
  std::string path;
};

struct ShaderConfigSrc {
  std::string name;
  ShaderType type;
  std::vector<uint8_t> data;
};

struct ShaderConfigStringSrc {
  std::string name;
  ShaderType type;
  std::string src;
};

vk::ShaderStageFlagBits ShaderTypeToVk(ShaderType type);

class Shader : public VulkanObject, public PtrConstructable<Shader> {
 public:
  explicit Shader(std::shared_ptr<LogicalDevice> device, const ShaderConfigFile &config);
  explicit Shader(std::shared_ptr<LogicalDevice> device, const ShaderConfigSrc &config);
  explicit Shader(std::shared_ptr<LogicalDevice> device, const ShaderConfigStringSrc &config);

  Shader(const Shader &other) = delete;
  Shader &operator=(const Shader &other) = delete;

  [[nodiscard]] const vk::ShaderModule &getShaderModule();
  [[nodiscard]] ShaderType getType() const;
  [[nodiscard]] vk::ShaderStageFlagBits getVkType() const;
  [[nodiscard]] const std::string &getName() const;
  const vk::ShaderModule &operator*() const;

  vk::ShaderModule const *operator->() const;

  [[nodiscard]] std::string info() const override;

 private:
  std::shared_ptr<LogicalDevice> logicalDevice;
  vk::UniqueShaderModule vkShader;
  ShaderType type;
  std::string name;
};

std::vector<uint8_t> readSpvFile(std::istream &istream);
std::vector<uint8_t> readSpvFile(std::istream &&istream);

}// namespace pf::vulkan

#endif//REALISTIC_VOXEL_SCENE_RENDERING_IN_REAL_TIME_SHADER_H

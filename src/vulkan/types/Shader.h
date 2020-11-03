//
// Created by petr on 9/28/20.
//

#ifndef VOXEL_RENDER_SHADER_H
#define VOXEL_RENDER_SHADER_H
#include <pf_common/concepts/PtrConstructible.h>
#include "../glsl/Compiler.h"
#include "VulkanObject.h"
#include "fwd.h"
#include <istream>
#include <vulkan/vulkan.hpp>

namespace pf::vulkan {
// TODO: more shader types
enum class ShaderType {
  Vertex,
  Fragment,
  Compute,
  Geometry,
  TessControl,
  TessEval,
  RayGen,
  AnyHit,
  ClosestHit,
  Miss,
  Intersection,
  Callable,
  Task,
  Mesg
};

shaderc_shader_kind toShaderc(ShaderType type);

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

struct ShaderConfigGlslFile {
  std::string name;
  ShaderType type;
  std::string path;
  glsl::MacroDefs macros;
  glsl::ReplaceMacroDefs replaceMacros = {};
  glsl::Optimization optimization = {};
};

struct ShaderConfigGlslSrc {
  std::string name;
  ShaderType type;
  std::string src;
  glsl::MacroDefs macros;
  glsl::ReplaceMacroDefs replaceMacros = {};
  glsl::Optimization optimization = {};
};

vk::ShaderStageFlagBits ShaderTypeToVk(ShaderType type);

class Shader : public VulkanObject, public PtrConstructible<Shader> {
 public:
  explicit Shader(std::shared_ptr<LogicalDevice> device, const ShaderConfigFile &config);
  explicit Shader(std::shared_ptr<LogicalDevice> device, const ShaderConfigSrc &config);
  explicit Shader(std::shared_ptr<LogicalDevice> device, const ShaderConfigGlslSrc &config);
  explicit Shader(std::shared_ptr<LogicalDevice> device, const ShaderConfigGlslFile &config);

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

std::vector<uint8_t> readSpvFile(std::ifstream &istream);
std::vector<uint8_t> readSpvFile(std::ifstream &&istream);

}// namespace pf::vulkan

#endif//VOXEL_RENDER_SHADER_H

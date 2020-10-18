//
// Created by petr on 9/28/20.
//

#include "Shader.h"
#include "PhysicalDevice.h"
#include <fmt/format.h>
#include <fstream>
#include <magic_enum.hpp>

namespace pf::vulkan {
vk::ShaderStageFlagBits ShaderTypeToVk(ShaderType type) {
  switch (type) {
    case ShaderType::Vertex: return vk::ShaderStageFlagBits::eVertex;
    case ShaderType::Fragment: return vk::ShaderStageFlagBits::eFragment;
    case ShaderType::Compute: return vk::ShaderStageFlagBits::eCompute;
    default: throw std::runtime_error("shader type not covered");
  }
}

std::vector<uint8_t> readSpvFile(std::istream &istream) {
  const auto fileSize = istream.tellg();
  auto buffer = std::vector<uint8_t>(fileSize);
  istream.seekg(0);
  istream.read(reinterpret_cast<char *>(buffer.data()), fileSize);
  return buffer;
}

std::vector<uint8_t> readSpvFile(std::istream &&istream) { return readSpvFile(istream); }

shaderc_shader_kind toShaderc(ShaderType type) {
  switch (type) {
    case ShaderType::Vertex: return shaderc_vertex_shader;
    case ShaderType::Fragment: return shaderc_fragment_shader;
    case ShaderType::Compute: return shaderc_compute_shader;
    case ShaderType::Geometry: return shaderc_geometry_shader;
    case ShaderType::TessControl: return shaderc_tess_control_shader;
    case ShaderType::TessEval: return shaderc_tess_evaluation_shader;
    case ShaderType::RayGen: return shaderc_raygen_shader;
    case ShaderType::AnyHit: return shaderc_anyhit_shader;
    case ShaderType::ClosestHit: return shaderc_closesthit_shader;
    case ShaderType::Miss: return shaderc_miss_shader;
    case ShaderType::Intersection: return shaderc_intersection_shader;
    case ShaderType::Callable: return shaderc_callable_shader;
    case ShaderType::Task: return shaderc_task_shader;
    case ShaderType::Mesg: return shaderc_mesh_shader;
  }
}

Shader::Shader(std::shared_ptr<LogicalDevice> device, const ShaderConfigFile &config)
    : Shader(std::move(device),
             ShaderConfigSrc{.name = config.name,
                             .type = config.type,
                             .data = readSpvFile(
                                 std::ifstream(config.path, std::ios::ate | std::ios::binary))}) {}

Shader::Shader(std::shared_ptr<LogicalDevice> device, const ShaderConfigSrc &config)
    : logicalDevice(std::move(device)) {
  auto createInfo = vk::ShaderModuleCreateInfo();
  createInfo.setCodeSize(config.data.size())
      .setPCode(reinterpret_cast<const uint32_t *>(config.data.data()));
  name = config.name;
  type = config.type;
  vkShader = logicalDevice->getVkLogicalDevice().createShaderModuleUnique(createInfo);
}

Shader::Shader(std::shared_ptr<LogicalDevice> device, const ShaderConfigStringSrc &config)
    : logicalDevice(std::move(device)) {
  name = config.name;
  type = config.type;
  auto compiler = glsl::Compiler(config.name, config.src, toShaderc(config.type), config.macros,
                                 config.replaceMacros);
  const auto binary = compiler.compile(config.optimization);
  auto createInfo = vk::ShaderModuleCreateInfo();
  createInfo.setCode(binary);
  vkShader = logicalDevice->getVkLogicalDevice().createShaderModuleUnique(createInfo);
}

const vk::ShaderModule &Shader::getShaderModule() { return vkShader.get(); }

ShaderType Shader::getType() const { return type; }

vk::ShaderStageFlagBits Shader::getVkType() const { return ShaderTypeToVk(type); }

const std::string &Shader::getName() const { return name; }

std::string Shader::info() const {
  return fmt::format("Vulkan shader unique name: {}, type: {}", name, magic_enum::enum_name(type));
}

const vk::ShaderModule &Shader::operator*() const { return *vkShader; }

vk::ShaderModule const *Shader::operator->() const { return &*vkShader; }

}// namespace pf::vulkan
//
// Created by petr on 9/28/20.
//

#include "Shader.h"
#include <fmt/format.h>
#include <fstream>
#include <magic_enum.hpp>

vk::ShaderStageFlagBits pf::vulkan::ShaderTypeToVk(pf::vulkan::ShaderType type) {
  switch (type) {
    case ShaderType::Vertex: return vk::ShaderStageFlagBits::eVertex;
    case ShaderType::Fragment: return vk::ShaderStageFlagBits::eFragment;
    case ShaderType::Compute: return vk::ShaderStageFlagBits::eCompute;
    default: throw std::runtime_error("shader type not covered");
  }
}

std::vector<uint8_t> pf::vulkan::readSpvFile(std::istream &istream) {
  const auto fileSize = istream.tellg();
  auto buffer = std::vector<uint8_t>(fileSize);
  istream.seekg(0);
  istream.read(reinterpret_cast<char *>(buffer.data()), fileSize);
  return buffer;
}

std::vector<uint8_t> pf::vulkan::readSpvFile(std::istream &&istream) {
  return readSpvFile(istream);
}

pf::vulkan::Shader::Shader(const pf::vulkan::ShaderConfigFile &config)
    : Shader(ShaderConfigSrc{
        .name = config.name,
        .type = config.type,
        .data = readSpvFile(std::ifstream(config.path, std::ios::ate | std::ios::binary)),
        .logicalDevice = config.logicalDevice}) {}

pf::vulkan::Shader::Shader(const pf::vulkan::ShaderConfigSrc &config) {
  auto create_info = vk::ShaderModuleCreateInfo();
  create_info.setCodeSize(config.data.size())
      .setPCode(reinterpret_cast<const uint32_t *>(config.data.data()));
  name = config.name;
  type = config.type;
  vkShader = config.logicalDevice.getVkLogicalDevice().createShaderModuleUnique(create_info);
}

const vk::ShaderModule &pf::vulkan::Shader::getShaderModule() { return vkShader.get(); }

pf::vulkan::ShaderType pf::vulkan::Shader::getType() const { return type; }

vk::ShaderStageFlagBits pf::vulkan::Shader::getVkType() const { return ShaderTypeToVk(type); }

const std::string &pf::vulkan::Shader::getName() const { return name; }
std::string pf::vulkan::Shader::info() const {
  return fmt::format("Vulkan shader unique name: {}, type: {}", name, magic_enum::enum_name(type));
}
const vk::ShaderModule &pf::vulkan::Shader::operator*() const { return *vkShader; }
vk::ShaderModule const *pf::vulkan::Shader::operator->() const { return &*vkShader; }

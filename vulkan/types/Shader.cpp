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

Shader::Shader(std::shared_ptr<LogicalDevice> device, const ShaderConfigFile &config)
    : Shader(std::move(device),
             ShaderConfigSrc{.name = config.name,
                             .type = config.type,
                             .data = readSpvFile(
                                 std::ifstream(config.path, std::ios::ate | std::ios::binary))}) {}

Shader::Shader(std::shared_ptr<LogicalDevice> device, const ShaderConfigSrc &config)
    : logicalDevice(std::move(device)) {
  auto create_info = vk::ShaderModuleCreateInfo();
  create_info.setCodeSize(config.data.size())
      .setPCode(reinterpret_cast<const uint32_t *>(config.data.data()));
  name = config.name;
  type = config.type;
  vkShader = logicalDevice->getVkLogicalDevice().createShaderModuleUnique(create_info);
}

Shader::Shader(std::shared_ptr<LogicalDevice> device, const ShaderConfigStringSrc &config)
    : logicalDevice(std::move(device)) {
  name = config.name;
  type = config.type;
  throw std::runtime_error("Not implemented");
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
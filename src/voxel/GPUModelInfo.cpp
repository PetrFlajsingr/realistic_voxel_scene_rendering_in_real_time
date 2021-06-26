//
// Created by petr on 6/3/21.
//

#include "GPUModelInfo.h"
#include <glm/gtx/quaternion.hpp>
#include <pf_imgui/serialization.h>
namespace pf::vox {

bool GPUModelInfo::operator==(const GPUModelInfo &rhs) const { return id == rhs.id; }

bool GPUModelInfo::operator!=(const GPUModelInfo &rhs) const { return !(rhs == *this); }

void GPUModelInfo::assignNewId() { id = getNext(IdGenerator); }

std::optional<std::uint32_t> GPUModelInfo::getModelIndex() const {
  if (modelInfoMemoryBlock == nullptr) { return std::nullopt; }
  return modelInfoMemoryBlock->getOffset() / MODEL_INFO_BLOCK_SIZE;
}

void GPUModelInfo::updateInfoToGPU() {
  const auto translateCenterMat = glm::translate(glm::mat4(1.f), center);
  const auto translateMat = glm::translate(glm::mat4(1.f), translateVec);
  const auto scaleMat = glm::scale(scaleVec);
  const auto quaternion = glm::quat{rotateVec};
  const auto rotateMat = glm::toMat4(quaternion);
  transformMatrix = translateMat * scaleMat * rotateMat * translateCenterMat;
  const auto invTransformMatrix = glm::inverse(transformMatrix);
  const std::uint32_t svoOffsetTmp = svoMemoryBlock->getOffset() / 4;
  const auto scaleBufferData = glm::vec4{/*1.f /*/ scaleVec, *reinterpret_cast<const float *>(&svoOffsetTmp)};

  auto mapping = modelInfoMemoryBlock->mapping();
  mapping.set(scaleBufferData);
  mapping.setRawOffset(transformMatrix, sizeof(glm::vec4));
  mapping.setRawOffset(invTransformMatrix, sizeof(glm::vec4) + sizeof(glm::mat4));
  const auto AABB1 = glm::vec4{AABB.p1, AABB.p2.x};
  const auto AABB2 = glm::vec4{AABB.p2.yz(), 0, 0};
  mapping.setRawOffset(AABB1, sizeof(glm::vec4) + sizeof(glm::mat4) * 2);
  mapping.setRawOffset(AABB2, sizeof(glm::vec4) + sizeof(glm::mat4) * 2 + sizeof(glm::vec4));
  const auto materialsOffset = materialsMemoryBlock->getOffset() / vox::ONE_MATERIAL_SIZE;
  mapping.setRawOffset(glm::ivec4(materialsOffset, 0, 0, 0),
                       sizeof(glm::vec4) + sizeof(glm::mat4) * 2 + sizeof(glm::vec4) * 2);
}

std::ostream &operator<<(std::ostream &os, const GPUModelInfo &info) {
  os << info.path.filename().string();
  return os;
}
toml::table GPUModelInfo::toToml() const {
  auto result = toml::table{};
  result.insert("path", path.string());
  result.insert("translateVec", ui::ig::serializeGlmVec(translateVec));
  result.insert("scaleVec", ui::ig::serializeGlmVec(scaleVec));
  result.insert("rotateVec", ui::ig::serializeGlmVec(rotateVec));
  return result;
}
void GPUModelInfo::fromToml(const toml::table &src) {
  path = *src["path"].value<std::string>();
  translateVec = ui::ig::deserializeGlmVec<glm::vec3>(*src["translateVec"].as_array());
  scaleVec = ui::ig::deserializeGlmVec<glm::vec3>(*src["scaleVec"].as_array());
  rotateVec = ui::ig::deserializeGlmVec<glm::vec3>(*src["rotateVec"].as_array());
}
}// namespace pf::vox
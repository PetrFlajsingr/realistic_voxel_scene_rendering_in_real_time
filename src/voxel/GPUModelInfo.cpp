//
// Created by petr on 6/3/21.
//

#include "GPUModelInfo.h"

namespace pf::vox {

bool GPUModelInfo::operator==(const GPUModelInfo &rhs) const { return id == rhs.id; }

bool GPUModelInfo::operator!=(const GPUModelInfo &rhs) const { return !(rhs == *this); }

void GPUModelInfo::assignNewId() { id = getNext(IdGenerator); }

std::optional<std::uint32_t> GPUModelInfo::getModelIndex() const {
  if (modelInfoMemoryBlock == nullptr) { return std::nullopt; }
  return modelInfoMemoryBlock->getOffset() / MODEL_INFO_BLOCK_SIZE;
}

void GPUModelInfo::updateInfoToGPU() {
  const auto translateCenterMat = glm::translate(glm::mat4(1.f), glm::vec3{0.5, 0.5, 0.5});
  const auto translateBackFromCenterMat = glm::inverse(translateCenterMat);
  const auto translateMat = glm::translate(glm::mat4(1.f), translateVec);
  const auto scaleMat = glm::scale(scaleVec);
  const auto rotateMatX = glm::rotate(rotateVec.x, glm::vec3{1, 0, 0});
  const auto rotateMatY = glm::rotate(rotateVec.y, glm::vec3{0, 1, 0});
  const auto rotateMatZ = glm::rotate(rotateVec.z, glm::vec3{0, 0, 1});
  const auto rotateMat = rotateMatX * rotateMatY * rotateMatZ;
  transformMatrix = translateMat * scaleMat * translateBackFromCenterMat * rotateMat * translateCenterMat;
  const auto invTransformMatrix = glm::inverse(translateCenterMat) * glm::inverse(rotateMat)
      * glm::inverse(translateBackFromCenterMat) * glm::inverse(scaleMat) * glm::inverse(translateMat);
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
}

std::ostream &operator<<(std::ostream &os, const GPUModelInfo &info) {
  os << info.path.filename().string();
  return os;
}
}// namespace pf::vox
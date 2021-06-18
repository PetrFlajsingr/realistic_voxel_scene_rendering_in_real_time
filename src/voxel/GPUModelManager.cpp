//
// Created by petr on 6/8/21.
//

#include "GPUModelManager.h"
#include "SVO_utils.h"
#include "SparseVoxelOctreeCreation.h"
#include <algorithm>
#include <mutex>
#include <utility>

namespace pf::vox {

GPUModelManager::GPUModelManager(std::shared_ptr<vulkan::BufferMemoryPool> svoMemoryPool,
                                 std::shared_ptr<vulkan::BufferMemoryPool> modelInfoMemoryPool,
                                 std::size_t defaultSvoHeightSize)
    : defaultSVOHeightSize(defaultSvoHeightSize), svoMemoryPool(std::move(svoMemoryPool)),
      modelInfoMemoryPool(std::move(modelInfoMemoryPool)) {}

tl::expected<std::vector<GPUModelManager::ModelPtr>, std::string>
GPUModelManager::loadModel(const std::filesystem::path &path, const Callbacks &callbacks, bool sceneAsOneSVO,
                           bool autoScale) {
  try {
    callbacks.progress(0);
    const auto svoCreate = loadFileAsSVO(path, sceneAsOneSVO);
    callbacks.progress(50);
    auto resultModels = std::vector<ModelPtr>{};
    auto cnt = 0.f;
    auto newModels = std::vector<std::unique_ptr<GPUModelInfo>>{};
    for (auto svo : svoCreate) {
      auto newModelInfo = std::make_unique<GPUModelInfo>();
      newModelInfo->path = path;
      newModelInfo->voxelCount = svo.initVoxelCount;
      newModelInfo->minimizedVoxelCount = svo.voxelCount;
      newModelInfo->svoHeight = svo.depth;
      newModelInfo->AABB = svo.AABB;
      newModelInfo->translateVec = glm::vec3{0, 0, 0};
      newModelInfo->scaleVec = glm::vec3{1, 1, 1};
      newModelInfo->rotateVec = glm::vec3{0, 0, 0};
      auto svoData = vox::SparseVoxelOctree{std::move(svo.data)};
      auto svoBlockResult = copySvoToMemoryBlock(svoData, *svoMemoryPool);

      auto modelInfoBlockResult = modelInfoMemoryPool->leaseMemory(vox::MODEL_INFO_BLOCK_SIZE);
      std::string err;
      if (!modelInfoBlockResult.has_value()) { err += modelInfoBlockResult.error(); }
      if (!svoBlockResult.has_value()) { err += modelInfoBlockResult.error(); }
      if (!err.empty()) { return tl::make_unexpected(err); }
      callbacks.progress(80);

      newModelInfo->svoMemoryBlock = std::make_shared<vulkan::BufferMemoryPool::Block>(std::move(*svoBlockResult));
      newModelInfo->modelInfoMemoryBlock =
          std::make_shared<vulkan::BufferMemoryPool::Block>(std::move(*modelInfoBlockResult));
      if (autoScale) {
        newModelInfo->scaleVec =
            glm::vec3{static_cast<float>(std::pow(2, svo.depth) / std::pow(2, defaultSVOHeightSize))};
      }
      //newModelInfo->center = glm::vec3{};
      newModelInfo->center = svo.center / static_cast<float>(std::pow(2, svo.depth));

      callbacks.progress(50 + cnt / svoCreate.size() * 100);
      resultModels.emplace_back(std::experimental::make_observer(newModelInfo.get()));
      newModels.emplace_back(std::move(newModelInfo));
    }
    auto lock = std::unique_lock{mutex};
    for (auto &newModel : newModels) { models.emplace_back(std::move(newModel)); }// TODO: <algorithm>
    callbacks.progress(100);
    return resultModels;
  } catch (const std::exception &e) {
    const auto excMessage = e.what();
    return tl::make_unexpected(excMessage);
  }
}
tl::expected<GPUModelManager::ModelPtr, std::string>
GPUModelManager::createModelInstance(GPUModelManager::ModelPtr model) {
  auto newItemResult = prepareDuplicate(model);
  if (!newItemResult.has_value()) { return tl::make_unexpected(newItemResult.error()); }
  auto newItem = std::move(newItemResult.value());

  newItem->svoMemoryBlock = model->svoMemoryBlock;

  auto lock = std::unique_lock{mutex};
  models.emplace_back(std::move(newItem));
  return std::experimental::make_observer(models.back().get());
}
tl::expected<GPUModelManager::ModelPtr, std::string> GPUModelManager::duplicateModel(GPUModelManager::ModelPtr model) {
  auto newItemResult = prepareDuplicate(model);
  if (!newItemResult.has_value()) { return tl::make_unexpected(newItemResult.error()); }
  auto newItem = std::move(newItemResult.value());

  auto svoBlockAllocResult = svoMemoryPool->leaseMemory(model->svoMemoryBlock->getSize());
  if (!svoBlockAllocResult.has_value()) { return tl::make_unexpected(svoBlockAllocResult.error()); }
  newItem->svoMemoryBlock = std::make_shared<vulkan::BufferMemoryPool::Block>(std::move(*svoBlockAllocResult));
  auto data = std::vector<std::byte>(model->svoMemoryBlock->getSize());
  std::ranges::copy(model->svoMemoryBlock->mapping().data<std::byte>(), data.begin());
  newItem->svoMemoryBlock->mapping().set(data);

  auto lock = std::unique_lock{mutex};
  models.emplace_back(std::move(newItem));
  return std::experimental::make_observer(models.back().get());
}
tl::expected<std::unique_ptr<GPUModelInfo>, std::string>
GPUModelManager::prepareDuplicate(GPUModelManager::ModelPtr original) {
  auto newItem = std::make_unique<GPUModelInfo>();
  newItem->path = original->path;
  newItem->svoHeight = original->svoHeight;
  newItem->voxelCount = original->voxelCount;
  newItem->minimizedVoxelCount = original->minimizedVoxelCount;
  newItem->translateVec = original->translateVec;
  newItem->scaleVec = original->scaleVec;
  newItem->rotateVec = original->rotateVec;
  newItem->AABB = original->AABB;
  newItem->transformMatrix = original->transformMatrix;
  auto modelBlockAllocResult = modelInfoMemoryPool->leaseMemory(original->modelInfoMemoryBlock->getSize());
  auto err = std::string{};
  if (!modelBlockAllocResult.has_value()) { err += modelBlockAllocResult.error(); }
  if (!err.empty()) { return tl::make_unexpected(err); }

  newItem->modelInfoMemoryBlock = std::make_shared<vulkan::BufferMemoryPool::Block>(std::move(*modelBlockAllocResult));
  return newItem;
}
void GPUModelManager::removeModel(GPUModelManager::ModelPtr toRemove) {
  models.erase(std::ranges::find_if(models, [toRemove](const auto &model) { return model.get() == toRemove.get(); }));
}
const BVHCreateInfo &GPUModelManager::rebuildBVH(bool createStats) {
  bvh = vox::createBVH(getModels(), createStats);
  return bvh;
}
const BVHCreateInfo &GPUModelManager::getBvh() const { return bvh; }

// FIXME
tl::expected<std::vector<GPUModelManager::ModelPtr>, std::string> GPUModelManager::loadModel(RawVoxelScene &scene,
                                                                                             bool autoScale) {
  const auto svoCreate = convertSceneToSVO(scene, true);
  auto resultModels = std::vector<ModelPtr>{};
  auto newModels = std::vector<std::unique_ptr<GPUModelInfo>>{};
  for (auto svo : svoCreate) {
    auto newModelInfo = std::make_unique<GPUModelInfo>();
    newModelInfo->path = "";
    newModelInfo->voxelCount = svo.initVoxelCount;
    newModelInfo->minimizedVoxelCount = svo.voxelCount;
    newModelInfo->svoHeight = svo.depth;
    newModelInfo->AABB = svo.AABB;
    newModelInfo->translateVec = glm::vec3{0, 0, 0};
    newModelInfo->scaleVec = glm::vec3{1, 1, 1};
    newModelInfo->rotateVec = glm::vec3{0, 0, 0};
    auto svoData = vox::SparseVoxelOctree{std::move(svo.data)};
    auto svoBlockResult = copySvoToMemoryBlock(svoData, *svoMemoryPool);

    auto modelInfoBlockResult = modelInfoMemoryPool->leaseMemory(vox::MODEL_INFO_BLOCK_SIZE);
    std::string err;
    if (!modelInfoBlockResult.has_value()) { err += modelInfoBlockResult.error(); }
    if (!svoBlockResult.has_value()) { err += modelInfoBlockResult.error(); }
    if (!err.empty()) { return tl::make_unexpected(err); }

    newModelInfo->svoMemoryBlock = std::make_shared<vulkan::BufferMemoryPool::Block>(std::move(*svoBlockResult));
    newModelInfo->modelInfoMemoryBlock =
        std::make_shared<vulkan::BufferMemoryPool::Block>(std::move(*modelInfoBlockResult));
    if (autoScale) {
      newModelInfo->scaleVec =
          glm::vec3{static_cast<float>(std::pow(2, svo.depth) / std::pow(2, defaultSVOHeightSize))};
    }
    newModelInfo->center = svo.center / static_cast<float>(std::pow(2, svo.depth));

    resultModels.emplace_back(std::experimental::make_observer(newModelInfo.get()));
    newModels.emplace_back(std::move(newModelInfo));
  }
  auto lock = std::unique_lock{mutex};
  for (auto &newModel : newModels) { models.emplace_back(std::move(newModel)); }// TODO: <algorithm>
  return resultModels;
}
tl::expected<GPUModelManager::ModelPtr, std::string> GPUModelManager::loadModel(RawVoxelModel &model, bool autoScale) {
  const auto svo = convertModelToSVO(model);
  auto resultModels = std::vector<ModelPtr>{};
  auto newModelInfo = std::make_unique<GPUModelInfo>();
  newModelInfo->path = "";
  newModelInfo->voxelCount = svo.initVoxelCount;
  newModelInfo->minimizedVoxelCount = svo.voxelCount;
  newModelInfo->svoHeight = svo.depth;
  newModelInfo->AABB = svo.AABB;
  newModelInfo->translateVec = glm::vec3{0, 0, 0};
  newModelInfo->scaleVec = glm::vec3{1, 1, 1};
  newModelInfo->rotateVec = glm::vec3{0, 0, 0};
  auto svoData = vox::SparseVoxelOctree{std::move(svo.data)};
  auto svoBlockResult = copySvoToMemoryBlock(svoData, *svoMemoryPool);

  auto modelInfoBlockResult = modelInfoMemoryPool->leaseMemory(vox::MODEL_INFO_BLOCK_SIZE);
  std::string err;
  if (!modelInfoBlockResult.has_value()) { err += modelInfoBlockResult.error(); }
  if (!svoBlockResult.has_value()) { err += modelInfoBlockResult.error(); }
  if (!err.empty()) { return tl::make_unexpected(err); }

  newModelInfo->svoMemoryBlock = std::make_shared<vulkan::BufferMemoryPool::Block>(std::move(*svoBlockResult));
  newModelInfo->modelInfoMemoryBlock =
      std::make_shared<vulkan::BufferMemoryPool::Block>(std::move(*modelInfoBlockResult));
  if (autoScale) {
    newModelInfo->scaleVec = glm::vec3{static_cast<float>(std::pow(2, svo.depth) / std::pow(2, defaultSVOHeightSize))};
  }
  newModelInfo->center = svo.center / static_cast<float>(std::pow(2, svo.depth));

  auto resultModel = std::experimental::make_observer(newModelInfo.get());
  auto lock = std::unique_lock{mutex};
  models.emplace_back(std::move(newModelInfo));
  return resultModel;
}
}// namespace pf::vox
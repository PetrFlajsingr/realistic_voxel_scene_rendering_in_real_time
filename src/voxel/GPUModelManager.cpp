//
// Created by petr on 6/8/21.
//

#include "GPUModelManager.h"
#include "SVO_utils.h"
#include "SparseVoxelOctreeCreation.h"
#include <algorithm>

namespace pf::vox {

GPUModelManager::GPUModelManager(const std::shared_ptr<vulkan::BufferMemoryPool<4>> &svoMemoryPool,
                                 const std::shared_ptr<vulkan::BufferMemoryPool<16>> &modelInfoMemoryPool,
                                 std::size_t defaultSvoHeightSize)
    : defaultSVOHeightSize(defaultSvoHeightSize), svoMemoryPool(svoMemoryPool),
      modelInfoMemoryPool(modelInfoMemoryPool) {}

tl::expected<GPUModelManager::ModelPtr, std::string>
GPUModelManager::loadModel(const std::filesystem::path &path, const Callbacks &callbacks, bool autoScale) {

  try {
    callbacks.progress(0);
    const auto svoCreate = loadFileAsSVO(path);
    callbacks.progress(50);
    auto newModelInfo = std::make_unique<GPUModelInfo>();
    newModelInfo->path = path;
    newModelInfo->voxelCount = svoCreate.second.initVoxelCount;
    newModelInfo->minimizedVoxelCount = svoCreate.second.voxelCount;
    newModelInfo->svoHeight = svoCreate.second.depth;
    newModelInfo->AABB = svoCreate.second.AABB;
    newModelInfo->translateVec = glm::vec3{0, 0, 0};
    newModelInfo->scaleVec = glm::vec3{1, 1, 1};
    newModelInfo->rotateVec = glm::vec3{0, 0, 0};
    callbacks.progress(50);
    auto svo = vox::SparseVoxelOctree{std::move(svoCreate.first)};
    auto svoBlockResult = copySvoToMemoryBlock(svo, *svoMemoryPool);
    callbacks.progress(70);

    auto modelInfoBlockResult = modelInfoMemoryPool->leaseMemory(vox::MODEL_INFO_BLOCK_SIZE);
    std::string err;
    if (!modelInfoBlockResult.has_value()) { err += modelInfoBlockResult.error(); }
    if (!svoBlockResult.has_value()) { err += modelInfoBlockResult.error(); }
    if (!err.empty()) { return tl::make_unexpected(err); }
    callbacks.progress(80);

    newModelInfo->svoMemoryBlock = std::make_shared<vulkan::BufferMemoryPool<4>::Block>(std::move(*svoBlockResult));
    newModelInfo->modelInfoMemoryBlock =
        std::make_shared<vulkan::BufferMemoryPool<16>::Block>(std::move(*modelInfoBlockResult));
    if (autoScale) {
      newModelInfo->scaleVec =
          glm::vec3{static_cast<float>(std::pow(2, svoCreate.second.depth) / std::pow(2, defaultSVOHeightSize))};
    }

    callbacks.progress(100);
    auto lock = std::unique_lock{mutex};
    models.emplace_back(std::move(newModelInfo));
    return std::experimental::make_observer(models.back().get());
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
  newItem->svoMemoryBlock = std::make_shared<vulkan::BufferMemoryPool<4>::Block>(std::move(*svoBlockAllocResult));
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

  newItem->modelInfoMemoryBlock =
      std::make_shared<vulkan::BufferMemoryPool<16>::Block>(std::move(*modelBlockAllocResult));
  return newItem;
}
void GPUModelManager::removeModel(GPUModelManager::ModelPtr toRemove) {
  models.erase(std::ranges::find_if(models, [toRemove](const auto &model) { return model.get() == toRemove.get(); }));
}
Tree<BVHData> GPUModelManager::buildBVH() { return vox::createBVH(getModels()); }
}// namespace pf::vox
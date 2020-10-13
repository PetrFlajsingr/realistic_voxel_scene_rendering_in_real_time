//
// Created by petr on 9/28/20.
//

#include "RenderPassBuilder.h"

pf::vulkan::details::AttachmentDescriptionBuilder::AttachmentDescriptionBuilder(
    std::string name, pf::vulkan::RenderPassBuilder &parent)
    : parent(parent), name(std::move(name)) {}
pf::vulkan::details::AttachmentDescriptionBuilder &
pf::vulkan::details::AttachmentDescriptionBuilder::format(vk::Format format) {
  description.setFormat(format);
  return *this;
}
pf::vulkan::details::AttachmentDescriptionBuilder &
pf::vulkan::details::AttachmentDescriptionBuilder::initialLayout(vk::ImageLayout layout) {
  description.setInitialLayout(layout);
  return *this;
}
pf::vulkan::details::AttachmentDescriptionBuilder &
pf::vulkan::details::AttachmentDescriptionBuilder::finalLayout(vk::ImageLayout layout) {
  description.setFinalLayout(layout);
  return *this;
}
pf::vulkan::details::AttachmentDescriptionBuilder &
pf::vulkan::details::AttachmentDescriptionBuilder::samples(vk::SampleCountFlagBits sampleCount) {
  description.setSamples(sampleCount);
  return *this;
}
pf::vulkan::details::AttachmentDescriptionBuilder &
pf::vulkan::details::AttachmentDescriptionBuilder::loadOp(vk::AttachmentLoadOp attLoadOp) {
  description.setLoadOp(attLoadOp);
  return *this;
}
pf::vulkan::details::AttachmentDescriptionBuilder &
pf::vulkan::details::AttachmentDescriptionBuilder::stencilLoadOp(vk::AttachmentLoadOp stLoadOp) {
  description.setStencilLoadOp(stLoadOp);
  return *this;
}
pf::vulkan::details::AttachmentDescriptionBuilder &
pf::vulkan::details::AttachmentDescriptionBuilder::stencilStoreOp(vk::AttachmentStoreOp stStoreOp) {
  description.setStencilStoreOp(stStoreOp);
  return *this;
}
pf::vulkan::details::AttachmentDescriptionBuilder &
pf::vulkan::details::AttachmentDescriptionBuilder::flags(
    vk::AttachmentDescriptionFlagBits descFlags) {
  description.setFlags(descFlags);
  return *this;
}
pf::vulkan::RenderPassBuilder &pf::vulkan::details::AttachmentDescriptionBuilder::attachmentDone() {
  parent.attachDescriptions[name] = description;
  return parent;
}
pf::vulkan::details::AttachmentDescriptionBuilder &
pf::vulkan::details::AttachmentDescriptionBuilder::storeOp(vk::AttachmentStoreOp attStoreOp) {
  description.setStoreOp(attStoreOp);
  return *this;
}
pf::vulkan::details::SubPassBuilder::SubPassBuilder(std::string name,
                                                    pf::vulkan::RenderPassBuilder &parent)
    : name(std::move(name)), parent(parent) {}
pf::vulkan::details::SubPassBuilder &
pf::vulkan::details::SubPassBuilder::flags(const vk::SubpassDescriptionFlags &flags_) {
  data.description.setFlags(flags_);
  return *this;
}
pf::vulkan::details::SubPassBuilder &
pf::vulkan::details::SubPassBuilder::colorAttachment(std::string attachmentName) {
  data.colorAttachmentNames.emplace_back(std::move(attachmentName));
  return *this;
}
pf::vulkan::details::SubPassBuilder &
pf::vulkan::details::SubPassBuilder::inputAttachment(std::string attachmentName) {
  data.inputAttachmentNames.emplace_back(std::move(attachmentName));
  return *this;
}
pf::vulkan::details::SubPassBuilder &
pf::vulkan::details::SubPassBuilder::depthStencilAttachment(std::string attachmentName) {
  data.depthStencilAttachmentName = std::move(attachmentName);
  return *this;
}
pf::vulkan::details::SubPassBuilder &
pf::vulkan::details::SubPassBuilder::resolveAttachment(std::string attachmentName) {
  data.resolveAttachmentNames.emplace_back(std::move(attachmentName));
  return *this;
}
pf::vulkan::details::SubPassBuilder &
pf::vulkan::details::SubPassBuilder::preserveAttachment(std::string attachmentName) {
  data.preserveAttachmentNames.emplace_back(std::move(attachmentName));
  return *this;
}
pf::vulkan::details::SubPassBuilder &
pf::vulkan::details::SubPassBuilder::pipelineBindPoint(vk::PipelineBindPoint bindPoint) {
  data.description.setPipelineBindPoint(bindPoint);
  return *this;
}
pf::vulkan::RenderPassBuilder &pf::vulkan::details::SubPassBuilder::subpassDone() {
  parent.subpasses[name] = std::move(data);
  return parent;
}
pf::vulkan::details::SubPassDependencyBuilder pf::vulkan::details::SubPassBuilder::dependency() {
  return SubPassDependencyBuilder(*this);
}
pf::vulkan::details::SubPassDependencyBuilder &
pf::vulkan::details::SubPassDependencyBuilder::srcSubpass(std::string name) {
  data.srcSubpassName = std::move(name);
  return *this;
}
pf::vulkan::details::SubPassDependencyBuilder &
pf::vulkan::details::SubPassDependencyBuilder::dstSubpass(std::string name) {
  data.dstSubpassName = std::move(name);
  return *this;
}
pf::vulkan::details::SubPassDependencyBuilder &
pf::vulkan::details::SubPassDependencyBuilder::srcStageMask(
    const vk::PipelineStageFlags &stageFlags) {
  data.dependency.setSrcStageMask(stageFlags);
  return *this;
}
pf::vulkan::details::SubPassDependencyBuilder &
pf::vulkan::details::SubPassDependencyBuilder::dstStageMask(
    const vk::PipelineStageFlags &stageFlags) {
  data.dependency.setDstStageMask(stageFlags);
  return *this;
}
pf::vulkan::details::SubPassDependencyBuilder &
pf::vulkan::details::SubPassDependencyBuilder::srcAccessFlags(const vk::AccessFlags &accessFlags) {
  data.dependency.setSrcAccessMask(accessFlags);
  return *this;
}
pf::vulkan::details::SubPassDependencyBuilder &
pf::vulkan::details::SubPassDependencyBuilder::dstAccessFlags(const vk::AccessFlags &accessFlags) {
  data.dependency.setDstAccessMask(accessFlags);
  return *this;
}
pf::vulkan::details::SubPassBuilder &
pf::vulkan::details::SubPassDependencyBuilder::dependencyDone() {
  parent.data.dependencies.emplace_back(std::move(data));
  return parent;
}
pf::vulkan::details::SubPassDependencyBuilder::SubPassDependencyBuilder(
    pf::vulkan::details::SubPassBuilder &parent)
    : parent(parent) {}

pf::vulkan::details::AttachmentDescriptionBuilder
pf::vulkan::RenderPassBuilder::attachment(std::string name) {
  return details::AttachmentDescriptionBuilder(std::move(name), *this);
}

std::pair<std::vector<std::string>, vk::UniqueRenderPass>
pf::vulkan::RenderPassBuilder::build(pf::vulkan::LogicalDevice &device) {
  using namespace ranges;

  auto attachmentRefs = std::map<std::string, vk::AttachmentReference>();
  for (auto [idx, pair] : views::enumerate(attachDescriptions)) {
    auto attachmentRef = vk::AttachmentReference();
    attachmentRef.setLayout(pair.second.finalLayout).setAttachment(idx);
    attachmentRefs[pair.first] = attachmentRef;
  }

  auto colorAttachmentNames =
      std::unordered_map<std::string, std::vector<vk::AttachmentReference>>();
  auto inputAttachmentNames =
      std::unordered_map<std::string, std::vector<vk::AttachmentReference>>();
  auto resolveAttachmentNames =
      std::unordered_map<std::string, std::vector<vk::AttachmentReference>>();
  auto preserveAttachmentNames = std::unordered_map<std::string, std::vector<uint32_t>>();
  auto depthStencilAttachmentName =
      std::unordered_map<std::string, std::optional<vk::AttachmentReference>>();
  auto subpassDependencies = std::vector<vk::SubpassDependency>();
  for (auto &[name, spDescr] : subpasses) {
    for (const auto &colorAtt : spDescr.colorAttachmentNames) {
      colorAttachmentNames[name].emplace_back(attachmentRefs[colorAtt]);
    }
    for (const auto &inputAtt : spDescr.inputAttachmentNames) {
      inputAttachmentNames[name].emplace_back(attachmentRefs[inputAtt]);
    }
    for (const auto &resolveAtt : spDescr.resolveAttachmentNames) {
      resolveAttachmentNames[name].emplace_back(attachmentRefs[resolveAtt]);
    }
    for (const auto &preserveAtt : spDescr.preserveAttachmentNames) {
      preserveAttachmentNames[name].emplace_back(attachmentRefs[preserveAtt].attachment);
    }
    if (spDescr.depthStencilAttachmentName.has_value()) {
      depthStencilAttachmentName[name] = attachmentRefs[spDescr.depthStencilAttachmentName.value()];
    }
    spDescr.description.setColorAttachments(colorAttachmentNames[name])
        .setResolveAttachments(resolveAttachmentNames[name])
        .setInputAttachments(inputAttachmentNames[name])
        .setPreserveAttachments(preserveAttachmentNames[name]);
    if (depthStencilAttachmentName[name].has_value()) {
      spDescr.description.setPDepthStencilAttachment(&*depthStencilAttachmentName[name]);
    }
    for (auto &dependency : spDescr.dependencies) {
      if (dependency.srcSubpassName.empty()) {
        dependency.dependency.setSrcSubpass(VK_SUBPASS_EXTERNAL);
      } else {
        if (const auto idx = getSubpassIdx(dependency.srcSubpassName); idx.has_value()) {
          dependency.dependency.setSrcSubpass(*idx);
        } else {
          throw VulkanException::fmt("Invalid subpass name: {}", dependency.srcSubpassName);
        }
      }
      if (const auto idx = getSubpassIdx(dependency.dstSubpassName); idx.has_value()) {
        dependency.dependency.setDstSubpass(*idx);
      } else {
        throw VulkanException::fmt("Invalid subpass name: {}", dependency.dstSubpassName);
      }
      subpassDependencies.emplace_back(dependency.dependency);
    }
  }
  const auto attachDescrVec = attachDescriptions | views::values | to_vector;
  const auto subpassDecrVec = subpasses | views::values
      | views::transform([](const auto &subpass) { return subpass.description; }) | to_vector;
  const auto subpassNames = subpasses | views::keys | to_vector;
  auto createInfo = vk::RenderPassCreateInfo();
  createInfo.setSubpasses(subpassDecrVec)
      .setAttachments(attachDescrVec)
      .setDependencies(subpassDependencies);

  return std::make_pair(subpassNames,
                        device.getVkLogicalDevice().createRenderPassUnique(createInfo));
}

std::unique_ptr<pf::vulkan::RenderPass>
pf::vulkan::RenderPassBuilder::buildUnique(pf::vulkan::LogicalDevice &device) {
  return RenderPass::CreateUnique(*this, device);
}

std::shared_ptr<pf::vulkan::RenderPass>
pf::vulkan::RenderPassBuilder::buildShared(pf::vulkan::LogicalDevice &device) {
  return RenderPass::CreateShared(*this, device);
}

std::optional<uint32_t> pf::vulkan::RenderPassBuilder::getSubpassIdx(const std::string &name) {
  for (const auto [idx, pair] : ranges::views::enumerate(subpasses)) {
    if (pair.first == name) { return idx; }
  }
  return std::nullopt;
}
pf::vulkan::details::SubPassBuilder pf::vulkan::RenderPassBuilder::subpass(std::string name) {
  return details::SubPassBuilder(std::move(name), *this);
}

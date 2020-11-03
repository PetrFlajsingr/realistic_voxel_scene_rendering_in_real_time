//
// Created by petr on 10/27/20.
//

#ifndef VOXEL_RENDER_IMGUIBASE_H
#define VOXEL_RENDER_IMGUIBASE_H

#include "utils/config.h"
#include "vulkan/types/CommandBuffer.h"
#include "elements/Dialog.h"
#include "elements/MenuBars.h"
#include "elements/interface/Container.h"
#include "elements/interface/SavableElement.h"
#include <imgui.h>

namespace pf::ui::ig {

class ImGuiInterface : public Container {
 public:
  explicit ImGuiInterface(ImGuiConfigFlags flags, TomlConfig tomlConfig);

  virtual void addToCommandBuffer(vulkan::CommandBufferRecording &recording) = 0;

  [[nodiscard]] ImGuiIO &getIo() const;

  std::shared_ptr<Dialog> createDialog(const std::string &elementName,
                                            const std::string &caption, Modal modal = Modal::Yes);

  [[nodiscard]] ImGuiAppMenuBar &getMenuBar();
  [[nodiscard]] bool hasMenuBar() const;

  [[nodiscard]] const toml::table &getConfig() const;

  void updateConfig();
  void setStateFromConfig();

 protected:
  std::optional<ImGuiAppMenuBar> menuBar = std::nullopt;

 private:
  static ImGuiIO &baseInit(ImGuiConfigFlags flags);
  ImGuiIO &io;

  TomlConfig config;
};

}// namespace pf::ui
#endif//VOXEL_RENDER_IMGUIBASE_H

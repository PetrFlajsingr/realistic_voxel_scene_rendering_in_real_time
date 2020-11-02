//
// Created by petr on 11/2/20.
//

#ifndef REALISTIC_VOXEL_RENDERING_UI_IMGUI_ELEMENTS_IMGUIMEMO_H
#define REALISTIC_VOXEL_RENDERING_UI_IMGUI_ELEMENTS_IMGUIMEMO_H

#include "ImGuiPanel.h"
#include "ImGuiText.h"
#include "interface/ImGuiCaptionedElement.h"
#include <vector>

namespace pf::ui {

class ImGuiMemo : public ImGuiCaptionedElement {
 public:
  ImGuiMemo(const std::string &elementName, const std::string &caption, float textAHeight = 0, bool buttonsEnabled = true,
            bool filterEnabled = true, const std::optional<std::size_t> &recordLimit = std::nullopt);

  [[nodiscard]] const std::vector<std::string> &getRecords() const;
  [[nodiscard]] std::string getText() const;

  void addRecord(std::string_view record);
  void removeRecord(std::size_t index);
  void clearRecords();

  [[nodiscard]] bool isButtonsEnabled() const;
  void setButtonsEnabled(bool buttonsEnabled);
  [[nodiscard]] bool isFilterEnabled() const;
  void setFilterEnabled(bool filterEnabled);
  [[nodiscard]] const std::optional<std::size_t> &getRecordLimit() const;
  void setRecordLimit(std::size_t limit);

 protected:
  void renderImpl() override;

 private:
  void rebuildPanel();
  void removeRecordsAboveLimit();

  std::unique_ptr<ImGuiPanel> controlsPanel = nullptr;
  ImGuiPanel textAreaPanel;
  bool buttonsEnabled;
  bool filterEnabled;
  bool rebuild = true;
  std::function<bool(std::string_view)> filterFnc = [](auto) { return true; };
  std::optional<std::size_t> recordLimit;
  std::vector<std::string> records;
  float textAreaHeight;
  std::shared_ptr<ImGuiText> textArea;
};
}// namespace pf::ui
#endif//REALISTIC_VOXEL_RENDERING_UI_IMGUI_ELEMENTS_IMGUIMEMO_H

//
// Created by petr on 11/2/20.
//

#include "ImGuiMemo.h"
#include "ImGuiButton.h"
#include "ImGuiInputText.h"
#include <range/v3/view.hpp>

namespace pf::ui {

ImGuiMemo::ImGuiMemo(const std::string &elementName, const std::string &caption, float textAHeight,
                     bool buttonsEnabled, bool filterEnabled,
                     const std::optional<std::size_t> &recordLimit)
    : ImGuiElement(elementName), ImGuiCaptionedElement(elementName, caption),
      textAreaPanel(elementName + "_memo_panel###", getCaption(), PanelLayout::Vertical,
                    ImVec2{0, textAHeight}),
      buttonsEnabled(buttonsEnabled), filterEnabled(filterEnabled), recordLimit(recordLimit),
      textAreaHeight(textAHeight) {}

void ImGuiMemo::renderImpl() {
  if (rebuild) { rebuildPanel(); }
  removeRecordsAboveLimit();
  textArea->setText(getText());
  ImGui::Text("%s", getCaption().c_str());
  if (controlsPanel != nullptr) { controlsPanel->render(); }
  ImGui::Separator();
  textAreaPanel.render();
  ImGui::Separator();
}

const std::vector<std::string> &ImGuiMemo::getRecords() const { return records; }

std::string ImGuiMemo::getText() const {
  return records | ranges::views::filter(filterFnc) | ranges::views::join('\n')
      | ranges::to<std::string>();
}

void ImGuiMemo::addRecord(std::string_view record) { records.emplace_back(std::string(record)); }

void ImGuiMemo::removeRecord(std::size_t index) {
  if (index >= records.size()) { return; }
  records.erase(records.begin() + index);
}

void ImGuiMemo::clearRecords() { records.clear(); }

void ImGuiMemo::rebuildPanel() {
  if (buttonsEnabled || filterEnabled) {
    controlsPanel =
        std::make_unique<ImGuiPanel>(getName() + "button_filter_panel", getCaption() + " controls",
                                     PanelLayout::Horizontal, ImVec2{0, 20});
    if (buttonsEnabled) {
      controlsPanel->createChild<ImGuiButton>(getName() + "clear_btn", "Clear")->setOnClick([this] {
        clearRecords();
      });
      controlsPanel->createChild<ImGuiButton>(getName() + "copy_btn", "Copy")->setOnClick([this] {
        ImGui::SetClipboardText(getText().c_str());
      });
    }
    if (filterEnabled) {
      controlsPanel->createChild<ImGuiInputText>(getName() + "filter_input", "Filter")
          ->addValueListener([this](std::string_view str) {
            const auto filterStr = std::string(str);
            filterFnc = [filterStr](std::string_view recordStr) {
              return recordStr.find(filterStr) != std::string::npos;
            };
          });
    }
  }
  textArea = textAreaPanel.createChild<ImGuiText>(getName() + "memo_text", "Memo");
  rebuild = false;
}
bool ImGuiMemo::isButtonsEnabled() const { return buttonsEnabled; }

void ImGuiMemo::setButtonsEnabled(bool enabled) {
  rebuild = true;
  buttonsEnabled = enabled;
}

bool ImGuiMemo::isFilterEnabled() const { return filterEnabled; }

void ImGuiMemo::setFilterEnabled(bool enabled) {
  rebuild = true;
  filterEnabled = enabled;
}

const std::optional<std::size_t> &ImGuiMemo::getRecordLimit() const { return recordLimit; }

void ImGuiMemo::setRecordLimit(std::size_t limit) { recordLimit = limit; }

void ImGuiMemo::removeRecordsAboveLimit() {
  if (recordLimit.has_value() && *recordLimit < records.size()) {
    const auto cntToRemove = records.size() - *recordLimit;
    records.erase(records.begin(), records.begin() + cntToRemove);
  }
}

}// namespace pf::ui
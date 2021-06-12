//
// Created by petr on 6/11/21.
//

#ifndef REALISTIC_VOXEL_RENDERING_SRC_UI_SVOCONVERTDIALOG_H
#define REALISTIC_VOXEL_RENDERING_SRC_UI_SVOCONVERTDIALOG_H

#include <filesystem>
#include <functional>
#include <pf_common/parallel/ThreadPool.h>
#include <pf_glfw_vulkan/ui/Window.h>
#include <pf_imgui/ImGuiInterface.h>
#include <pf_imgui/dialogs/FileDialog.h>
#include <pf_imgui/elements/Button.h>
#include <pf_imgui/elements/Text.h>
#include <range/v3/view/cache1.hpp>
#include <range/v3/view/join.hpp>
#include <range/v3/view/take.hpp>
#include <vector>

namespace pf {
// TODO:
void createSVOConvertWindow(ui::ig::ImGuiInterface &imgui, ui::Window &window, [[maybe_unused]] ThreadPool &threadPool,
                            [[maybe_unused]] std::invocable<std::vector<std::filesystem::path>> auto &&conversion) {
  using namespace ui::ig;
  if (imgui.windowByName("convert_svo_window").has_value()) { return; }
  auto &convertWindow = imgui.createWindow("convert_svo_window", "Convert svo to binary");
  convertWindow.setCloseable(true);
  convertWindow.setSize(Size{500, 400});
  auto &selectFilesButton = convertWindow.createChild<Button>("convert_svo_select_files_btn", "Select input files");
  auto &selectedFilesText = convertWindow.createChild<Text>("convert_svo_select_files_list", "Selected files:");
  auto &selectOutputAndConvertButton =
      convertWindow.createChild<Button>("convert_svo_select_output_btn", "Select output folder and convert");

  selectFilesButton.template addClickListener([&] {
    imgui.template openFileDialog(
        "Select files to convert", {FileExtensionSettings{{"vox"}, "Vox model", ImVec4{1, 0, 0, 1}}},
        [&selectedFilesText](const std::vector<std::filesystem::path> &selectedFiles) {
          constexpr auto SELECTED_FILES_TEMPLATE = "Selected files:\n{}";
          selectedFilesText.setText(SELECTED_FILES_TEMPLATE,
                                    selectedFiles | ranges::views::take(10)
                                        | ranges::views::transform([](const auto &path) { return path.string(); })
                                        | ranges::views::cache1 | ranges::views::join('\n')
                                        | ranges::to<std::string>());
        },
        [] {}, Size{500, 400}, ".", "", Modal::Yes, 500);
  });
  // progress bar
  selectOutputAndConvertButton.template addClickListener([&, conversion] {
    // do stuff
  });

  convertWindow.template addCloseListener(
      [&] { window.enqueue([&imgui, &convertWindow] { imgui.removeWindow(convertWindow.getName()); }); });
}

}// namespace pf
#endif//REALISTIC_VOXEL_RENDERING_SRC_UI_SVOCONVERTDIALOG_H

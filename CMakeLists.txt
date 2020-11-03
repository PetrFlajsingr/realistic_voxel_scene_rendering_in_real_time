cmake_minimum_required(VERSION 3.17)
include("cmake/HunterGate.cmake")
set(CPM_SOURCE_CACHE ${CMAKE_SOURCE_DIR}/.cache)
include(FetchContent)

set(CPM_DOWNLOAD_LOCATION "${CMAKE_SOURCE_DIR}/cmake/CPM.cmake")
set(CPM_DOWNLOAD_VERSION 0.27.4)

if (NOT (EXISTS ${CPM_DOWNLOAD_LOCATION} AND CPM_VERSION STREQUAL CPM_DOWNLOAD_VERSION))
    message(STATUS "Downloading CPM.cmake")
    file(DOWNLOAD https://github.com/TheLartians/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake ${CPM_DOWNLOAD_LOCATION})
endif ()

include(${CPM_DOWNLOAD_LOCATION})

function(print_all_vars)
    get_cmake_property(_variableNames VARIABLES)
    list(SORT _variableNames)
    foreach (_variableName ${_variableNames})
        message(STATUS "${_variableName}=${${_variableName}}")
    endforeach ()
endfunction()

HunterGate(
        URL "https://github.com/cpp-pm/hunter/archive/v0.23.269.tar.gz"
        SHA1 "64024b7b95b4c86d50ae05b926814448c93a70a0"
)

find_program(CCACHE_PROGRAM ccache)
if (CCACHE_PROGRAM)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
endif ()


project(
        realistic_voxel_rendering
        VERSION 0.1.0
)
add_compile_definitions(GLFW_INCLUDE_VULKAN
        GLM_FORCE_RADIANS
        GLM_FORCE_DEPTH_ZERO_TO_ONE
        GLM_ENABLE_EXPERIMENTAL
        GLM_FORCE_SWIZZLE
        VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE
        VULKAN_HPP_NO_NODISCARD_WARNINGS
        STB_IMAGE_IMPLEMENTATION
        STACKTRACE_VULKAN_REPORT
        )

include_directories(include)

set(CMAKE_CXX_STANDARD 20)
list(APPEND flags "-o3" "-fconcepts" "-fconcepts-diagnostics-depth=10" "-Werror=return-type" "-fcoroutines"
        "-Wall" "-Wextra" "-Werror" "-Wpedantic" "-Wno-unknown-pragmas" "-Wno-unused-function"
        "-Wpointer-arith" "-Wcast-qual" "-Wno-type-limits")

if (CMAKE_BUILD_TYPE MATCHES Debug)
    message(" ")
    message("CMAKE IN DEBUG MODE")
    message(" ")
elseif (CMAKE_BUILD_TYPE MATCHES Release)
    message(" ")
    message("CMAKE IN RELEASE MODE")
    message(" ")
    list(APPEND flags "-o3")
elseif (CMAKE_BUILD_TYPE MATCHES ASAN)
    message(" ")
    message("CMAKE IN ADDRESS SANITISE MODE")
    message(" ")
    list(APPEND flags "-fno-omit-frame-pointer" "-fsanitize=address")
elseif (CMAKE_BUILD_TYPE MATCHES LSAN)
    message(" ")
    message("CMAKE IN LEAK SANITISE MODE")
    message(" ")
    list(APPEND flags "-fno-omit-frame-pointer" "-fsanitize=leak")
elseif (CMAKE_BUILD_TYPE MATCHES UBSAN)
    message(" ")
    message("CMAKE IN UNDEFINED SANITISE MODE")
    message(" ")
    list(APPEND flags "-fsanitize=undefined")
elseif (CMAKE_BUILD_TYPE MATCHES TSAN)
    message(" ")
    message("CMAKE IN THREAD SANITISE MODE")
    message(" ")
    list(APPEND flags "-fsanitize=thread")
endif ()

find_package(Vulkan REQUIRED)
include_directories(${Vulkan_INCLUDE_DIRS})
set(VK_LAYER_PATH "/etc/vulkan/explicit_layer.d")
set(LD_LIBRARY_PATH "/usr/lib")

find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)
include_directories(${GLFW_INCLUDE_DIRS})

set(RANGE_V3_TESTS OFF)
set(SHADERC_SKIP_TESTS ON)
set(SPIRV_CROSS_ENABLE_TESTS OFF)

hunter_add_package(imgui)
find_package(imgui CONFIG REQUIRED)
hunter_add_package(fmt)
find_package(fmt CONFIG REQUIRED)
hunter_add_package(glm)
find_package(glm CONFIG REQUIRED)
hunter_add_package(spdlog)
find_package(spdlog CONFIG REQUIRED)
add_compile_definitions(SPDLOG_FMT_EXTERNAL)
hunter_add_package(stb)
find_package(stb CONFIG REQUIRED)

CPMAddPackage(
        NAME magic_enum
        GITHUB_REPOSITORY Neargye/magic_enum
        GIT_TAG v0.6.6
)
CPMAddPackage(
        NAME units
        GITHUB_REPOSITORY nholthaus/units
        GIT_TAG v2.3.1
)
CPMAddPackage(
        NAME backward_cpp
        GITHUB_REPOSITORY bombela/backward-cpp
        GIT_TAG v1.5
)
CPMAddPackage(
        NAME range-v3
        GITHUB_REPOSITORY ericniebler/range-v3
        GIT_TAG 0.11.0
)
CPMAddPackage(
        NAME nanobench
        GITHUB_REPOSITORY martinus/nanobench
        GIT_TAG v4.2.0
)
CPMAddPackage(
        NAME subprocess
        GITHUB_REPOSITORY benman64/subprocess
        GIT_TAG v0.4.0
)
CPMAddPackage(
        NAME argparse
        GITHUB_REPOSITORY p-ranav/argparse
        GIT_TAG v2.1
)
CPMAddPackage(
        NAME toml++
        GITHUB_REPOSITORY marzer/tomlplusplus
        GIT_TAG v2.2.0
)
include_directories(${toml++_SOURCE_DIR}/include)
CPMAddPackage(
        NAME chaiscript
        GITHUB_REPOSITORY ChaiScript/ChaiScript
        GIT_TAG v6.1.0
)
CPMAddPackage(
        NAME cppcoro
        GITHUB_REPOSITORY dutow/cppcoro
        GIT_TAG 4b1951ff3c25fea78198a5c0aabb50589946b694
)

include_directories(${cppcoro_SOURCE_DIR}/include)

CPMAddPackage(
        NAME glslang
        GITHUB_REPOSITORY KhronosGroup/glslang
        GIT_TAG master-tot
)
CPMAddPackage(
        NAME spirvheaders
        GITHUB_REPOSITORY KhronosGroup/SPIRV-Headers
        GIT_TAG 1.5.3.reservations1
)
CPMAddPackage(
        NAME spirvtools
        GITHUB_REPOSITORY KhronosGroup/SPIRV-Tools
        GIT_TAG v2020.4
)
CPMAddPackage(
        NAME spirvcross
        GITHUB_REPOSITORY KhronosGroup/SPIRV-Cross
        GIT_TAG 2020-09-17
)
CPMAddPackage(
        NAME shaderc
        GITHUB_REPOSITORY google/shaderc
        GIT_TAG v2020.3
)

if (chaiscript_ADDED)
    add_library(chaiscript INTERFACE IMPORTED)
    target_include_directories(chaiscript INTERFACE ${chaiscript_SOURCE_DIR}/include)
endif ()

include_directories("${CMAKE_SOURCE_DIR}/src")

add_executable(realistic_voxel_rendering src/main.cpp ${BACKWARD_ENABLE}
        src/exceptions/StackTraceException.cpp src/exceptions/StackTraceException.h
        src/args/ValidPathCheckAction.cpp
        src/logging/loggers.cpp src/logging/loggers.h src/logging/SomeLevelsSink.h
        src/args/RegexCheckAction.h src/args/RegexCheckAction.cpp src/Application.h
        src/concepts/Window.cpp src/ui/GlfwWindow.cpp src/ui/GlfwWindow.h src/ui/events/common.h
        src/ui/events/Subscription.cpp src/ui/events/Subscription.h src/concepts/Iterable.h src/concepts/StreamConcepts.h
        src/coroutines/Sequence.h src/concepts/Incrementable.h src/ui/events/EventDispatchImpl.h
        src/vulkan/VulkanException.cpp src/vulkan/VulkanException.h src/vulkan/DefaultDeviceSuitabilityScorer.cpp
        src/vulkan/DefaultDeviceSuitabilityScorer.h src/rendering/TriangleRenderer.cpp
        src/rendering/TriangleRenderer.h src/vulkan/types/SwapChain.cpp
        src/vulkan/types/SwapChain.h src/vulkan/types/PhysicalDevice.cpp src/vulkan/types/PhysicalDevice.h src/vulkan/types/Instance.cpp
        src/vulkan/types/Instance.h src/vulkan/types/VulkanCommon.h
        src/vulkan/types/Surface.h src/concepts/Renderer.h src/vulkan/types/ImageView.cpp src/vulkan/types/ImageView.h
        src/vulkan/types/Image.cpp src/vulkan/types/Image.h src/vulkan/types/RenderPass.cpp src/vulkan/types/RenderPass.h
        src/vulkan/types/builders/RenderPassBuilder.cpp src/vulkan/types/builders/RenderPassBuilder.h src/vulkan/types/Shader.cpp
        src/vulkan/types/Shader.h src/vulkan/types/GraphicsPipeline.cpp src/vulkan/types/GraphicsPipeline.h
        src/vulkan/types/builders/GraphicsPipelineBuilder.cpp src/vulkan/types/builders/GraphicsPipelineBuilder.h
        src/vulkan/types/DescriptorSetLayout.cpp src/vulkan/types/DescriptorSetLayout.h src/vulkan/types/CommandPool.cpp
        src/vulkan/types/CommandPool.h src/vulkan/types/FrameBuffer.cpp src/vulkan/types/FrameBuffer.h
        src/vulkan/types/VulkanObject.h src/vulkan/types/Surface.cpp src/ui/events/EventDispatchImpl.cpp src/concepts/PtrConstructible.h
        src/rendering/TriangleRenderer.cpp src/vulkan/types/fwd.h src/vulkan/types/types.h src/vulkan/types/VulkanCommon.cpp
        src/vulkan/types/LogicalDevice.cpp src/vulkan/types/LogicalDevice.h src/vulkan/types/CommandBuffer.cpp src/vulkan/types/CommandBuffer.h
        src/vulkan/types/Fence.cpp src/vulkan/types/Fence.h src/vulkan/glsl/Compiler.cpp src/vulkan/glsl/Compiler.h src/concepts/OneOf.h
        src/vulkan/types/Buffer.cpp src/vulkan/types/Buffer.h src/vulkan/types/BufferView.cpp src/vulkan/types/BufferView.h
        src/utils/tuple.h src/utils/algorithms.h src/threading/ThreadPool.cpp src/threading/ThreadPool.h include/imgui/imgui_impl_glfw.h
        include/imgui/imgui_impl_glfw.cpp include/imgui/imgui_impl_vulkan.cpp include/imgui/imgui_impl_vulkan.h
        src/ui/imgui/ImGuiInterface.cpp src/ui/imgui/ImGuiInterface.h src/ui/imgui/ImGuiGlfwVulkan.cpp src/ui/imgui/ImGuiGlfwVulkan.h
        src/vulkan/types/DescriptorPool.cpp src/vulkan/types/DescriptorPool.h src/vulkan/types/Semaphore.cpp src/vulkan/types/Semaphore.h
        src/utils/common_enums.h src/ui/imgui/elements/interface/Element.cpp src/ui/imgui/elements/interface/Element.h
        src/ui/imgui/elements/interface/Container.cpp src/ui/imgui/elements/interface/Container.h src/ui/imgui/elements/Panel.cpp
        src/ui/imgui/elements/Panel.h src/ui/imgui/elements/Button.cpp src/ui/imgui/elements/Button.h src/ui/imgui/elements/Text.cpp
        src/ui/imgui/elements/Text.h src/ui/imgui/elements/InputText.cpp src/ui/imgui/elements/InputText.h src/ui/imgui/elements/interface/LabeledElement.cpp
        src/ui/imgui/elements/interface/LabeledElement.h src/ui/imgui/elements/interface/ResizableElement.cpp
        src/ui/imgui/elements/interface/ResizableElement.h src/ui/imgui/elements/Window.cpp src/ui/imgui/elements/Window.h
        src/ui/imgui/elements/interface/ValueObservableElement.cpp src/ui/imgui/elements/interface/ValueObservableElement.h
        src/ui/imgui/elements/Slider.h src/ui/imgui/elements.h src/ui/imgui/elements/Input.h src/ui/imgui/elements/DragInput.h
        src/ui/imgui/elements/ColorChooser.cpp src/ui/imgui/elements/ColorChooser.h src/ui/imgui/elements/Tree.cpp src/ui/imgui/elements/Tree.h
        src/ui/imgui/elements/ComboBox.cpp src/ui/imgui/elements/ComboBox.h src/ui/imgui/elements/RadioButton.cpp src/ui/imgui/elements/RadioButton.h
        src/ui/imgui/elements/RadioGroup.cpp src/ui/imgui/elements/RadioGroup.h src/ui/imgui/elements/Image.cpp src/ui/imgui/elements/Image.h
        src/ui/imgui/elements/Checkbox.cpp src/ui/imgui/elements/Checkbox.h src/ui/imgui/elements/ProgressBar.cpp src/ui/imgui/elements/ProgressBar.h
        src/ui/imgui/elements/Dialog.cpp src/ui/imgui/elements/Dialog.h src/ui/imgui/elements/Group.cpp src/ui/imgui/elements/Group.h src/utils/math.h
        src/ui/imgui/elements/ListBox.cpp src/ui/imgui/elements/ListBox.h src/ui/imgui/elements/Plot.cpp src/ui/imgui/elements/Plot.h
        src/ui/imgui/elements/MenuBars.cpp src/ui/imgui/elements/MenuBars.h src/utils/Visitor.h src/vulkan/types/Texture.cpp
        src/vulkan/types/Texture.h src/utils/RAII.h src/vulkan/types/TextureSampler.cpp src/vulkan/types/TextureSampler.h
        src/ui/imgui/elements/Memo.cpp src/ui/imgui/elements/Memo.h src/logging/CallbackSink.cpp src/logging/CallbackSink.h
        src/ui/imgui/elements/TabBar.cpp src/ui/imgui/elements/TabBar.h src/ui/imgui/elements/interface/SavableElement.cpp
        src/ui/imgui/elements/interface/SavableElement.h src/utils/config.h src/ui/imgui/serialization.h src/ui/imgui/ui_tree_traversal.h)
target_link_libraries(realistic_voxel_rendering
        -lbfd -ldl
        magic_enum imgui::imgui units fmt::fmt range-v3 nanobench subprocess argparse chaiscript
        shaderc glslang cppcoro
        ${GLFW_STATIC_LIBRARIES} ${GLM_LIBRARIES} ${Vulkan_LIBRARIES})
add_backward(realistic_voxel_rendering)

target_compile_options(realistic_voxel_rendering PRIVATE ${flags})


print_all_vars()
cmake_minimum_required(VERSION 3.17)
include("cmake/HunterGate.cmake")
include("cmake/CPM.cmake")
include(FetchContent)

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
        realistic_voxel_scene_rendering_in_real_time
        VERSION 0.1.0
)
add_compile_definitions(GLFW_INCLUDE_VULKAN
        GLM_FORCE_RADIANS
        GLM_FORCE_DEPTH_ZERO_TO_ONE
        GLM_ENABLE_EXPERIMENTAL
        GLM_FORCE_SWIZZLE
        VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE)

set(CMAKE_CXX_STANDARD 20)
list(APPEND flags "-fconcepts" "-fconcepts-diagnostics-depth=10" "-Werror=return-type" "-fcoroutines"
        "-Wall" "-Wextra" "-Werror" "-Wpedantic" "-Wno-unknown-pragmas" "-Wno-unused-function"
        "-Wpointer-arith" "-Wcast-qual" "-Wmissing-braces" "-Wno-type-limits")

if (CMAKE_BUILD_TYPE MATCHES Debug)
    message(" ")
    message("CMAKE IN DEBUG MODE")
    message(" ")
elseif(CMAKE_BUILD_TYPE MATCHES Release)
    message(" ")
    message("CMAKE IN RELEASE MODE")
    message(" ")
    list(APPEND flags "-o3")
elseif(CMAKE_BUILD_TYPE MATCHES ASAN)
    message(" ")
    message("CMAKE IN ADDRESS SANITISE MODE")
    message(" ")
    list(APPEND flags "-fno-omit-frame-pointer" "-fsanitize=address")
elseif(CMAKE_BUILD_TYPE MATCHES LSAN)
    message(" ")
    message("CMAKE IN LEAK SANITISE MODE")
    message(" ")
    list(APPEND flags "-fno-omit-frame-pointer" "-fsanitize=leak")
elseif(CMAKE_BUILD_TYPE MATCHES UBSAN)
    message(" ")
    message("CMAKE IN UNDEFINED SANITISE MODE")
    message(" ")
    list(APPEND flags "-fsanitize=undefined")
elseif(CMAKE_BUILD_TYPE MATCHES TSAN)
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
        NAME toml11
        GITHUB_REPOSITORY ToruNiina/toml11
        GIT_TAG v3.6.0
)
CPMAddPackage(
        NAME chaiscript
        GITHUB_REPOSITORY ChaiScript/ChaiScript
        GIT_TAG v6.1.0
)

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
include_directories(${CMAKE_SOURCE_DIR}/third_party/)



#set(IMPLOT_CPP_FILES third_party/implot/implot.cpp third_party/implot/implot_items.cpp)
add_executable(realistic_voxel_scene_rendering_in_real_time main.cpp ${BACKWARD_ENABLE}
        exceptions/StackTraceException.cpp exceptions/StackTraceException.h
        args/ValidPathCheckAction.cpp
        #${IMPLOT_CPP_FILES}
        utils.h logging/loggers.cpp logging/loggers.h logging/SomeLevelsSink.h
        args/RegexCheckAction.h args/RegexCheckAction.cpp application.h
        concepts/Window.cpp ui/GlfwWindow.cpp ui/GlfwWindow.h ui/events/common.h
        ui/events/Subscription.cpp ui/events/Subscription.h concepts/Iterable.h concepts/StreamConcepts.h
        coroutines/Sequence.h concepts/Incrementable.h ui/events/EventDispatchImpl.h
        vulkan/VulkanException.cpp vulkan/VulkanException.h vulkan/DefaultDeviceSuitabilityScorer.cpp
        vulkan/DefaultDeviceSuitabilityScorer.h rendering/TriangleRenderer.cpp
        rendering/TriangleRenderer.h vulkan/types/SwapChain.cpp
        vulkan/types/SwapChain.h vulkan/types/PhysicalDevice.cpp vulkan/types/PhysicalDevice.h vulkan/types/Instance.cpp
        vulkan/types/Instance.h vulkan/types/VulkanCommon.h
        vulkan/types/Surface.h concepts/Renderer.h vulkan/types/ImageView.cpp vulkan/types/ImageView.h
        vulkan/types/Image.cpp vulkan/types/Image.h vulkan/types/RenderPass.cpp vulkan/types/RenderPass.h
        vulkan/types/builders/RenderPassBuilder.cpp vulkan/types/builders/RenderPassBuilder.h vulkan/types/Shader.cpp
        vulkan/types/Shader.h vulkan/types/GraphicsPipeline.cpp vulkan/types/GraphicsPipeline.h
        vulkan/types/builders/GraphicsPipelineBuilder.cpp vulkan/types/builders/GraphicsPipelineBuilder.h
        vulkan/types/DescriptorSetLayout.cpp vulkan/types/DescriptorSetLayout.h vulkan/types/CommandPool.cpp
        vulkan/types/CommandPool.h vulkan/types/FrameBuffer.cpp vulkan/types/FrameBuffer.h vulkan/types/VulkanObject.cpp
        vulkan/types/VulkanObject.h vulkan/types/Surface.cpp ui/events/EventDispatchImpl.cpp concepts/PtrConstructable.h
        rendering/TriangleRenderer.cpp vulkan/types/fwd.h vulkan/types/types.h vulkan/types/VulkanCommon.cpp vulkan/types/LogicalDevice.cpp vulkan/types/LogicalDevice.h vulkan/types/CommandBuffer.cpp vulkan/types/CommandBuffer.h vulkan/types/Fence.cpp vulkan/types/Fence.h vulkan/glsl/Compiler.cpp vulkan/glsl/Compiler.h)
target_link_libraries(realistic_voxel_scene_rendering_in_real_time
        -lbfd -ldl
        magic_enum imgui::imgui units fmt::fmt range-v3 nanobench subprocess argparse toml11 chaiscript
        shaderc glslang
        ${GLFW_STATIC_LIBRARIES} ${GLM_LIBRARIES} ${Vulkan_LIBRARIES})
add_backward(realistic_voxel_scene_rendering_in_real_time)

target_compile_options(realistic_voxel_scene_rendering_in_real_time PRIVATE ${flags})



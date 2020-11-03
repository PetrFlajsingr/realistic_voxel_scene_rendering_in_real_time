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
add_compile_definitions(
        GLM_FORCE_RADIANS
        GLM_FORCE_DEPTH_ZERO_TO_ONE
        GLM_ENABLE_EXPERIMENTAL
        GLM_FORCE_SWIZZLE
        STB_IMAGE_IMPLEMENTATION
        STACKTRACE_VULKAN_REPORT
        )

include_directories(include)

set(CMAKE_CXX_STANDARD 20)
list(APPEND flags "-fconcepts" "-fconcepts-diagnostics-depth=10" "-Werror=return-type" "-fcoroutines"
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
        NAME pf_common
        GITHUB_REPOSITORY PetrFlajsingr/pf_common
        GIT_TAG master
)
CPMAddPackage(
        NAME pf_imgui
        GITHUB_REPOSITORY PetrFlajsingr/pf_imgui
        GIT_TAG master
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
        OPTIONS
        "SHADERC_SKIP_INSTALL ON"
)
CPMAddPackage(
        NAME pf_glfw_vulkan
        GITHUB_REPOSITORY PetrFlajsingr/pf_glfw_vulkan
        GIT_TAG master
)


if (chaiscript_ADDED)
    add_library(chaiscript INTERFACE IMPORTED)
    target_include_directories(chaiscript INTERFACE ${chaiscript_SOURCE_DIR}/include)
endif ()

include_directories("${CMAKE_SOURCE_DIR}/src")

set(SOURCES
        src/main.cpp
        src/args/ValidPathCheckAction.cpp
        src/logging/loggers.cpp
        src/args/RegexCheckAction.cpp
        src/rendering/TriangleRenderer.cpp
        src/logging/CallbackSink.cpp
        src/ui/ImGuiGlfwVulkan.cpp
        include/imgui/imgui_impl_glfw.cpp
        include/imgui/imgui_impl_vulkan.cpp
        include/ogt_vox.h)

set(INCLUDES
        src/logging/loggers.h
        src/logging/SomeLevelsSink.h
        src/args/RegexCheckAction.h
        src/Application.h
        src/ui/GlfwWindow.h
        src/ui/events/common.h
        src/ui/events/EventDispatchImpl.h
        src/rendering/TriangleRenderer.h
        src/concepts/Renderer.h
        src/threading/ThreadPool.h
        src/ui/ImGuiGlfwVulkan.h
        src/utils/common_enums.h
        src/logging/CallbackSink.h
        src/utils/config.h
        src/ui/ImGuiGlfwVulkan.h
        include/imgui/imgui_impl_glfw.h
        include/imgui/imgui_impl_vulkan.h)

add_executable(realistic_voxel_rendering src/main.cpp
        ${SOURCES})
target_link_libraries(realistic_voxel_rendering
        -lbfd -ldl
        magic_enum imgui::imgui nanobench subprocess argparse chaiscript
        pf_imgui::pf_imgui pf_common::pf_common pf_glfw_vulkan::pf_glfw_vulkan
        shaderc glslang
        ${GLFW_STATIC_LIBRARIES} ${GLM_LIBRARIES} ${Vulkan_LIBRARIES})
add_backward(realistic_voxel_rendering)

target_compile_options(realistic_voxel_rendering PRIVATE ${flags})

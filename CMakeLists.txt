cmake_minimum_required(VERSION 3.17)
include("cmake/HunterGate.cmake")
include(FetchContent)

set(CPM_DOWNLOAD_LOCATION "${CMAKE_SOURCE_DIR}/cmake/CPM.cmake")
set(CPM_DOWNLOAD_VERSION 0.32.2)

message(https://github.com/TheLartians/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake)
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
        VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
        PF_ENUM_OUT
)

include_directories(include)

option(ENABLE_STACKTRACE "enable stacktrace in exceptions" OFF)
if (ENABLE_STACKTRACE)
    add_compile_definitions(STACKTRACE_ENABLE
            STACKTRACE_VULKAN_REPORT)
endif ()

set(CMAKE_CXX_STANDARD 20)
list(APPEND flags "-fconcepts" "-fconcepts-diagnostics-depth=10" "-fcoroutines" "-Werror=return-type"
        "-Wall" "-Wextra" "-Werror" "-Wpedantic" "-Wno-unknown-pragmas" "-Wno-unused-function"
        "-Wpointer-arith" "-Wno-cast-qual" "-Wno-type-limits" "-Wno-strict-aliasing" "-Wno-stringop-truncation")

if (USE_LLD_LINKER)
    list(APPEND flags "-fuse-ld=lld")
elseif (USE_GOLD_LINKER)
    list(APPEND flags "-fuse-ld=gold")
endif ()


if (CMAKE_BUILD_TYPE MATCHES Debug)
    message(" ")
    message("CMAKE IN DEBUG MODE")
    message(" ")
    list(APPEND flags "-g")
elseif (CMAKE_BUILD_TYPE MATCHES Release)
    message(" ")
    message("CMAKE IN RELEASE MODE")
    message(" ")
    list(APPEND flags "-O3")
elseif (CMAKE_BUILD_TYPE MATCHES ASAN)
    message(" ")
    message("CMAKE IN ADDRESS SANITISE MODE")
    message(" ")
    list(APPEND flags "-fno-omit-frame-pointer" "-fsanitize=address" "-g")
elseif (CMAKE_BUILD_TYPE MATCHES LSAN)
    message(" ")
    message("CMAKE IN LEAK SANITISE MODE")
    message(" ")
    list(APPEND flags "-fno-omit-frame-pointer" "-fsanitize=leak" "-g")
elseif (CMAKE_BUILD_TYPE MATCHES UBSAN)
    message(" ")
    message("CMAKE IN UNDEFINED SANITISE MODE")
    message(" ")
    list(APPEND flags "-fsanitize=undefined" "-g")
elseif (CMAKE_BUILD_TYPE MATCHES TSAN)
    message(" ")
    message("CMAKE IN THREAD SANITISE MODE")
    message(" ")
    list(APPEND flags "-fsanitize=thread" "-g")
elseif (CMAKE_BUILD_TYPE MATCHES MSAN)
    message(" ")
    message("CMAKE IN MEMORY SANITISE MODE")
    message(" ")
    list(APPEND flags "-fsanitize=memory" "-g")
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
        NAME argparse
        GITHUB_REPOSITORY p-ranav/argparse
        GIT_TAG v2.1
)
CPMAddPackage(
        NAME chaiscript
        GITHUB_REPOSITORY ChaiScript/ChaiScript
        GIT_TAG v6.1.0
)
CPMAddPackage(
        NAME tinyxml2
        GITHUB_REPOSITORY leethomason/tinyxml2
        GIT_TAG 9.0.0
)


if (ENABLE_STACKTRACE)
    CPMAddPackage(
            NAME pf_common
            GITHUB_REPOSITORY PetrFlajsingr/pf_common
            GIT_TAG v1.1.0
            OPTIONS "ENABLE_STACKTRACE ON"
    )
else ()
    CPMAddPackage(
            NAME pf_common
            GITHUB_REPOSITORY PetrFlajsingr/pf_common
            GIT_TAG v1.1.0
    )
endif ()
if (ENABLE_STACKTRACE)
    CPMAddPackage(
            NAME pf_imgui
            GITHUB_REPOSITORY PetrFlajsingr/pf_imgui
            GIT_TAG v0.6.0
            OPTIONS "ENABLE_STACKTRACE ON"
    )
else ()
    CPMAddPackage(
            NAME pf_imgui
            GITHUB_REPOSITORY PetrFlajsingr/pf_imgui
            GIT_TAG v0.6.0
    )
endif ()

if (ENABLE_STACKTRACE)
    CPMAddPackage(
            NAME pf_glfw_vulkan
            GITHUB_REPOSITORY PetrFlajsingr/pf_glfw_vulkan
            GIT_TAG v0.1.3
            OPTIONS "PF_GLFW_EXCEPTIONS ON"
    )
    add_compile_definitions(PF_GLFW_WINDOW_EXCEPTIONS_ENABLED)
else ()
    CPMAddPackage(
            NAME pf_glfw_vulkan
            GITHUB_REPOSITORY PetrFlajsingr/pf_glfw_vulkan
            GIT_TAG v0.1.3
    )
endif ()

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
        src/rendering/EditRenderer.cpp
        src/rendering/MainRenderer.cpp
        src/rendering/GBufferRenderer.cpp
        src/rendering/VulkanDebugCallbackImpl.cpp
        src/rendering/BakedProbesRenderer.cpp
        src/logging/CallbackSink.cpp
        src/ui/EditorUI.cpp
        src/ui/MainUI.cpp
        src/utils/Camera.cpp
        src/utils/FPSCounter.cpp
        src/utils/FlameGraphSampler.cpp
        src/voxel/RawVoxelModel.cpp
        src/voxel/ModelLoading.cpp
        src/voxel/RawVoxelScene.cpp
        src/voxel/SparseVoxelOctree.cpp
        src/voxel/GPUModelInfo.cpp
        src/voxel/SparseVoxelOctreeCreation.cpp
        src/voxel/AABB_BVH.cpp
        src/voxel/Materials.cpp
        src/voxel/GPUModelManager.cpp
        src/voxel/TeardownMaps.cpp
        src/rendering/light_field_probes/ProbeRenderer.cpp
        src/rendering/light_field_probes/ProbeManager.cpp
        src/rendering/light_field_probes/ProbeBakeRenderer.cpp
        ${pf_imgui_SOURCE_DIR}/src/pf_imgui/backends/ImGuiGlfwVulkanInterface.cpp
        ${pf_imgui_SOURCE_DIR}/src/pf_imgui/backends/impl/imgui_impl_glfw.cpp
        ${pf_imgui_SOURCE_DIR}/src/pf_imgui/backends/impl/imgui_impl_vulkan.cpp

        )

set(INCLUDES
        src/logging/loggers.h
        src/logging/SomeLevelsSink.h
        src/args/RegexCheckAction.h
        src/Application.h
        src/ui/GlfwWindow.h
        src/ui/events/common.h
        src/ui/events/EventDispatchImpl.h
        src/ui/EditorUI.h
        src/rendering/VulkanDebugCallbackImpl.h
        src/rendering/EditRenderer.h
        src/concepts/Renderer.h
        src/ui/ImGuiGlfwVulkan.h
        src/utils/common_enums.h
        src/logging/CallbackSink.h
        src/utils/config.h
        src/utils/Camera.h
        src/utils/FPSCounter.h
        src/utils/FlameGraphSampler.h
        src/utils/interface/Serializable.h
        src/voxel/RawVoxelModel.h
        src/voxel/ModelLoading.h
        src/voxel/RawVoxelScene.h
        src/voxel/SparseVoxelOctree.h
        src/voxel/SparseVoxelOctreeCreation.h
        src/voxel/AABB_BVH.h
        src/voxel/GPUModelInfo.h
        src/voxel/SceneFileManager.h
        src/voxel/GPUModelManager.h
        src/voxel/Materials.h
        src/rendering/light_field_probes/ProbeRenderer.h
        src/rendering/light_field_probes/ProbeManager.h
        )

set(SVO_SOURCES
        src/main_svo.cpp
        src/logging/loggers.cpp
        src/logging/CallbackSink.cpp
        src/voxel/RawVoxelModel.cpp
        src/voxel/ModelLoading.cpp
        src/voxel/RawVoxelScene.cpp
        src/voxel/SparseVoxelOctree.cpp
        src/voxel/SparseVoxelOctreeCreation.cpp
        )


if (CMAKE_BUILD_TYPE MATCHES ASAN)
    set(LASAN -lasan)
else ()
    set(LASAN)
endif ()

add_executable(realistic_voxel_rendering ${SOURCES})
target_link_libraries(realistic_voxel_rendering
        ${LASAN}
        -lbfd -ldl
        magic_enum nanobench argparse chaiscript
        pf_imgui::pf_imgui pf_common::pf_common pf_glfw_vulkan::pf_glfw_vulkan tinyxml2::tinyxml2
        shaderc glslang
        ${GLFW_STATIC_LIBRARIES} ${GLM_LIBRARIES} ${Vulkan_LIBRARIES})
if (ENABLE_STACKTRACE)
    add_backward(realistic_voxel_rendering)
endif ()


target_compile_options(realistic_voxel_rendering PRIVATE ${flags})


if (MEASURE_BUILD_TIME)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CMAKE_COMMAND} -E time")
endif ()



file(GLOB SPECIAL_SRC_FILES
        "${spirvtools_SOURCE_DIR}/source/.*")
set_property(SOURCE ${SPECIAL_SRC_FILES} PROPERTY COMPILE_FLAGS -Wno-error=switch)

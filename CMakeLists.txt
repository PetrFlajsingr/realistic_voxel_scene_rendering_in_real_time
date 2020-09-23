cmake_minimum_required(VERSION 3.17)
include("cmake/HunterGate.cmake")
include("cmake/CPM.cmake")
include(FetchContent)

function(print_all_vars)
    get_cmake_property(_variableNames VARIABLES)
    list (SORT _variableNames)
    foreach (_variableName ${_variableNames})
        message(STATUS "${_variableName}=${${_variableName}}")
    endforeach()
endfunction()

HunterGate(
        URL "https://github.com/cpp-pm/hunter/archive/v0.23.269.tar.gz"
        SHA1 "64024b7b95b4c86d50ae05b926814448c93a70a0"
)

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
endif()


project(
        realistic_voxel_scene_rendering_in_real_time
        VERSION 0.1.0
        LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fconcepts -fconcepts-diagnostics-depth=10 -Werror=return-type -Wall -Wextra -Werror -pedantic -Wno-unknown-pragmas")


hunter_add_package(imgui)
find_package(imgui CONFIG REQUIRED)
find_package(Vulkan REQUIRED)
hunter_add_package(glfw)
find_package(glfw3 CONFIG REQUIRED)
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
if (chaiscript_ADDED)
    add_library(chaiscript INTERFACE IMPORTED)
    target_include_directories(chaiscript INTERFACE ${chaiscript_SOURCE_DIR}/include)
endif()

print_all_vars()
#set(IMPLOT_CPP_FILES third_party/implot/implot.cpp third_party/implot/implot_items.cpp)
add_executable(realistic_voxel_scene_rendering_in_real_time main.cpp ${BACKWARD_ENABLE}
        exceptions/stacktrace_exception.cpp exceptions/stacktrace_exception.h
        args/valid_path_check_action.cpp
        #${IMPLOT_CPP_FILES}
        utils.h logging/global_logger.cpp logging/global_logger.h)
target_link_libraries(realistic_voxel_scene_rendering_in_real_time
        -lbfd -ldl
        magic_enum imgui::imgui units fmt::fmt range-v3 nanobench subprocess argparse toml11 chaiscript
        ${Vulkan_LIBRARIES} glfw)
add_backward(realistic_voxel_scene_rendering_in_real_time)



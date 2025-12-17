cmake_minimum_required(VERSION 4.0)
project(vee)

set(ENGINE_EXEC "vee_engine")
set(EDITOR_EXEC "vee_editor")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# ----------------------------------------------------
# 1. External Dependency Definitions (FetchContent)
# ----------------------------------------------------

# yaml-cpp (Full CMake Project)
FetchContent_Declare(
        yaml-cpp
        GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
        GIT_TAG master
)
FetchContent_MakeAvailable(yaml-cpp)

# ImGui (Manual Target Creation needed for Vulkan backends)
FetchContent_Declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG v1.92.5-docking
)
FetchContent_GetProperties(imgui)
if (NOT imgui_POPULATED)
    FetchContent_MakeAvailable(imgui)
endif ()

# tinyobjloader (Header-Only with Optional Source)
FetchContent_Declare(
        tinyobjloader
        GIT_REPOSITORY https://github.com/tinyobjloader/tinyobjloader.git
        GIT_TAG v2.0.0rc13
)
FetchContent_MakeAvailable(tinyobjloader)

FetchContent_Declare(
        VMA
        GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
        GIT_TAG master
)
FetchContent_MakeAvailable(VMA)

# ----------------------------------------------------
# 2. Find and Configure System Dependencies
# ----------------------------------------------------

find_package(glfw3 3.4 REQUIRED)
find_package(glm REQUIRED)
find_package(Vulkan REQUIRED)

find_path(SHADERC_INCLUDE_DIR NAMES shaderc/shaderc.h HINTS /opt/homebrew/include /usr/local/include)
find_library(SHADERC_LIB NAMES shaderc_shared shaderc_combined shaderc HINTS /opt/homebrew/lib /usr/local/lib)

if (NOT SHADERC_LIB)
    message(FATAL_ERROR "shaderc library not found. Install shaderc (Homebrew: `brew install shaderc`) or set SHADERC_LIB manually.")
endif ()

# ----------------------------------------------------
# 3. Create Custom Targets for ImGui and STB
# ----------------------------------------------------

add_library(ImGui STATIC
        "${imgui_SOURCE_DIR}/imgui.cpp"
        "${imgui_SOURCE_DIR}/imgui_draw.cpp"
        "${imgui_SOURCE_DIR}/imgui_widgets.cpp"
        "${imgui_SOURCE_DIR}/imgui_tables.cpp"
        "${imgui_SOURCE_DIR}/imgui_demo.cpp"
        "${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp"
        "${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp"
)
target_include_directories(ImGui PUBLIC
        "${imgui_SOURCE_DIR}"
        "${imgui_SOURCE_DIR}/backends"
)

# ----------------------------------------------------
# 4. Source Files and Executables
# ----------------------------------------------------

# Note: Keeping the manual globs for *your own* engine/editor files is fine.
FILE(GLOB_RECURSE ENGINE_SOURCE_FILES src/engine/*.cpp)
FILE(GLOB_RECURSE EDITOR_SOURCE_FILES src/editor/*.cpp)
FILE(GLOB_RECURSE EXTERNAL_SOURCE_FILES external/stb/*.cpp)

SET(EXTERNAL_DIRS external/stb)

add_executable(${ENGINE_EXEC} engine.cpp ${ENGINE_SOURCE_FILES} ${EXTERNAL_SOURCE_FILES})
add_executable(${EDITOR_EXEC} editor.cpp ${ENGINE_SOURCE_FILES} ${EDITOR_SOURCE_FILES} ${EXTERNAL_SOURCE_FILES})

# ----------------------------------------------------
# 5. Link Dependencies
# ----------------------------------------------------

SET(EXECUTABLES ${ENGINE_EXEC} ${EDITOR_EXEC})

target_link_libraries(ImGui PRIVATE glfw)

foreach (EXECUTABLE ${EXECUTABLES})
    target_include_directories(${EXECUTABLE} PRIVATE
            ${SHADERC_INCLUDE_DIR}
            ${EXTERNAL_DIRS}
            "${vma_SOURCE_DIR}/include"
    )

    target_link_libraries(${EXECUTABLE} PRIVATE
            ${SHADERC_LIB}
            glfw
            glm::glm
            Vulkan::Vulkan
            "-framework Metal"
            "-framework Foundation"
            "-framework QuartzCore"

            yaml-cpp::yaml-cpp
            tinyobjloader
            ImGui
    )
endforeach ()
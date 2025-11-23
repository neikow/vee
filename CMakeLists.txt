cmake_minimum_required(VERSION 4.0)
project(vee)

set(ENGINE_EXEC "vee_engine")
set(EDITOR_EXEC "vee_editor")

set(CMAKE_CXX_STANDARD 20)

find_package(glfw3 3.4 REQUIRED)
find_package(glm REQUIRED)
find_package(Vulkan REQUIRED)

find_path(SHADERC_INCLUDE_DIR NAMES shaderc/shaderc.h HINTS /opt/homebrew/include /usr/local/include)
find_library(SHADERC_LIB NAMES shaderc_shared shaderc_combined shaderc HINTS /opt/homebrew/lib /usr/local/lib)

FILE(GLOB_RECURSE ENGINE_SOURCE_FILES src/engine/*.cpp)
FILE(GLOB_RECURSE ENGINE_HEADER_FILES src/engine/*.h)
FILE(GLOB_RECURSE EDITOR_SOURCE_FILES src/editor/*.cpp)
FILE(GLOB_RECURSE EDITOR_HEADER_FILES src/editor/*.h)
FILE(GLOB_RECURSE EXTERNAL_SOURCE_FILES
        external/stb/*.cpp
        external/tinyobjectloader/*.cpp
        external/imgui/*.cpp
        external/imgui/backends/*.cpp
)
FILE(GLOB_RECURSE EXTERNAL_HEADER_FILES
        external/stb/*.h
        external/tinyobjectloader/*.h
        external/imgui/*.h
        external/imgui/backends/*.h
)

SET(EXTERNAL_DIRS
        external/stb
        external/tinyobjectloader
        external/imgui
        external/imgui/backends
)

SET(ENGINE_FILES
        ${ENGINE_SOURCE_FILES}
        ${ENGINE_HEADER_FILES}
)
SET(EDITOR_FILES
        ${EDITOR_SOURCE_FILES}
        ${EDITOR_HEADER_FILES}
)
SET(EXTERNAL_FILES
        ${EXTERNAL_SOURCE_FILES}
        ${EXTERNAL_HEADER_FILES}
)

add_executable(${ENGINE_EXEC} engine.cpp ${ENGINE_FILES} ${EXTERNAL_FILES})
add_executable(${EDITOR_EXEC} editor.cpp ${ENGINE_FILES} ${EDITOR_FILES} ${EXTERNAL_FILES})

target_include_directories(
        ${ENGINE_EXEC} PRIVATE ${EXTERNAL_DIRS}
)
target_include_directories(
        ${EDITOR_EXEC} PRIVATE ${EXTERNAL_DIRS}
)

if (NOT SHADERC_LIB)
    message(FATAL_ERROR "shaderc library not found. Install shaderc (Homebrew: `brew install shaderc`) or set SHADERC_LIB manually.")
endif ()

if (SHADERC_INCLUDE_DIR)
    target_include_directories(${ENGINE_EXEC} PRIVATE ${SHADERC_INCLUDE_DIR})
    target_include_directories(${EDITOR_EXEC} PRIVATE ${SHADERC_INCLUDE_DIR})
endif ()

SET(EXECUTABLES ${ENGINE_EXEC} ${EDITOR_EXEC})

foreach (EXECUTABLE ${EXECUTABLES})
    target_link_libraries(${EXECUTABLE} PRIVATE
            ${SHADERC_LIB}
            glfw
            glm::glm
            Vulkan::Vulkan
            "-framework Metal"
            "-framework Foundation"
            "-framework QuartzCore"
    )
endforeach ()
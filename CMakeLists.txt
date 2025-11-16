cmake_minimum_required(VERSION 4.0)
project(game_engine)

set(CMAKE_CXX_STANDARD 20)

find_package(glfw3 3.4 REQUIRED)
find_package(glm REQUIRED)
find_package(Vulkan REQUIRED)

find_path(SHADERC_INCLUDE_DIR NAMES shaderc/shaderc.h HINTS /opt/homebrew/include /usr/local/include)
find_library(SHADERC_LIB NAMES shaderc_shared shaderc_combined shaderc HINTS /opt/homebrew/lib /usr/local/lib)

add_executable(${PROJECT_NAME} main.cpp
        src/shaders/compile.cpp
        src/shaders/compile.h
)

if (NOT SHADERC_LIB)
    message(FATAL_ERROR "shaderc library not found. Install shaderc (Homebrew: `brew install shaderc`) or set SHADERC_LIB manually.")
endif ()

if (SHADERC_INCLUDE_DIR)
    target_include_directories(${PROJECT_NAME} PRIVATE ${SHADERC_INCLUDE_DIR})
endif ()

target_link_libraries(${PROJECT_NAME} PRIVATE
        ${SHADERC_LIB}
        glfw
        glm::glm
        Vulkan::Vulkan
        "-framework Metal"
        "-framework Foundation"
        "-framework QuartzCore"
)

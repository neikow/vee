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
        external/stb/stb_image.cpp
        external/tinyobjectloader/tiny_obj_loader.cpp
        src/entities/manager.cpp
        src/entities/manager.h
        src/entities/components/component_manager.cpp
        src/entities/components/component_manager.h
        src/entities/system/system.cpp
        src/entities/system/system.h
        src/entities/system/movement_system.cpp
        src/entities/system/movement_system.h
        src/entities/components/entity_component_base.cpp
        src/entities/components/entity_component_base.h
        src/entities/components/transform_component.cpp
        src/entities/components/transform_component.h
        src/entities/components/renderable_component.cpp
        src/entities/components/renderable_component.h
        src/entities/components/velocity_component.cpp
        src/entities/components/velocity_component.h
        src/entities/system/system_manager.cpp
        src/entities/system/system_manager.h
        src/entities/system/display_system.cpp
        src/entities/system/display_system.h
        src/renderer/abstract.cpp
        src/renderer/abstract.h
        src/renderer/vulkan/vulkan_renderer.cpp
        src/renderer/vulkan/vulkan_renderer.h
        src/entities/components/camera_component.cpp
        src/entities/components/camera_component.h
        src/entities/system/camera_system.cpp
        src/entities/system/camera_system.h
        src/engine.cpp
        src/engine.h
        src/entities/components/tags/active_camera_tag_component.cpp
        src/entities/components/tags/active_camera_tag_component.h
        src/utils/math_utils.cpp
        src/utils/math_utils.h
        src/utils/entity_utils.cpp
        src/utils/entity_utils.h
        src/entities/types.h
        src/renderer/vulkan/consts.h
        src/renderer/vulkan/utils.cpp
        src/renderer/vulkan/utils.h
        src/renderer/vulkan/types.h
        src/renderer/vulkan/vertex.cpp
        src/renderer/vulkan/vertex.h
        src/models/mesh_manager/abstract_mesh_manager.cpp
        src/models/mesh_manager/abstract_mesh_manager.h
        src/models/mesh_manager/vulkan_mesh_manager.cpp
        src/models/mesh_manager/vulkan_mesh_manager.h
        src/models/mesh_manager/vulkan_mesh_manager.h
        src/utils/type_utils.h
        src/utils/concepts/hashable.h
        src/utils/vectors.h
        src/models/texture_manager/vulkan_texture_manager.cpp
        src/models/texture_manager/vulkan_texture_manager.h
)

target_include_directories(${PROJECT_NAME} PRIVATE external/stb external/tinyobjectloader)

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

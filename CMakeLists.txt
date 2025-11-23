cmake_minimum_required(VERSION 4.0)
project(game_engine)

set(CMAKE_CXX_STANDARD 20)

find_package(glfw3 3.4 REQUIRED)
find_package(glm REQUIRED)
find_package(Vulkan REQUIRED)

find_path(SHADERC_INCLUDE_DIR NAMES shaderc/shaderc.h HINTS /opt/homebrew/include /usr/local/include)
find_library(SHADERC_LIB NAMES shaderc_shared shaderc_combined shaderc HINTS /opt/homebrew/lib /usr/local/lib)

add_executable(${PROJECT_NAME} main.cpp
        src/engine/shaders/compile.cpp
        src/engine/shaders/compile.h
        external/stb/stb_image.cpp
        external/tinyobjectloader/tiny_obj_loader.cpp
        src/engine/entities/manager.cpp
        src/engine/entities/manager.h
        src/engine/entities/components/component_manager.cpp
        src/engine/entities/components/component_manager.h
        src/engine/entities/system/system.cpp
        src/engine/entities/system/system.h
        src/engine/entities/system/movement_system.cpp
        src/engine/entities/system/movement_system.h
        src/engine/entities/components/entity_component_base.cpp
        src/engine/entities/components/entity_component_base.h
        src/engine/entities/components/transform_component.cpp
        src/engine/entities/components/transform_component.h
        src/engine/entities/components/renderable_component.cpp
        src/engine/entities/components/renderable_component.h
        src/engine/entities/components/velocity_component.cpp
        src/engine/entities/components/velocity_component.h
        src/engine/entities/system/system_manager.cpp
        src/engine/entities/system/system_manager.h
        src/engine/entities/system/display_system.cpp
        src/engine/entities/system/display_system.h
        src/engine/renderer/abstract.cpp
        src/engine/renderer/abstract.h
        src/engine/renderer/vulkan/vulkan_renderer.cpp
        src/engine/renderer/vulkan/vulkan_renderer.h
        src/engine/entities/components/camera_component.cpp
        src/engine/entities/components/camera_component.h
        src/engine/entities/system/camera_system.cpp
        src/engine/entities/system/camera_system.h
        src/engine/engine.cpp
        src/engine/engine.h
        src/engine/entities/components/tags/active_camera_tag_component.cpp
        src/engine/entities/components/tags/active_camera_tag_component.h
        src/engine/utils/math_utils.cpp
        src/engine/utils/math_utils.h
        src/engine/utils/entity_utils.cpp
        src/engine/utils/entity_utils.h
        src/engine/entities/types.h
        src/engine/renderer/vulkan/consts.h
        src/engine/renderer/vulkan/utils.cpp
        src/engine/renderer/vulkan/utils.h
        src/engine/renderer/vulkan/types.h
        src/engine/renderer/vulkan/vertex.cpp
        src/engine/renderer/vulkan/vertex.h
        src/engine/models/mesh_manager/abstract_mesh_manager.cpp
        src/engine/models/mesh_manager/abstract_mesh_manager.h
        src/engine/models/mesh_manager/vulkan_mesh_manager.cpp
        src/engine/models/mesh_manager/vulkan_mesh_manager.h
        src/engine/models/mesh_manager/vulkan_mesh_manager.h
        src/engine/utils/type_utils.h
        src/engine/utils/concepts/hashable.h
        src/engine/utils/vectors.h
        src/engine/models/texture_manager/vulkan_texture_manager.cpp
        src/engine/models/texture_manager/vulkan_texture_manager.h
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

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <shaderc/shaderc.hpp>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES

#include <glm/glm.hpp>

#include <chrono>

#include <iostream>
#include <stdexcept>
#include <cstdlib>

#include "src/editor/editor.h"

#include "src/engine/engine.h"
#include "src/engine/renderer/vulkan/vulkan_renderer.h"
#include "src/editor/renderer/vulkan/vulkan_renderer_with_ui.h"

constexpr uint32_t WIDTH = 1920;
constexpr uint32_t HEIGHT = 1080;

int main() {
    const auto g_Renderer = std::make_shared<Vulkan::RendererWithUi>();

    const auto g_Engine = std::make_shared<Engine>(std::static_pointer_cast<AbstractRenderer>(g_Renderer));

    const auto g_Editor = std::make_shared<Editor>(g_Engine);

    g_Engine->LoadScene("../.editor_data/scenes/scene1.scene");

    bool hasError = false;
    try {
        g_Editor->Run(WIDTH, HEIGHT);
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        hasError = true;
    }

    return hasError ? EXIT_FAILURE : EXIT_SUCCESS;
}

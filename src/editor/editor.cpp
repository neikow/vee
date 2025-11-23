#include "editor.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
bool show_demo_window = true;

void Editor::DrawSceneHierarchy() {
    ImGui::ShowDemoWindow(&show_demo_window);
}

void Editor::DrawInspector() {
}

void Editor::Run(const int width, const int height) {
    m_Engine->Initialize(width, height, "Editor", VK_MAKE_VERSION(1, 0, 0));

    auto lastTime = std::chrono::high_resolution_clock::now();

    while (!m_Engine->GetRenderer()->ShouldClose()) {
        glfwPollEvents();

        const auto currentTime = std::chrono::high_resolution_clock::now();
        const float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;

        m_Engine->Update(deltaTime);

        ImGui::NewFrame();

        ImGui::DockSpaceOverViewport(
            ImGui::GetID("MyDockSpace"),
            ImGui::GetMainViewport()
        );

        {
            ImGui::Begin("Component Editor");
            ImGui::Text("Hello from the editor!");
            ImGui::End();
        }
        {
            ImGui::Begin("Scene Hierarchy");
            ImGui::Text("Hello from the editor!");
            ImGui::End();
        }
        {
            ImGui::Begin("Assets Manager");
            ImGui::Text("Hello from the editor!");
            ImGui::End();
        }

        // DrawSceneHierarchy();
        DrawInspector();

        ImGui::Render();

        m_Engine->GetRenderer()->SubmitUIDrawData(ImGui::GetDrawData());
    }

    m_Engine->Shutdown();
}

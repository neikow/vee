#include "engine.h"

#include "entities/system/movement_system.h"
#include "serialization/scene_serializer.h"

void Engine::Initialize(const int width, const int height, const std::string &appName, const uint32_t version) const {
    m_Renderer->Initialize(width, height, appName, version);
}

void Engine::Update(const float deltaTime) const {
    if (!m_Paused) {
        m_Scene->GetSystemManager()->UpdateSystems(deltaTime);
    }
    m_Scene->GetDisplaySystem()->Render(0.01f);
}

void Engine::Shutdown() const {
    m_Renderer->WaitIdle();
    m_Renderer->Cleanup();
}

void Engine::LoadScene(const std::string &scenePath) {
    m_Scene = SceneSerializer::LoadScene(scenePath, m_Renderer);
}

bool Engine::Paused() const {
    return m_Paused;
}

bool Engine::ShouldQuit() const {
    return m_ShouldQuit;
}

std::shared_ptr<AbstractRenderer> Engine::GetRenderer() const {
    return m_Renderer;
}

std::shared_ptr<Scene> Engine::GetScene() {
    return m_Scene;
}

void Engine::Pause() {
    m_Paused = true;
}

void Engine::Resume() {
    m_Paused = false;
}

void Engine::Reset() {
    m_Paused = false;
    // TODO: Implement reset
}

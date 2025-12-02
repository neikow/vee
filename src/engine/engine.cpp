#include "engine.h"

#include "entities/system/movement_system.h"
#include "serialization/scene_serializer.h"

void Engine::Initialize(const int width, const int height, const std::string &appName, const uint32_t version) const {
    m_Renderer->Initialize(width, height, appName, version);
}

void Engine::Update(const float deltaTime) const {
    m_Scene->GetSystemManager()->UpdateSystems(m_Paused ? 0.0f : deltaTime);
    m_Scene->GetDisplaySystem()->Render(m_ActiveCameraEntityId);
}

void Engine::Shutdown() const {
    m_Renderer->WaitIdle();
    m_Renderer->Cleanup();
}

void Engine::LoadScene(const std::string &scenePath) {
    if (m_Renderer->Initialized()) m_Renderer->Reset();
    m_Scene = SceneSerializer::LoadScene(scenePath, m_Renderer, m_IsEditorMode);
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
    m_Scene = SceneSerializer::LoadScene(m_Scene->GetPath(), m_Renderer, m_IsEditorMode);
}

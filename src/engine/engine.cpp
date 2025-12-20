#include "engine.h"

#include "entities/components_system/components/physics_settings_component.h"
#include "entities/system/movement_system.h"
#include "serialization/scene_serializer.h"

void Engine::Initialize(const std::string &appName, const uint32_t version) const {
    m_Renderer->Initialize(appName, version);
}

void Engine::UpdateSystems(const float deltaTime) const {
    m_Scene->GetSystemManager()->UpdateSystems(m_Paused ? 0.0f : deltaTime);
}

void Engine::RegisterSystems(const SystemRegistrationFunction &regFunction) {
    m_SystemRegistrations.push_back(regFunction);
}

void Engine::PrepareForRendering() const {
    m_Scene->GetDisplaySystem()->PrepareForRendering(m_ActiveCameraEntityId);
    m_Renderer->PrepareForRendering();
}

void Engine::Shutdown() const {
    m_Renderer->WaitIdle();
    m_Renderer->Cleanup();
}

void Engine::CreateInternalEntities() const {
    const auto sceneEntity = m_Scene->GetEntityManager()->CreateEntity("Scene");
    m_Scene->SetSceneEntity(sceneEntity);
    const auto componentManager = m_Scene->GetComponentManager();
    componentManager->AddComponent<InternalTagComponent>(sceneEntity, InternalTagComponent{});
    componentManager->AddComponent<PhysicsSettingsComponent>(sceneEntity, PhysicsSettingsComponent{});
}

void Engine::LoadScene(const std::string &scenePath) {
    if (m_Renderer->Initialized()) m_Renderer->Reset();
    m_Scene = SceneSerializer::LoadScene(scenePath, m_Renderer, m_SystemRegistrations);
    CreateInternalEntities();
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
    m_Scene = SceneSerializer::LoadScene(m_Scene->GetPath(), m_Renderer, m_SystemRegistrations);
}

void Engine::NewEmptyScene() {
    if (m_Renderer->Initialized()) m_Renderer->Reset();
    m_Scene = SceneSerializer::LoadScene(
        "../src/editor/assets/scenes/empty.scene",
        m_Renderer,
        m_SystemRegistrations
    );
    m_Scene->SetPath("");
}

#include "engine.h"

void Engine::Initialize(const int width, const int height, const std::string &appName, const uint32_t version) const {
    m_Renderer->Initialize(width, height, appName, version);
}

void Engine::Update(const float deltaTime) const {
    m_SystemManager->UpdateSystems(deltaTime);
    m_DisplaySystem->Render(0.01f);
}

void Engine::Shutdown() const {
    m_Renderer->WaitIdle();
    m_Renderer->Cleanup();
}

bool Engine::ShouldQuit() const {
    return m_ShouldQuit;
}

std::shared_ptr<AbstractRenderer> Engine::GetRenderer() const {
    return m_Renderer;
}

void Engine::RegisterInternalSystems() {
    Signature renderableSignature;
    renderableSignature.set(ComponentTypeHelper<RenderableComponent>::ID);
    renderableSignature.set(ComponentTypeHelper<TransformComponent>::ID);
    m_DisplaySystem = m_SystemManager->RegisterSystem<DisplaySystem>(
        std::make_shared<DisplaySystem>(
            m_Renderer,
            m_ComponentManager,
            m_EntityManager
        )
    );
    m_SystemManager->SetSignature<DisplaySystem>(renderableSignature);

    Signature activeCameraSignature;
    activeCameraSignature.set(ComponentTypeHelper<CameraComponent>::ID);
    activeCameraSignature.set(ComponentTypeHelper<ActiveCameraTagComponent>::ID);

    m_SystemManager->RegisterSystem<CameraSystem>(
        std::make_shared<CameraSystem>(
            m_Renderer,
            m_ComponentManager
        )
    );
    m_SystemManager->SetSignature<CameraSystem>(activeCameraSignature);

    Signature movementSignature;
    movementSignature.set(ComponentTypeHelper<TransformComponent>::ID);
    movementSignature.set(ComponentTypeHelper<VelocityComponent>::ID);
    m_SystemManager->RegisterSystem<MovementSystem>(
        std::make_shared<MovementSystem>(m_ComponentManager)
    );
    m_SystemManager->SetSignature<MovementSystem>(movementSignature);
}

void Engine::RegisterInternalComponents() {
    m_ComponentManager->RegisterComponent<TransformComponent>();
    m_ComponentManager->RegisterComponent<VelocityComponent>();
    m_ComponentManager->RegisterComponent<RenderableComponent>();
    m_ComponentManager->RegisterComponent<CameraComponent>();
    m_ComponentManager->RegisterComponent<ActiveCameraTagComponent>();
}

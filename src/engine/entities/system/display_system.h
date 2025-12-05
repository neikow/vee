#ifndef GAME_ENGINE_DISPLAY_SYSTEM_H
#define GAME_ENGINE_DISPLAY_SYSTEM_H
#include "system.h"
#include "system_manager.h"

#include "../../renderer/abstract.h"

class DisplaySystem final : public SystemBase {
    std::shared_ptr<AbstractRenderer> m_Renderer;
    std::shared_ptr<EntityManager> m_EntityManager;

public:
    DisplaySystem(
        const std::shared_ptr<AbstractRenderer> &renderer,
        const std::shared_ptr<ComponentManager> &componentManager,
        const std::shared_ptr<EntityManager> &entityManager
    ) : SystemBase(componentManager), m_Renderer(renderer),
        m_EntityManager(entityManager) {
    }

    void Update(float dt) override {
    };

    void PrepareForRendering(EntityID cameraEntityId) const;

private:
    void PrepareCamera(EntityID cameraEntityId) const;

    void SubmitDrawCalls() const;
};


#endif //GAME_ENGINE_DISPLAY_SYSTEM_H

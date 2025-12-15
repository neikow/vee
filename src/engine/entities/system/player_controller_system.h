#ifndef VEE_PLAYER_CONTROLLER_SYSTEM_H
#define VEE_PLAYER_CONTROLLER_SYSTEM_H
#include "system.h"

class PlayerControllerSystem final : public SystemBase {
public:
    explicit PlayerControllerSystem(const std::shared_ptr<ComponentManager> &componentManager);

    void Update(float dt) override;;
};


#endif //VEE_PLAYER_CONTROLLER_SYSTEM_H

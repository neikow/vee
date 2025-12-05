#ifndef VEE_SYSTEM_REGISTRATION_H
#define VEE_SYSTEM_REGISTRATION_H
#include <functional>

#include "system/system_manager.h"

using SystemRegistrationFunction = std::function<void(
    std::shared_ptr<EntityManager> &entityManager,
    std::shared_ptr<SystemManager> &systemManager,
    std::shared_ptr<ComponentManager> &componentManager
)>;

#endif //VEE_SYSTEM_REGISTRATION_H

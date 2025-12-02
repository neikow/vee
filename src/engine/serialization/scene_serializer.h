#ifndef VEE_SCENE_SERIALIZER_H
#define VEE_SCENE_SERIALIZER_H
#include <string>

#include "../scenes/scene.h"

using SceneVersionRaw = uint16_t;

enum class SceneVersion : SceneVersionRaw {
    V_0 = 0,
};

class SceneSerializer {
public:
    static std::unique_ptr<Scene> LoadScene(
        const std::string &scenePath,
        const std::shared_ptr<AbstractRenderer> &renderer, bool editorMode
    );
};


#endif //VEE_SCENE_SERIALIZER_H

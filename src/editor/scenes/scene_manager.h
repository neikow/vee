#ifndef VEE_SCENE_MANAGER_H
#define VEE_SCENE_MANAGER_H
#include <string>
#include <vector>

#include "../../engine/engine.h"

struct SceneData {
    std::string name;
    std::string path;
};

class SceneManager {
    std::shared_ptr<Engine> m_Engine;

public:
    explicit SceneManager(
        const std::shared_ptr<Engine> &engine
    )
        : m_Engine(engine) {
    }

    ~SceneManager() = default;

    void NewEmptyScene();

    [[nodiscard]] static std::vector<SceneData> ListScenes();

    void LoadScene(const std::string &path);

    void SaveScene(const std::string &path);
};


#endif //VEE_SCENE_MANAGER_H

#include "scene_manager.h"
#include <iostream>
#include <dirent.h>
#include <sys/types.h>

#include "yaml-cpp/depthguard.h"
#include "yaml-cpp/node/node.h"
#include "yaml-cpp/node/parse.h"


void SceneManager::NewEmptyScene() {
}

SceneData ParseSceneFile(const std::string &filePath) {
    SceneData sceneData;
    YAML::Node data;
    try {
        data = YAML::LoadFile(filePath);
    } catch (YAML::ParserException &e) {
        throw std::runtime_error("Failed to load scene from YAML, error: " + std::string(e.what()));
    }

    if (data["name"]) {
        sceneData.name = data["name"].as<std::string>();
    } else {
        sceneData.name = "Unnamed Scene";
    }

    if (data["type"].as<std::string>() != "scene") {
        throw std::runtime_error("Invalid scene file: missing 'scene' type");
    }

    sceneData.path = filePath;

    return sceneData;
}

std::vector<SceneData> SceneManager::ListScenes() {
    dirent *en;
    const std::string dir_path = "../.editor_data/scenes/";
    DIR *dir = opendir(dir_path.c_str());
    if (dir) {
        std::vector<SceneData> scenes;
        while ((en = readdir(dir)) != nullptr) {
            std::string fileName = en->d_name;
            if (fileName.length() > 6 && fileName.substr(fileName.length() - 6) == ".scene") {
                try {
                    scenes.push_back(
                        ParseSceneFile(dir_path + fileName)
                    );
                } catch (const std::runtime_error &e) {
                    std::cerr << "[WARN] Failed to parse scene file '" << fileName << "': " << e.what() << std::endl;
                }
            }
        }
        closedir(dir);

        std::cout << "[INFO] Found " << scenes.size() << " scenes." << std::endl;

        return scenes;
    }

    std::cerr << "[WARN] Could not open scenes directory." << std::endl;

    return {};
}

void SceneManager::LoadScene(const std::string &path) {
    m_Engine->LoadScene(path);
}

void SceneManager::SaveScene(const std::string &path) {
}

#ifndef VEE_SAVE_SCENE_AS_MODAL_H
#define VEE_SAVE_SCENE_AS_MODAL_H
#include "types.h"

class SceneManager;

namespace Editor::UI::Modals {
    constexpr auto SAVE_SCENE_AS = "Save Scene As";

    class SaveSceneAsModal final : public IEditorModal {
        SceneManager *m_SceneManager;

    public:
        explicit SaveSceneAsModal(SceneManager *sceneManager);

        void Draw() override;
    };
}

#endif //VEE_SAVE_SCENE_AS_MODAL_H

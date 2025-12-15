#ifndef VEE_INSPECTOR_H
#define VEE_INSPECTOR_H
#include "../../editor.h"

namespace Editor::UI {
    class Inspector {
        /** Draws the Scene Inspector UI component.
         *
         * @param editor Pointer to the VeeEditor instance.
         * @param scene Shared pointer to the current scene.
         */
        static void DrawSceneInspector(const VeeEditor *editor, const std::shared_ptr<Scene> &scene);

        /** Draws the Entity Inspector UI component.
         *
         * @param editor Pointer to the VeeEditor instance.
         * @param scene Shared pointer to the current scene.
         * @param entity The ID of the entity to inspect.
         */
        static void DrawEntityInspector(VeeEditor *editor, const std::shared_ptr<Scene> &scene, EntityID entity);

    public:
        /** Draws the Inspector window.
         *
         * @param title The title of the window.
         * @param editor Pointer to the VeeEditor instance.
         */
        static void Draw(const char *title, VeeEditor *editor);
    };
}

#endif //VEE_INSPECTOR_H

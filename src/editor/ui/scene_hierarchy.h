#ifndef VEE_SCENE_HIERARCHY_H
#define VEE_SCENE_HIERARCHY_H
#include "../editor.h"
#include "../../engine/entities/components_system/components/children_component.h"
#include "../../engine/entities/components_system/components/parent_component.h"
#include "../../engine/scenes/scene.h"
#include "../../engine/utils/strings.h"

namespace Editor::UI {
    class SceneHierarchy {
        /** Draws the scene hierarchy UI component.
         *
         * @param editor Pointer to the VeeEditor instance.
         * @param scene Shared pointer to the current scene.
         */
        static void DrawHierarchy(
            VeeEditor *editor,
            const std::shared_ptr<Scene> &scene
        );

        /** Draws a single entity node in the scene hierarchy.
         *
         * Called recursively to draw child entities.
         *
         * @param editor Pointer to the VeeEditor instance.
         * @param scene Shared pointer to the current scene.
         * @param entityID The ID of the entity to draw.
         */
        static void DrawEntityNode(VeeEditor *editor, const std::shared_ptr<Scene> &scene, EntityID entityID);

        /** Draws the scene picker UI component.
         *
         * @param editor Pointer to the VeeEditor instance.
         * @param scene Shared pointer to the current scene.
         */
        static void DrawScenePicker(
            VeeEditor *editor,
            const std::shared_ptr<Scene> &scene
        );

    public:
        /** Draws the Scene Hierarchy window.
         *
         * @param title The title of the window.
         * @param editor Pointer to the VeeEditor instance.
         */
        static void Draw(
            const char *title,
            VeeEditor *editor
        );
    };
}


#endif //VEE_SCENE_HIERARCHY_H

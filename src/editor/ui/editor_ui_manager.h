#ifndef VEE_EDITOR_UI_MANAGER_H
#define VEE_EDITOR_UI_MANAGER_H
#include <unordered_map>

#include "interface/asset_manager.h"

namespace Editor {
    class UIManager {
        std::unordered_map<const char *, bool> m_ModalsVisibility;

    public:
        /** Opens a modal dialog with the given ID.
         *
         * @param modalName The unique identifier for the modal dialog.
         */
        void OpenModal(const char *modalName);

        /** Closes a modal dialog with the given ID.
         *
         * @param modalName The unique identifier for the modal dialog.
         */
        bool ShouldDrawModal(const char *modalName) const;

        /** Marks a modal dialog as closed.
         *
         * @param modalName The unique identifier for the modal dialog.
         */
        void CloseModal(const char *modalName);
    };
}


#endif //VEE_EDITOR_UI_MANAGER_H

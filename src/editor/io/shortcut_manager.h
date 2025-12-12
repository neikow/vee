#ifndef VEE_SHORTCUT_MANAGER_H
#define VEE_SHORTCUT_MANAGER_H
#include <map>
#include <ranges>

#include "../../engine/io/input_system.h"

enum Shortcut {
    SHORTCUT_SAVE,
    SHORTCUT_RELOAD,
    SHORTCUT_ADD_ENTITY_TO_SCENE,
    SHORTCUT_DELETE,
};

class ShortcutManager {
    /** Mapping of shortcuts to their actions */
    std::map<Shortcut, std::function<void()> > m_Shortcuts;
    /** Debouncing state for each shortcut */
    std::map<Shortcut, bool> m_ShortcutsActive;

public:
    ShortcutManager() = default;

    void RegisterShortcut(
        const Shortcut &shortcut,
        const std::vector<Key> &keys,
        const std::function<void()> &action
    );

    void HandleShortcuts();
};


#endif //VEE_SHORTCUT_MANAGER_H

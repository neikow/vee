#include "shortcut_manager.h"

void ShortcutManager::RegisterShortcut(
    const Shortcut &shortcut,
    const std::vector<Key> &keys,
    const std::function<void()> &action
) {
    if (m_Shortcuts.contains(shortcut)) {
        // TODO: use a proper logging system
        throw std::runtime_error("Shortcut already registered");
    }

    m_Shortcuts[shortcut] = [this, shortcut, keys, action] {
        bool allPressed = true;
        for (const auto &key: keys) {
            if (!InputSystem::IsKeyPressed(key)) {
                allPressed = false;
                break;
            }
        }
        if (allPressed) {
            if (!m_ShortcutsActive[shortcut]) action();
            m_ShortcutsActive[shortcut] = true;
        } else {
            m_ShortcutsActive[shortcut] = false;
        }
    };
}

void ShortcutManager::HandleShortcuts() {
    for (const auto &action: m_Shortcuts | std::views::values) {
        action();
    }
}

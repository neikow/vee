#include "editor_console.h"

ImVec4 GetLogLevelColor(const LogLevel level) {
    switch (level) {
        case LogLevel::INFO: return {0.7f, 0.7f, 0.7f, 1.0f}; // Light Gray
        case LogLevel::WARNING: return {1.0f, 1.0f, 0.0f, 1.0f}; // Yellow
        case LogLevel::ERROR: return {1.0f, 0.4f, 0.4f, 1.0f}; // Red
        case LogLevel::DEBUG: return {0.4f, 0.8f, 1.0f, 1.0f}; // Light Blue
        default: return {1.0f, 1.0f, 1.0f, 1.0f};
    }
}

void EditorConsole::Draw(const char *title, bool *p_open) {
    if (!ImGui::Begin(title, p_open)) {
        ImGui::End();
        return;
    }

    if (ImGui::Button("Clear")) {
        Logger::Clear();
    }

    auto console = GetInstance();

    ImGui::SameLine();
    bool auto_scroll = console.m_ScrollToBottom;
    if (ImGui::Checkbox("Auto-scroll", &auto_scroll)) {
        console.m_ScrollToBottom = auto_scroll;
    }

    ImGui::Separator();

    const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
    ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve), false,
                      ImGuiWindowFlags_HorizontalScrollbar);

    const std::vector<Message> messages = Logger::GetMessages();

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1));

    for (const auto &msg: messages) {
        ImVec4 color = GetLogLevelColor(msg.level);
        ImGui::PushStyleColor(ImGuiCol_Text, color);

        std::string line = "[" + msg.timestamp + "]";

        if (msg.hasLocation) {
            line += " (" + msg.location.file + ":" + std::to_string(msg.location.line) + ")";
        }

        line += ": " + msg.text;

        ImGui::TextUnformatted(line.c_str());

        ImGui::PopStyleColor();
    }

    if (console.m_ScrollToBottom && (ImGui::GetScrollY() >= ImGui::GetScrollMaxY() || !messages.empty())) {
        ImGui::SetScrollHereY(1.0f);
        console.m_ScrollToBottom = false;
    }

    ImGui::PopStyleVar();
    ImGui::EndChild();

    ImGui::End();
}

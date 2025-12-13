#include "logger.h"

#include "../utils/entity_utils.h"
#include "../utils/timestamp.h"

void Logger::Log(const LogLevel level, const std::string &text, const Location &location) {
    GetInstance().m_Messages.Push({
        .level = level,
        .text = text,
        .timestamp = Utils::GetCurrentTimestamp(),
        .location = location,
        .hasLocation = !location.file.empty()
    });
}

void Logger::Clear() {
    GetInstance().m_Messages.Clear();
}

std::vector<Message> Logger::GetMessages() {
    return GetInstance().m_Messages.AsVector();
}

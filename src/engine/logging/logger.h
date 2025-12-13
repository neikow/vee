#ifndef VEE_LOGGER_H
#define VEE_LOGGER_H
#include "../utils/circular_buffer.h"


struct Location {
    std::string file;
    int line = 0;
};

enum class LogLevel {
    INFO,
    WARNING,
    ERROR,
    DEBUG
};

struct Message {
    LogLevel level = LogLevel::INFO;
    std::string text;
    std::string timestamp;

    Location location;
    bool hasLocation = false;
};

constexpr auto MESSAGE_CAPACITY = 4096;

class Logger {
    CircularBuffer<Message> m_Messages{MESSAGE_CAPACITY};

    Logger() = default;

public:
    static Logger &GetInstance() {
        static Logger instance;
        return instance;
    }

    static void Log(LogLevel level, const std::string &text, const Location &location);

    static void Log(const LogLevel level, const std::string &text) {
        Log(level, text, {});
    }

    static void Info(const std::string &text, const Location &location) {
        Log(LogLevel::INFO, text, location);
    }

    static void Info(const std::string &text) {
        Log(LogLevel::INFO, text, {});
    }

    static void Warn(const std::string &text, const Location &location) {
        Log(LogLevel::WARNING, text, location);
    }

    static void Warn(const std::string &text) {
        Log(LogLevel::WARNING, text, {});
    }

    static void Error(const std::string &text, const Location &location) {
        Log(LogLevel::ERROR, text, location);
    }

    static void Error(const std::string &text) {
        Log(LogLevel::ERROR, text, {});
    }

    static void Debug(const std::string &text, const Location &location) {
        Log(LogLevel::DEBUG, text, location);
    }

    static void Debug(const std::string &text) {
        Log(LogLevel::DEBUG, text, {});
    }

    static void Clear();

    static std::vector<Message> GetMessages();
};

#endif //VEE_LOGGER_H

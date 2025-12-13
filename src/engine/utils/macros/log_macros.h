#ifndef VEE_LOG_MACROS_H
#define VEE_LOG_MACROS_H

#include "../../logging/logger.h"

#define LOG_INFO(text) Logger::Info(text, {.file = __FILE__, .line = __LINE__})
#define LOG_WARN(text) Logger::Warn(text, {.file = __FILE__, .line = __LINE__})
#define LOG_ERROR(text) Logger::Error(text, {.file = __FILE__, .line = __LINE__})
#define LOG_DEBUG(text) Logger::Debug(text, {.file = __FILE__, .line = __LINE__})

#endif //VEE_LOG_MACROS_H

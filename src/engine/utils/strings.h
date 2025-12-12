#ifndef VEE_STRINGS_H
#define VEE_STRINGS_H

#include <string>

namespace Utils::Strings {
    std::string ReplaceAll(
        const std::string &input,
        const std::string &search,
        const std::string &replace
    );

    std::string TrimWhitespace(
        const std::string &input
    );

    std::string ToLower(
        const std::string &input
    );

    std::string TruncateString(const std::string &name, int i);
}

#endif //VEE_STRINGS_H

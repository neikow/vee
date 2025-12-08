#include "strings.h"

namespace Utils::Strings {
    std::string ReplaceAll(
        const std::string &input,
        const std::string &search,
        const std::string &replace
    ) {
        std::string result = input;
        size_t pos = 0;
        while ((pos = result.find(search, pos)) != std::string::npos) {
            result.replace(pos, search.length(), replace);
            pos += replace.length();
        }
        return result;
    }

    std::string TrimWhitespace(
        const std::string &input
    ) {
        const std::string whitespace = " \t\n\r\f\v";
        const size_t start = input.find_first_not_of(whitespace);
        if (start == std::string::npos) {
            return "";
        }
        const size_t end = input.find_last_not_of(whitespace);
        return input.substr(start, end - start + 1);
    }

    std::string ToLower(const std::string &input) {
        std::string result = input;
        for (char &c: result) {
            c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        }
        return result;
    }
}

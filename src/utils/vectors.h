#ifndef GAME_ENGINE_UTILS_VECTORS_H
#define GAME_ENGINE_UTILS_VECTORS_H

#include <vector>
#include <algorithm>
#include <iterator>

namespace Utils::Vectors {
    template<typename T>
    std::vector<T> FlattenCopy(const std::vector<std::vector<T> > &nested_vec) {
        std::vector<T> result;

        size_t total_size = 0;
        for (const auto &inner_vec: nested_vec) {
            total_size += inner_vec.size();
        }
        result.reserve(total_size);

        for (const auto &inner_vec: nested_vec) {
            std::copy(
                inner_vec.begin(),
                inner_vec.end(),
                std::back_inserter(result)
            );
        }
        return result;
    }
}

#endif //GAME_ENGINE_UTILS_VECTORS_H

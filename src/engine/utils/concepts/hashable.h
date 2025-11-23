#ifndef GAME_ENGINE_UTILS_CONCEPTS_HASHABLE_H
#define GAME_ENGINE_UTILS_CONCEPTS_HASHABLE_H
#include <functional>

template<typename T>
concept Hashable = requires(const T &a, const T &b)
{
    { a == b } -> std::same_as<bool>;
    { std::hash<T>{}(a) } -> std::same_as<std::size_t>;
};

#endif //GAME_ENGINE_UTILS_CONCEPTS_HASHABLE_H

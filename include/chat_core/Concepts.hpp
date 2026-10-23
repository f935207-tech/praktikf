#pragma once
#include <concepts>
#include <string>
#include <string_view>
#include <vector>
#include <cstdint>
#include <type_traits>

template <typename T>
concept Serializable = requires(T obj, const T const_obj, std::vector<std::byte> bytes) {
    { const_obj.to_bytes() } -> std::convertible_to<std::vector<std::byte>>;
    { T::from_bytes(bytes) } -> std::convertible_to<T>;
};

template <typename T>
concept NickLike = requires(T nick) {
    { std::string_view(nick) } -> std::convertible_to<std::string_view>;
    { nick == nick } -> std::convertible_to<bool>;
} && !std::is_arithmetic_v<std::remove_cvref_t<T>>;
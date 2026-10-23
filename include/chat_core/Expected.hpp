#pragma once
#include <variant>
#include <utility>

template<typename T, typename E>
class Expected {
    std::variant<T, E> data;
public:
    Expected(T val) : data(std::move(val)) {}
    Expected(E err) : data(std::move(err)) {}
    
    bool has_value() const { return std::holds_alternative<T>(data); }
    explicit operator bool() const { return has_value(); }
    
    const T& value() const { return std::get<T>(data); }
    const E& error() const { return std::get<E>(data); }
};
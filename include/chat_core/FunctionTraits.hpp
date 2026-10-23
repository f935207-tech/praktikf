#pragma once
#include <tuple>
#include <type_traits>
#include <string_view>

template <typename T>
struct function_traits;

template <typename R, typename... Args>
struct function_traits<R(*)(Args...)> {
    using return_type = R;
    using args_tuple = std::tuple<Args...>;
    static constexpr std::size_t arity = sizeof...(Args);
    
    template <std::size_t N>
    using arg = typename std::tuple_element<N, args_tuple>::type;
};

template <typename T>
struct function_traits : function_traits<decltype(&T::operator())> {};

template <typename C, typename R, typename... Args>
struct function_traits<R(C::*)(Args...) const> {
    using return_type = R;
    using args_tuple = std::tuple<Args...>;
    static constexpr std::size_t arity = sizeof...(Args);
    
    template <std::size_t N>
    using arg = typename std::tuple_element<N, args_tuple>::type;
};
#pragma once
#include "Convert.hpp"
#include "FunctionTraits.hpp"
#include "ChatStore.hpp"
#include <functional>
#include <unordered_map>
#include <string>
#include <vector>
#include <iostream>

template <typename T>
concept CommandHandler = requires(T handler, std::string_view arg) {
    { handler(arg) } -> std::convertible_to<int>;
};

class CommandRegistry {
private:
    using HandlerFunc = std::function<int(const std::vector<std::string_view>&, ChatStore&)>;
    std::unordered_map<std::string, HandlerFunc> handlers_;

public:
    template <typename Handler>
    requires CommandHandler<Handler>
    void register_command(const std::string& name, Handler handler) {
        using Traits = function_traits<Handler>;
        constexpr std::size_t arity = Traits::arity;

        handlers_[name] = [handler](const std::vector<std::string_view>& tokens, ChatStore& store) mutable -> int {
            if (tokens.size() != arity) {
                std::cerr << "Error: command expects " << arity << " arguments, got " << tokens.size() << "\n";
                return 1;
            }

            return invoke_with_conversion(handler, tokens, store, std::make_index_sequence<arity>{});
        };
    }

    int execute(const std::string& cmd, const std::vector<std::string_view>& tokens, ChatStore& store) {
        auto it = handlers_.find(cmd);
        if (it != handlers_.end()) {
            return it->second(tokens, store);
        }
        std::cerr << "Unknown command: " << cmd << "\n";
        return 1;
    }

private:
    template <typename Handler, std::size_t... Is>
    int invoke_with_conversion(Handler& handler, const std::vector<std::string_view>& tokens, ChatStore& store, std::index_sequence<Is...>) {
        using Traits = function_traits<Handler>;
        return handler(store, convert<typename Traits::template arg<Is>>(tokens[Is])...);
    }
};
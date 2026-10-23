#include "chat_core/Command.hpp"
#include "chat_core/Lexer.hpp"

#include <vector>
#include <string_view>
#include <charconv>
#include <system_error>

namespace {

std::string join_tail(const std::vector<std::string_view>& tokens,
                      std::size_t from) {
    std::string result;
    for (std::size_t i = from; i < tokens.size(); ++i) {
        if (i > from) {
            result.push_back(' ');
        }
        result.append(tokens[i].data(), tokens[i].size());
    }
    return result;
}

bool parse_size(std::string_view s, std::size_t& out) {
    if (s.empty()) {
        return false;
    }
    std::size_t value = 0;
    auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), value);
    if (ec != std::errc() || ptr != s.data() + s.size()) {
        return false;
    }
    out = value;
    return true;
}

}

std::optional<Command> parse_command(const std::vector<std::string_view>& tokens) {
    if (tokens.empty()) {
        return std::nullopt;
    }

    const std::string_view cmd = tokens[0];

    if (cmd == "/join") {
        if (tokens.size() < 2) return std::nullopt;
        return JoinCommand{ std::string(tokens[1]) };
    }

    if (cmd == "/leave") {
        if (tokens.size() < 2) return std::nullopt;
        return LeaveCommand{ std::string(tokens[1]) };
    }

    if (cmd == "/msg") {
        if (tokens.size() < 3) return std::nullopt;
        return MsgCommand{
            std::string(tokens[1]),
            join_tail(tokens, 2)
        };
    }

    if (cmd == "/who") {
        if (tokens.size() < 2) return std::nullopt;
        return WhoCommand{ std::string(tokens[1]) };
    }

    if (cmd == "/history") {
        if (tokens.size() < 2) return std::nullopt;
        return HistoryCommand{ std::string(tokens[1]) };
    }

    if (cmd == "/type") {
        if (tokens.size() < 3) return std::nullopt;
        std::size_t idx = 0;
        if (!parse_size(tokens[2], idx)) return std::nullopt;
        return TypeCommand{ std::string(tokens[1]), idx };
    }

    if (cmd == "/react") {
        if (tokens.size() < 3) return std::nullopt;
        std::size_t idx = 0;
        if (!parse_size(tokens[2], idx)) return std::nullopt;
        return ReactCommand{ std::string(tokens[1]), idx };
    }

    return std::nullopt;
}
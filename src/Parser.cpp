#include "chat_core/Command.hpp"
#include "chat_core/Lexer.hpp"
#include <charconv>
#include <system_error>

namespace {
std::string join_tail(const std::vector<std::string_view>& tokens, std::size_t from) {
    std::string result;
    for (std::size_t i = from; i < tokens.size(); ++i) {
        if (i > from) result.push_back(' ');
        result.append(tokens[i].data(), tokens[i].size());
    }
    return result;
}

bool parse_size(std::string_view s, std::size_t& out) {
    if (s.empty()) return false;
    std::size_t value = 0;
    auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), value);
    if (ec != std::errc() || ptr != s.data() + s.size()) return false;
    out = value;
    return true;
}

// Общая логика парсинга
Command parse_impl(const std::vector<std::string_view>& tokens) {
    if (tokens.empty()) throw ParseError("Empty command");

    const std::string_view cmd = tokens[0];

    if (cmd == "/join") {
        if (tokens.size() < 2) throw ParseError("/join requires room");
        return JoinCommand{ std::string(tokens[1]) };
    }
    if (cmd == "/leave") {
        if (tokens.size() < 2) throw ParseError("/leave requires room");
        return LeaveCommand{ std::string(tokens[1]) };
    }
    if (cmd == "/msg") {
        if (tokens.size() < 3) throw ParseError("/msg requires rooms and text");
        std::vector<std::string> rooms;
        std::size_t i = 1;
        while (i < tokens.size() && tokens[i][0] == '#') {
            rooms.push_back(std::string(tokens[i].substr(1))); // убираем '#'
            ++i;
        }
        if (rooms.empty()) throw ParseError("/msg requires at least one #room");
        return MsgCommand{ rooms, join_tail(tokens, i) };
    }
    if (cmd == "/who") {
        if (tokens.size() < 2) throw ParseError("/who requires room");
        return WhoCommand{ std::string(tokens[1]) };
    }
    if (cmd == "/history") {
        if (tokens.size() < 2) throw ParseError("/history requires room");
        return HistoryCommand{ std::string(tokens[1]) };
    }
    if (cmd == "/type" || cmd == "/react") {
        if (tokens.size() < 3) throw ParseError("Command requires room and index");
        std::size_t idx = 0;
        if (!parse_size(tokens[2], idx)) throw ParseError("Invalid index");
        if (cmd == "/type") return TypeCommand{ std::string(tokens[1]), idx };
        return ReactCommand{ std::string(tokens[1]), idx };
    }

    throw ParseError("Unknown command");
}
}

// Версия 1
std::optional<Command> parse_command(const std::vector<std::string_view>& tokens) {
    try {
        return parse_impl(tokens);
    } catch (const ParseError&) {
        return std::nullopt;
    }
}

// Версия 2
Expected<Command, ParseError> parse_command_expected(const std::vector<std::string_view>& tokens) {
    try {
        return Expected<Command, ParseError>(parse_impl(tokens));
    } catch (const ParseError& e) {
        return Expected<Command, ParseError>(e);
    }
}
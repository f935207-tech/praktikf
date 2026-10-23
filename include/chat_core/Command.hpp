#pragma once

#include "Errors.hpp"
#include "Expected.hpp"

#include <string>
#include <string_view>
#include <variant>
#include <optional>
#include <vector>

struct JoinCommand    { std::string room; };
struct LeaveCommand   { std::string room; };
struct MsgCommand     { std::vector<std::string> rooms; std::string text; };
struct WhoCommand     { std::string room; };
struct HistoryCommand { std::string room; };
struct TypeCommand    { std::string room; std::size_t index; };
struct ReactCommand   { std::string room; std::size_t index; };

using Command = std::variant<
    JoinCommand, LeaveCommand, MsgCommand, WhoCommand, 
    HistoryCommand, TypeCommand, ReactCommand
>;

std::optional<Command> parse_command(const std::vector<std::string_view>& tokens);
Expected<Command, ParseError> parse_command_expected(const std::vector<std::string_view>& tokens);
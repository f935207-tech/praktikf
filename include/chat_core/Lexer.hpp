#pragma once

#include <string_view>
#include <vector>

class Lexer {
public:
    // Разбивает input на токены. Не копирует символы.
    static std::vector<std::string_view> lex(std::string_view input);
};
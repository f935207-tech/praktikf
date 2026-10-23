#include "chat_core/Lexer.hpp"
#include <cctype>

namespace {

bool is_space(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

bool is_special(char c) {
    return is_space(c) || c == '"';
}

} 

std::vector<std::string_view> Lexer::lex(std::string_view input) {
    std::vector<std::string_view> tokens;

    std::size_t i = 0;
    const std::size_t n = input.size();

    while (i < n) {
        while (i < n && is_space(input[i])) {
            ++i;
        }
        if (i >= n) {
            break;
        }

        if (input[i] == '"') {
            ++i; 
            std::size_t start = i;
            while (i < n && input[i] != '"') {
                ++i;
            }
            tokens.emplace_back(input.substr(start, i - start));
            if (i < n) {
                ++i; 
            }
            continue;
        }

        std::size_t start = i;
        while (i < n && !is_special(input[i])) {
            ++i;
        }
        tokens.emplace_back(input.substr(start, i - start));
    }
    return tokens;
}
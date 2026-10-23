#pragma once

#include <string>
#include <chrono>
#include <variant>

struct TextBody {
    std::string text;
};

struct SystemNotice {
    std::string text;  
};

struct Reaction {
    int count = 0;
};

using Body = std::variant<TextBody, SystemNotice, Reaction>;

class Message {
public:
    std::string author;
    Body body;
    std::chrono::system_clock::time_point timestamp;

    Message() = default;

    Message(std::string a, std::string t)
        : author(std::move(a)),
          body(TextBody{std::move(t)}),
          timestamp(std::chrono::system_clock::now()) {}

    Message(std::string a, Body b)
        : author(std::move(a)),
          body(std::move(b)),
          timestamp(std::chrono::system_clock::now()) {}
};
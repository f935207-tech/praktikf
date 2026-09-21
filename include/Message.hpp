#pragma once

#include <string>
#include <chrono>

class Message {
public:
    std::string author;
    std::string text;
    std::chrono::system_clock::time_point timestamp;

    Message() = default;

    Message(std::string a, std::string t)
        : author(std::move(a)),
          text(std::move(t)),
          timestamp(std::chrono::system_clock::now()) {}
};
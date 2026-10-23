#pragma once

#include <string>
#include <chrono>
#include <variant>
#include <stdexcept>

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
    static inline bool throw_on_copy = false;

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

    Message(const Message& other)
        : author(other.author),
          body(other.body),
          timestamp(other.timestamp) 
    {
        if (throw_on_copy) {
            throw std::runtime_error("Simulated copy failure inside Message");
        }
    }

    Message& operator=(const Message& other) {
        if (this != &other) {
            if (throw_on_copy) {
                throw std::runtime_error("Simulated copy failure inside Message");
            }
            author = other.author;
            body = other.body;
            timestamp = other.timestamp;
        }
        return *this;
    }

    Message(Message&& other) noexcept = default;
    Message& operator=(Message&& other) noexcept = default;
};
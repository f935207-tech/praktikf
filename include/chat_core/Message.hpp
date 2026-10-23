#pragma once
#include <string>
#include <chrono>
#include <memory>
#include "IMessage.hpp"

class Message {
public:
    static inline bool throw_on_copy = false;

    std::string author;
    std::unique_ptr<IMessage> body; 
    std::chrono::system_clock::time_point timestamp;

    Message() = default;

    Message(std::string a, std::unique_ptr<IMessage> b)
        : author(std::move(a)), body(std::move(b)), timestamp(std::chrono::system_clock::now()) {}

    Message(const Message& other)
        : author(other.author), timestamp(other.timestamp) {
        if (throw_on_copy) {
            throw std::runtime_error("Simulated copy failure");
        }
        if (other.body) {
            if (other.body->get_type() == "TextBody") 
                body = std::make_unique<TextMessage>(other.body->get_text());
            else if (other.body->get_type() == "SystemNotice") 
                body = std::make_unique<SystemMessage>(other.body->get_text());
            else if (other.body->get_type() == "Reaction") {
                auto* r = dynamic_cast<ReactionMessage*>(other.body.get());
                body = std::make_unique<ReactionMessage>(r ? r->get_count() : 0);
            }
        }
    }

    Message& operator=(const Message& other) {
        if (this != &other) {
            if (throw_on_copy) throw std::runtime_error("Simulated copy failure");
            Message tmp(other);
            *this = std::move(tmp);
        }
        return *this;
    }

    Message(Message&& other) noexcept = default;
    Message& operator=(Message&& other) noexcept = default;
};
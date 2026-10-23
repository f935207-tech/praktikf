#pragma once
#include <string>
#include <memory>
#include "Command.hpp"

class IMessage {
public:
    virtual ~IMessage() = default;
    virtual std::string get_type() const = 0;
    virtual std::string get_text() const = 0;
};

class TextMessage : public IMessage {
    std::string text_;
public:
    explicit TextMessage(std::string text) : text_(std::move(text)) {}
    std::string get_type() const override { return "TextBody"; }
    std::string get_text() const override { return text_; }
};

class SystemMessage : public IMessage {
    std::string text_;
public:
    explicit SystemMessage(std::string text) : text_(std::move(text)) {}
    std::string get_type() const override { return "SystemNotice"; }
    std::string get_text() const override { return text_; }
};

class ReactionMessage : public IMessage {
    int count_;
public:
    explicit ReactionMessage(int count = 0) : count_(count) {}
    std::string get_type() const override { return "Reaction"; }
    std::string get_text() const override { return "reaction x" + std::to_string(count_); }
    int get_count() const { return count_; }
    void increment() { ++count_; }
};
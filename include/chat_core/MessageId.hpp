#pragma once
#include <compare>
#include <chrono>
#include <string>

#pragma once
#include <compare>
#include <chrono>
#include <string>

class MessageId {
public:
    std::size_t sequence_number;
    std::chrono::system_clock::time_point timestamp;
    
    std::string author_nickname;

    MessageId(std::size_t seq, std::chrono::system_clock::time_point ts, std::string nick)
        : sequence_number(seq), timestamp(ts), author_nickname(std::move(nick)) {}

    // auto operator<=>(const MessageId&) const = default;
    // bool operator==(const MessageId&) const = default;

    auto operator<=>(const MessageId& other) const {
        if (auto cmp = sequence_number <=> other.sequence_number; cmp != 0) {
            return cmp;
        }
        return timestamp <=> other.timestamp;
    }

    bool operator==(const MessageId& other) const {
        return sequence_number == other.sequence_number && timestamp == other.timestamp;
    }
};
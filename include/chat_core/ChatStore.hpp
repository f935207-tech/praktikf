#pragma once

#include "Message.hpp"

#include <map>
#include <vector>
#include <string>
#include <optional>
#include <functional>

class ChatStore {
private:
    std::map<std::string, std::vector<Message>> rooms_;

public:
    void post(const std::string& room, Message msg);

    std::optional<std::reference_wrapper<const std::vector<Message>>>
    history(const std::string& room) const;

    void create_room(const std::string& room);
    std::size_t room_count() const;

    std::optional<std::string> type_of(const std::string& room, std::size_t index) const;

    bool react(const std::string& room, std::size_t index);

    void for_each_room(
        const std::function<void(const std::string&, const std::vector<Message>&)>& visitor
    ) const;
};
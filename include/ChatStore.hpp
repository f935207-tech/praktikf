#pragma once

#include "Message.hpp"

#include <map>
#include <vector>
#include <string>
#include <functional>
#include <cstddef>

class ChatStore {
private:
    std::map<std::string, std::vector<Message> > rooms_;

public:
    void post(const std::string& room, Message msg);
    const std::vector<Message>* history(const std::string& room) const;
    void create_room(const std::string& room);
    std::size_t room_count() const;

    void for_each_room(
        std::function<void(const std::string&, const std::vector<Message>&)> visitor
    ) const;
};
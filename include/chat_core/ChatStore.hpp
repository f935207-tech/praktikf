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
    void create_room(const std::string& room);
    std::size_t room_count() const;
    
    void post(const std::string& room, Message msg);
    
    void post_many(const std::vector<std::string>& room_names, Message msg);

    std::optional<std::reference_wrapper<const std::vector<Message>>>
    history(const std::string& room) const;

    std::optional<std::string> type_of(const std::string& room, std::size_t index) const;
    
    void react(const std::string& room, std::size_t index);

    void for_each_room(
        const std::function<void(const std::string&, const std::vector<Message>&)>& visitor
    ) const;

    const std::map<std::string, std::vector<Message>>& get_rooms_data() const { return rooms_; }
};
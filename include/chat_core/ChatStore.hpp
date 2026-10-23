#pragma once
#include "Message.hpp"
#include "AppendOnlyLog.hpp"
#include <map>
#include <vector>
#include <string>
#include <memory>
#include <functional>

class User {
public:
    std::string name;
    explicit User(std::string n) : name(std::move(n)) {}
};

class ChatStore {
private:
    std::map<std::string, std::vector<Message>> rooms_;
    AppendOnlyLog log_;
    
    std::map<std::string, std::shared_ptr<User>> user_registry_;
    std::map<std::string, std::vector<std::weak_ptr<User>>> room_members_;

public:
    ChatStore(const std::string& log_filename = "chat.log");

    void create_room(const std::string& room);
    std::size_t room_count() const;
    void post(const std::string& room, Message msg);
    void post_many(const std::vector<std::string>& room_names, Message msg);
    std::optional<std::reference_wrapper<const std::vector<Message>>> history(const std::string& room) const;
    void react(const std::string& room, std::size_t index);
    void for_each_room(const std::function<void(const std::string&, const std::vector<Message>&)>& visitor) const;
    const std::map<std::string, std::vector<Message>>& get_rooms_data() const { return rooms_; }

    void register_user(const std::string& name);
    void join_room(const std::string& user, const std::string& room);
    void cleanup_expired_members(const std::string& room);
    std::size_t active_members_count(const std::string& room) const;
    void remove_user(const std::string& user);
};
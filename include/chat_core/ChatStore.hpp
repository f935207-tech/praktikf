#pragma once
#include "Message.hpp"
#include "AppendOnlyLog.hpp"
#include "MessageId.hpp"
#include "Concepts.hpp"
#include <map>
#include <vector>
#include <string>
#include <memory>
#include <functional>
#include <ranges>

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
    void post(const std::string& room, Message msg);
    void post_many(const std::vector<std::string>& room_names, Message msg);
    void react(const std::string& room, std::size_t index);
    const std::map<std::string, std::vector<Message>>& get_rooms_data() const { return rooms_; }

    template <NickLike T>
    void register_user(T&& name) {
        std::string str(name);
        if (user_registry_.find(str) == user_registry_.end()) {
            user_registry_[str] = std::make_shared<User>(str);
        }
    }

    template <NickLike T>
    void remove_user(T&& name) {
        std::string str(name);
        user_registry_.erase(str);
    }

    void join_room(const std::string& user, const std::string& room);
    void cleanup_expired_members(const std::string& room);
    void remove_user(const std::string& user); // Перегрузка для string

    std::vector<std::string> get_history_filtered(
        const std::string& room, 
        const std::string& author_filter = "", 
        std::size_t limit = 10);

    std::vector<std::string> get_members_sorted_by_length(const std::string& room);

    auto get_dangling_view() const;
};
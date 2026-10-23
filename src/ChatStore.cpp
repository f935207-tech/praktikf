#include "chat_core/ChatStore.hpp"
#include "chat_core/Errors.hpp"
#include <algorithm>
#include <iostream>
#include <ranges>

ChatStore::ChatStore(const std::string& log_filename) : log_(log_filename) {}

void ChatStore::create_room(const std::string& room) { rooms_[room]; }

void ChatStore::post(const std::string& room, Message msg) {
    if (rooms_.find(room) == rooms_.end()) throw UnknownRoomError(room);
    log_.write("[" + room + "] " + msg.author + ": " + msg.body->get_text());
    rooms_[room].push_back(std::move(msg));
}

void ChatStore::post_many(const std::vector<std::string>& room_names, Message msg) {
    for (const auto& r : room_names) {
        if (rooms_.find(r) == rooms_.end()) throw UnknownRoomError(r);
    }
    auto temp = rooms_;
    for (const auto& r : room_names) {
        Message copy = msg;
        temp[r].push_back(std::move(copy));
    }
    rooms_.swap(temp);
}

void ChatStore::react(const std::string& room, std::size_t index) {
    auto it = rooms_.find(room);
    if (it == rooms_.end()) throw UnknownRoomError(room);
    if (index >= it->second.size()) throw TypeMismatchError("Reaction", "Out of bounds");
    auto* r = dynamic_cast<ReactionMessage*>(it->second[index].body.get());
    if (!r) throw TypeMismatchError("Reaction", "Other type");
    r->increment();
}

void ChatStore::join_room(const std::string& user, const std::string& room) {
    create_room(room);
    auto it = user_registry_.find(user);
    if (it != user_registry_.end()) {
        room_members_[room].push_back(it->second);
    }
}

void ChatStore::cleanup_expired_members(const std::string& room) {
    auto& members = room_members_[room];
    std::erase_if(members, [](const std::weak_ptr<User>& wp) { return wp.expired(); });
}

void ChatStore::remove_user(const std::string& user) {
    user_registry_.erase(user);
}

std::vector<std::string> ChatStore::get_history_filtered(
    const std::string& room, const std::string& author_filter, std::size_t limit) {
    
    auto it = rooms_.find(room);
    if (it == rooms_.end()) return {};

    auto lazy_check = [](const Message& m, const std::string& author) {
        return author.empty() || m.author == author;
    };

    auto view = it->second 
        | std::views::filter([&](const Message& m) { return lazy_check(m, author_filter); })
        | std::views::transform([](const Message& m) { return m.body->get_text(); })
        | std::views::take(limit);

    std::vector<std::string> result;
    std::ranges::copy(view, std::back_inserter(result));
    return result;
}

std::vector<std::string> ChatStore::get_members_sorted_by_length(const std::string& room) {
    auto it = room_members_.find(room);
    if (it == room_members_.end()) return {};

    std::vector<std::string> names;
    for (const auto& wp : it->second) {
        if (auto sp = wp.lock()) {
            names.push_back(sp->name);
        }
    }

    std::ranges::sort(names, {}, [](const std::string& s) { return s.length(); });
    
    return names;
}

auto ChatStore::get_dangling_view() const {
    std::vector<Message> temp_room; // Временный вектор
    temp_room.emplace_back("user", std::make_unique<TextMessage>("test"));
    
    return temp_room | std::views::transform([](const Message& m) { return m.author; });
}
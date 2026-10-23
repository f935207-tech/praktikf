#include "chat_core/ChatStore.hpp"
#include "chat_core/Errors.hpp"
#include <algorithm>

ChatStore::ChatStore(const std::string& log_filename) : log_(log_filename) {}

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
        log_.write("[" + r + "] " + msg.author + ": " + msg.body->get_text());
    }
    rooms_.swap(temp);
}

void ChatStore::create_room(const std::string& room) { rooms_[room]; }
std::size_t ChatStore::room_count() const { return rooms_.size(); }

std::optional<std::reference_wrapper<const std::vector<Message>>>
ChatStore::history(const std::string& room) const {
    auto it = rooms_.find(room);
    if (it == rooms_.end()) return std::nullopt;
    return std::cref(it->second);
}

void ChatStore::react(const std::string& room, std::size_t index) {
    auto it = rooms_.find(room);
    if (it == rooms_.end()) throw UnknownRoomError(room);
    if (index >= it->second.size()) throw TypeMismatchError("Reaction", "Out of bounds");

    auto* r = dynamic_cast<ReactionMessage*>(it->second[index].body.get());
    if (!r) throw TypeMismatchError("Reaction", "Other type");
    r->increment();
}

void ChatStore::for_each_room(
    const std::function<void(const std::string&, const std::vector<Message>&)>& visitor) const {
    for (const auto& [room, messages] : rooms_) visitor(room, messages);
}

void ChatStore::register_user(const std::string& name) {
    if (user_registry_.find(name) == user_registry_.end()) {
        user_registry_[name] = std::make_shared<User>(name);
    }
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
    members.erase(
        std::remove_if(members.begin(), members.end(), 
            [](const std::weak_ptr<User>& wp) { return wp.expired(); }),
        members.end()
    );
}

std::size_t ChatStore::active_members_count(const std::string& room) const {
    auto it = room_members_.find(room);
    if (it == room_members_.end()) return 0;
    std::size_t count = 0;
    for (const auto& wp : it->second) {
        if (!wp.expired()) ++count;
    }
    return count;
}

void ChatStore::remove_user(const std::string& user) {
    user_registry_.erase(user);
}
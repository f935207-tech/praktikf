#include "chat_core/ChatStore.hpp"
#include "chat_core/Errors.hpp"
#include <algorithm>

void ChatStore::post(const std::string& room, Message msg) {
    if (rooms_.find(room) == rooms_.end()) throw UnknownRoomError(room);
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

void ChatStore::create_room(const std::string& room) { rooms_[room]; }
std::size_t ChatStore::room_count() const { return rooms_.size(); }

std::optional<std::reference_wrapper<const std::vector<Message>>>
ChatStore::history(const std::string& room) const {
    auto it = rooms_.find(room);
    if (it == rooms_.end()) return std::nullopt;
    return std::cref(it->second);
}

std::optional<std::string> ChatStore::type_of(const std::string& room, std::size_t index) const {
    auto it = rooms_.find(room);
    if (it == rooms_.end()) return std::nullopt;
    if (index >= it->second.size()) return std::nullopt;

    return std::visit([](const auto& body) -> std::string {
        using T = std::decay_t<decltype(body)>;
        if constexpr (std::is_same_v<T, TextBody>) return "TextBody";
        if constexpr (std::is_same_v<T, SystemNotice>) return "SystemNotice";
        if constexpr (std::is_same_v<T, Reaction>) return "Reaction";
        return "Unknown";
    }, it->second[index].body);
}

void ChatStore::react(const std::string& room, std::size_t index) {
    auto it = rooms_.find(room);
    if (it == rooms_.end()) throw UnknownRoomError(room);
    if (index >= it->second.size()) throw TypeMismatchError("Reaction", "Out of bounds");

    auto* r = std::get_if<Reaction>(&it->second[index].body);
    if (!r) throw TypeMismatchError("Reaction", "Other type");
    
    ++r->count;
}

void ChatStore::for_each_room(
    const std::function<void(const std::string&, const std::vector<Message>&)>& visitor) const {
    for (const auto& [room, messages] : rooms_) visitor(room, messages);
}
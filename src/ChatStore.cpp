#include "chat_core/ChatStore.hpp"

void ChatStore::post(const std::string& room, Message msg) {
    rooms_[room].push_back(std::move(msg));
}

const std::vector<Message>* ChatStore::history(const std::string& room) const {
    std::map<std::string, std::vector<Message> >::const_iterator it = rooms_.find(room);
    if (it == rooms_.end()) {
        return nullptr;
    }
    return &it->second;
}

void ChatStore::create_room(const std::string& room) {
    // Просто обращаемся по ключу — если нет, создастся пустой вектор.
    rooms_[room];
}

std::size_t ChatStore::room_count() const {
    return rooms_.size();
}

void ChatStore::for_each_room(
    std::function<void(const std::string&, const std::vector<Message>&)> visitor
) const {
    // Structured bindings тут недоступны — используем .first / .second.
    for (const auto& pair : rooms_) {
        visitor(pair.first, pair.second);
    }
}
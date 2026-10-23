#include "chat_core/ChatStore.hpp"
#include "chat_core/Overloaded.hpp"

#include <algorithm>
#include <type_traits>

void ChatStore::post(const std::string& room, Message msg) {
    rooms_[room].push_back(std::move(msg));
}

std::optional<std::reference_wrapper<const std::vector<Message>>>
ChatStore::history(const std::string& room) const {
    auto it = rooms_.find(room);
    if (it == rooms_.end()) {
        return std::nullopt;
    }
    return std::cref(it->second);
}

void ChatStore::create_room(const std::string& room) {
    rooms_[room];
}

std::size_t ChatStore::room_count() const {
    return rooms_.size();
}

std::optional<std::string> ChatStore::type_of(const std::string& room,
                                              std::size_t index) const {
    auto it = rooms_.find(room);
    if (it == rooms_.end()) {
        return std::nullopt;
    }
    const auto& msgs = it->second;
    if (index >= msgs.size()) {
        return std::nullopt;
    }

    return std::visit(Overloaded{
        [](const TextBody&)     -> std::string { return "TextBody"; },
        [](const SystemNotice&) -> std::string { return "SystemNotice"; },
        [](const Reaction&)     -> std::string { return "Reaction"; }
    }, msgs[index].body);
}

bool ChatStore::react(const std::string& room, std::size_t index) {
    auto it = rooms_.find(room);
    if (it == rooms_.end()) {
        return false;
    }
    auto& msgs = it->second;
    if (index >= msgs.size()) {
        return false;
    }

    if (auto* r = std::get_if<Reaction>(&msgs[index].body)) {
        ++r->count;
        return true;
    }
    return false;
}

void ChatStore::for_each_room(
    const std::function<void(const std::string&, const std::vector<Message>&)>& visitor
) const {
    for (const auto& [room, messages] : rooms_) {
        visitor(room, messages);
    }
}
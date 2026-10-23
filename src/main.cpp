#include "chat_core/ChatStore.hpp"
#include "chat_core/Lexer.hpp"
#include "chat_core/Command.hpp"
#include "chat_core/Overloaded.hpp"
#include "chat_core/Errors.hpp"
#include <iostream>

void execute(const Command& cmd, ChatStore& store) {
    std::visit(Overloaded{
        [&](const JoinCommand& c) { store.create_room(c.room); std::cout << "joined\n"; },
        [&](const LeaveCommand& c) { std::cout << "left\n"; },
        [&](const MsgCommand& c) {
            auto msg = Message("me", std::make_unique<TextMessage>(c.text));
            store.post_many(c.rooms, std::move(msg));
            std::cout << "sent\n";
        },
        [&](const WhoCommand& c) {
            auto h = store.history(c.room);
            if (!h) throw UnknownRoomError(c.room);
            std::cout << "participants: " << h->get().size() << "\n";
        },
        [&](const HistoryCommand& c) {
            if (auto h = store.history(c.room); h) {
                for (const auto& m : h->get()) {
                    std::cout << "msg from " << m.author << ": " << m.body->get_text() << "\n";
                }
            } else throw UnknownRoomError(c.room);
        },
        [&](const TypeCommand& c) {
            auto h = store.history(c.room);
            if (!h) throw UnknownRoomError(c.room);
            if (c.index >= h->get().size()) throw TypeMismatchError("Index", "Out of bounds");
            std::cout << "type: " << h->get()[c.index].body->get_type() << "\n";
        },
        [&](const ReactCommand& c) {
            store.react(c.room, c.index);
            std::cout << "reacted\n";
        }
    }, cmd);
}

int main() {
    MessageId id1(1, std::chrono::system_clock::now(), "alice");
    MessageId id2(1, std::chrono::system_clock::now(), "bob");
    MessageId id3(2, std::chrono::system_clock::now(), "charlie");

    std::cout << "id1 < id2: " << (id1 < id2) << "\n";
    std::cout << "id2 < id3: " << (id2 < id3) << "\n";
    std::cout << "id1 == id2: " << (id1 == id2) << "\n";

    ChatStore store("chat.log");
    store.register_user("alice");
    std::string bob_str = "bob";
    store.register_user(bob_str);
    std::string_view charlie_view = "charlie";
    store.register_user(charlie_view);

    std::cout << "=== Chat v05 ===\n";
    std::string line;
    while (std::getline(std::cin, line)) {
        if (line == "/exit") break;
        try {
            auto tokens = Lexer::lex(line);
            auto cmd = parse_command_expected(tokens);
            if (!cmd) throw cmd.error();
        } catch (const ChatError& e) {
            std::cerr << "Error: " << e.what() << "\n";
        }
    }
    return 0;
}
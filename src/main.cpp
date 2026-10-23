#include "chat_core/ChatStore.hpp"
#include "chat_core/Lexer.hpp"
#include "chat_core/Command.hpp"
#include "chat_core/Overloaded.hpp"
#include "chat_core/Errors.hpp"
#include <iostream>
#include <string>

void execute(const Command& cmd, ChatStore& store) {
    std::visit(Overloaded{
        [&](const JoinCommand& c) { store.create_room(c.room); std::cout << "joined\n"; },
        [&](const LeaveCommand&) { std::cout << "left\n"; },
        [&](const MsgCommand& c) {
            store.post_many(c.rooms, Message("me", c.text));
            std::cout << "sent\n";
        },
        [&](const WhoCommand& c) {
            auto h = store.history(c.room);
            if (!h) throw UnknownRoomError(c.room);
            std::cout << "participants: " << h->get().size() << "\n";
        },
        [&](const HistoryCommand& c) {
            if (auto h = store.history(c.room); h) {
                for (const auto& m : h->get()) std::cout << "msg from " << m.author << "\n";
            } else throw UnknownRoomError(c.room);
        },
        [&](const TypeCommand& c) {
            auto t = store.type_of(c.room, c.index);
            if (t) std::cout << "type: " << *t << "\n";
            else throw UnknownRoomError(c.room);
        },
        [&](const ReactCommand& c) {
            store.react(c.room, c.index);
            std::cout << "reacted\n";
        }
    }, cmd);
}

int main() {
    ChatStore store;
    store.create_room("general");
    store.create_room("random");

    std::cout << "=== Chat v03 ===\n";
    std::string line;
    while (std::getline(std::cin, line)) {
        if (line == "/exit") break;

        try {
            auto tokens = Lexer::lex(line);
            auto cmd = parse_command_expected(tokens);
            if (!cmd) {
                throw cmd.error();
            }
            execute(cmd.value(), store);
        } catch (const ChatError& e) {
            std::cerr << "Error: " << e.what() << "\n";
        } catch (const std::exception& e) {
            std::cerr << "System error: " << e.what() << "\n";
        }
    }
    return 0;
}
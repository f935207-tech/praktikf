#include "chat_core/ChatStore.hpp"
#include "chat_core/Lexer.hpp"
#include "chat_core/Command.hpp"
#include "chat_core/Overloaded.hpp"

#include <iostream>
#include <string>

namespace {

void execute(const Command& cmd, ChatStore& store) {
    std::visit(Overloaded{
        [&](const JoinCommand& c) {
            store.create_room(c.room);
            std::cout << "joined " << c.room << "\n";
        },
        [&](const LeaveCommand& c) {
            std::cout << "left " << c.room << "\n";
        },
        [&](const MsgCommand& c) {
            store.post(c.room, Message("me", c.text));
            std::cout << "sent\n";
        },
        [&](const WhoCommand& c) {
            auto h = store.history(c.room);
            if (!h) {
                std::cout << "room not found\n";
                return;
            }
            std::cout << "participants in " << c.room << ":\n";
            for (const auto& m : h->get()) {
                std::cout << "  " << m.author << "\n";
            }
        },
        [&](const HistoryCommand& c) {
            if (auto h = store.history(c.room); h) {
                for (const auto& m : h->get()) {
                    std::visit(Overloaded{
                        [](const TextBody& b)     { std::cout << "  " << b.text << "\n"; },
                        [](const SystemNotice& n) { std::cout << "  * " << n.text << "\n"; },
                        [](const Reaction& r)     { std::cout << "  [reaction x" << r.count << "]\n"; }
                    }, m.body);
                }
            } else {
                std::cout << "room not found\n";
            }
        },
        [&](const TypeCommand& c) {
            auto t = store.type_of(c.room, c.index);
            if (t) {
                std::cout << "type: " << *t << "\n";
            } else {
                std::cout << "no such message\n";
            }
        },
        [&](const ReactCommand& c) {
            if (store.react(c.room, c.index)) {
                std::cout << "reacted\n";
            } else {
                std::cout << "cannot react: not a Reaction or no such message\n";
            }
        }
    }, cmd);
}

} // namespace

int main() {
    ChatStore store;
    store.create_room("general");
    store.create_room("random");

    std::cout << "=== for_each_room ===\n";
    int by_ref = 0;
    store.for_each_room([&by_ref](const std::string& name, const std::vector<Message>& msgs) {
        by_ref += static_cast<int>(msgs.size());
        std::cout << "  [ref] " << name << " -> " << msgs.size() << "\n";
    });

    std::cout << "=== chat_cli (v02) ===\n";
    std::cout << "Команды: /join /leave /msg /who /history /type /react\n";
    std::cout << "Выход: /exit\n";

    std::string line;
    while (std::getline(std::cin, line)) {
        if (line == "/exit") {
            break;
        }

        auto tokens = Lexer::lex(line);
        auto cmd = parse_command(tokens);

        if (!cmd) {
            std::cout << "unknown or malformed command\n";
            continue;
        }
        execute(*cmd, store);
    }

    return 0;
}
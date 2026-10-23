#include "chat_core/ChatStore.hpp"
#include "chat_core/Lexer.hpp"
#include "chat_core/Errors.hpp"
#include "chat_core/CommandRegistry.hpp"
#include <iostream>

int main() {
    ChatStore store("chat.log");
    store.create_room("general");
    store.create_room("random");
    
    store.register_user("alice");
    store.register_user("bob");

    CommandRegistry registry;

    registry.register_command("JOIN", [](ChatStore& s, RoomName room) -> int {
        s.create_room(room.name);
        std::cout << "joined " << room.name << "\n";
        return 0;
    });

    registry.register_command("LEAVE", [](ChatStore& s, std::string_view room) -> int {
        std::cout << "left " << room << "\n";
        return 0;
    });

    registry.register_command("MSG", [](ChatStore& s, RoomName room, std::string_view text) -> int {
        s.post(room.name, Message("me", std::make_unique<TextMessage>(std::string(text))));
        std::cout << "sent to " << room.name << "\n";
        return 0;
    });

    registry.register_command("WHO", [](ChatStore& s, std::string_view room) -> int {
        std::cout << "participants in " << room << ": ";
        auto members = s.get_members_sorted_by_length(std::string(room));
        for (const auto& m : members) std::cout << m << " ";
        std::cout << "\n";
        return 0;
    });

    registry.register_command("HISTORY", [](ChatStore& s, std::string_view room, std::size_t limit) -> int {
        auto history = s.get_history_filtered(std::string(room), "", limit);
        for (const auto& msg : history) std::cout << "  " << msg << "\n";
        return 0;
    });

    registry.register_command("TYPE", [](ChatStore& s, std::string_view room, std::size_t index) -> int {
        auto h = s.history(std::string(room));
        std::cout << "type: " << "TextBody" << "\n"; 
        return 0;
    });

    registry.register_command("REACT", [](ChatStore& s, std::string_view room, std::size_t index) -> int {
        s.react(std::string(room), index);
        std::cout << "reacted\n";
        return 0;
    });

    registry.register_command("TOPIC", [](ChatStore& s, RoomName room, std::string_view topic) -> int {
        std::cout << "Topic for #" << room.name << " set to: " << topic << "\n";
        return 0;
    });

    // ДОМАШНЕЕ ЗАДАНИЕ:
    registry.register_command("NICK", [](ChatStore& s, std::string_view old_nick, std::string_view new_nick) -> int {
        s.remove_user(std::string(old_nick));
        s.register_user(std::string(new_nick));
        std::cout << "nick changed\n";
        return 0;
    });

    registry.register_command("KICK", [](ChatStore& s, std::string_view room, std::string_view user) -> int {
        s.remove_user(std::string(user));
        std::cout << user << " kicked from " << room << "\n";
        return 0;
    });

    registry.register_command("ROOMS", [](ChatStore& s) -> int {
        std::cout << "active rooms:\n";
        for (const auto& pair : s.get_rooms_data()) {
            std::cout << "  " << pair.first << "\n";
        }
        return 0;
    });

    std::cout << "=== Chat v06 ===\n";
    std::string line;
    while (std::getline(std::cin, line)) {
        if (line == "/exit") break;
        try {
            auto tokens = Lexer::lex(line);
            if (tokens.empty()) continue;

            std::string cmd_name(tokens[0]);
            if (!cmd_name.empty() && cmd_name[0] == '/') {
                cmd_name = cmd_name.substr(1); // Убираем слэш
            }
            
            std::vector<std::string_view> args(tokens.begin() + 1, tokens.end());
            registry.execute(cmd_name, args, store);
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << "\n";
        }
    }
    return 0;
}
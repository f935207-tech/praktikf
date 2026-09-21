#include "chat_core/ChatStore.hpp"

#include <iostream>
#include <string>
#include <cstddef>

// Проверка префикса строки: возвращает true, если line начинается с prefix.
// В C++14 нет std::string::starts_with — пишем сами.
static bool starts_with(const std::string& line, const std::string& prefix) {
    if (line.size() < prefix.size()) {
        return false;
    }
    return line.compare(0, prefix.size(), prefix) == 0;
}

int main() {
    ChatStore store;
    store.create_room("general");
    store.create_room("random");

    // Демонстрация разницы между захватом по ссылке и по значению.
    std::cout << "=== Проверка for_each_room ===\n";

    int by_ref = 0;

    // Захват по ссылке — счётчик снаружи реально меняется.
    store.for_each_room([&by_ref](const std::string& name, const std::vector<Message>& msgs) {
        by_ref += static_cast<int>(msgs.size());
        std::cout << "  [ref]  room=" << name
                  << " messages=" << msgs.size() << "\n";
    });
    std::cout << "by_ref после обхода: " << by_ref << "\n\n";

    int by_val = 0;

    // Захват по значению + mutable — меняется только копия внутри лямбды.
    store.for_each_room([by_val](const std::string& name, const std::vector<Message>& msgs) mutable {
        by_val += static_cast<int>(msgs.size());
        std::cout << "  [val]  room=" << name
                  << " local_counter=" << by_val << "\n";
    });
    std::cout << "by_val после обхода (снаружи): " << by_val << "\n\n";

    // Небольшой интерактив.
    std::cout << "=== chat_cli ===\n";
    std::cout << "Команды:\n"
              << "  /post <room> <author> <text>\n"
              << "  /history <room>\n"
              << "  /exit\n";

    std::string line;
    while (std::getline(std::cin, line)) {
        if (line == "/exit") {
            break;
        }

        if (starts_with(line, "/post ")) {
            // Разбираем строку: /post room author text
            std::size_t p1 = line.find(' ', 6);
            if (p1 == std::string::npos) {
                std::cout << "Не хватает аргументов\n";
                continue;
            }
            std::string room = line.substr(6, p1 - 6);

            std::size_t p2 = line.find(' ', p1 + 1);
            if (p2 == std::string::npos) {
                std::cout << "Не хватает аргументов\n";
                continue;
            }
            std::string author = line.substr(p1 + 1, p2 - p1 - 1);
            std::string text = line.substr(p2 + 1);

            store.post(room, Message(author, text));
            std::cout << "OK\n";
        }
        else if (starts_with(line, "/history ")) {
            std::string room = line.substr(9);
            const std::vector<Message>* h = store.history(room);
            if (h == nullptr) {
                std::cout << "Комнаты нет\n";
                continue;
            }
            for (const Message& m : *h) {
                std::cout << m.author << ": " << m.text << "\n";
            }
        }
        else {
            std::cout << "Неизвестная команда\n";
        }
    }

    return 0;
}
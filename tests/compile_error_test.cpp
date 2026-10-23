#include "chat_core/CommandRegistry.hpp"
#include "chat_core/ChatStore.hpp"

int main() {
    CommandRegistry registry;
    ChatStore store;

    auto bad_handler = [](ChatStore& s, int x) { };
    
    return 0;
}
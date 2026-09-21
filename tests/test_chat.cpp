#include <gtest/gtest.h>

#include "chat_core/Message.hpp"
#include "chat_core/OwningBuffer.hpp"
#include "chat_core/ChatStore.hpp"

static int g_copies = 0;
static int g_moves  = 0;

struct SpyMessage {
    Message msg;

    SpyMessage(std::string a, std::string t)
        : msg(std::move(a), std::move(t)) {}

    SpyMessage(const SpyMessage& other)
        : msg(other.msg) {
        ++g_copies;
    }

    SpyMessage(SpyMessage&& other) noexcept
        : msg(std::move(other.msg)) {
        ++g_moves;
    }

    SpyMessage& operator=(const SpyMessage& other) {
        if (this != &other) {
            msg = other.msg;
            ++g_copies;
        }
        return *this;
    }

    SpyMessage& operator=(SpyMessage&& other) noexcept {
        if (this != &other) {
            msg = std::move(other.msg);
            ++g_moves;
        }
        return *this;
    }
};

// Тесты Message

TEST(MessageTest, MoveDoesNotCopy) {
    g_copies = 0;
    g_moves  = 0;

    SpyMessage a("Alice", "hello");
    SpyMessage b = std::move(a);

    EXPECT_EQ(g_copies, 0);
    EXPECT_EQ(g_moves, 1);
    EXPECT_EQ(b.msg.author, "Alice");
    EXPECT_EQ(b.msg.text, "hello");
}

// Тесты ChatStore

TEST(ChatStoreTest, PostTemporaryMessageDoesNotCopy) {
    g_copies = 0;
    g_moves  = 0;

    ChatStore store;
    store.create_room("test");

    store.post("test", Message("Bob", "hi"));

    EXPECT_EQ(g_copies, 0);
}

TEST(ChatStoreTest, HistoryReturnsNullptrForUnknownRoom) {
    ChatStore store;
    EXPECT_EQ(store.history("nope"), nullptr);
}

TEST(ChatStoreTest, HistoryWorks) {
    ChatStore store;
    store.create_room("r");
    store.post("r", Message("A", "1"));
    store.post("r", Message("B", "2"));

    const std::vector<Message>* h = store.history("r");
    ASSERT_NE(h, nullptr);
    EXPECT_EQ(h->size(), 2u);
    EXPECT_EQ((*h)[0].author, "A");
    EXPECT_EQ((*h)[1].text, "2");
}

// Тесты OwningBuffer 

TEST(OwningBufferTest, SelfAssignmentIsSafe) {
    OwningBuffer buf(16);
    buf.set(0, 'X');
    buf.set(1, 'Y');

    buf = buf;

    EXPECT_EQ(buf.size(), 16u);
    EXPECT_EQ(buf.get(0), 'X');
    EXPECT_EQ(buf.get(1), 'Y');
}

TEST(OwningBufferTest, MoveTransfersOwnership) {
    OwningBuffer a(10);
    OwningBuffer b(4);

    b = std::move(a);

    EXPECT_EQ(b.size(), 10u);
    EXPECT_EQ(a.size(), 0u);
    EXPECT_EQ(a.data(), nullptr);
}

// ДОМАШНЕЕ ЗАДАНИЕ

// Тест 1
TEST(OwningBufferTest, CopyHasIndependentMemory) {
    OwningBuffer original(8);

    for (std::size_t i = 0; i < original.size(); ++i) {
        original.set(i, static_cast<char>('A' + i));
    }

    OwningBuffer copy = original;

    EXPECT_NE(original.data(), copy.data());
    EXPECT_EQ(original.size(), copy.size());

    copy.set(0, 'Z');
    copy.set(1, 'Z');

    EXPECT_EQ(original.get(0), 'A');
    EXPECT_EQ(original.get(1), 'B');
    EXPECT_EQ(copy.get(0), 'Z');
    EXPECT_EQ(copy.get(1), 'Z');

    original.set(2, '!');
    EXPECT_EQ(copy.get(2), 'C');
}

// Тест 1б
TEST(OwningBufferTest, CopyAssignmentHasIndependentMemory) {
    OwningBuffer a(4);
    a.set(0, '1');
    a.set(1, '2');

    OwningBuffer b(4);
    b.set(0, 'x');
    b.set(1, 'y');

    b = a;

    EXPECT_NE(a.data(), b.data());
    EXPECT_EQ(a.get(0), b.get(0));
    EXPECT_EQ(a.get(1), b.get(1));

    b.set(0, '@');
    EXPECT_EQ(a.get(0), '1');
    EXPECT_EQ(b.get(0), '@');
}

// Тест 2
TEST(OwningBufferTest, MovedFromObjectIsValid) {
    OwningBuffer src(10);
    for (std::size_t i = 0; i < src.size(); ++i) {
        src.set(i, 'a');
    }

    OwningBuffer dst = std::move(src);

    EXPECT_EQ(src.size(), 0u);         
    EXPECT_EQ(src.data(), nullptr);    

    src.set(0, 'q');                   
    EXPECT_EQ(src.get(0), '\0');

    OwningBuffer other(3);
    other.set(0, 'k');
    src = std::move(other);
    EXPECT_EQ(src.size(), 3u);
    EXPECT_EQ(src.get(0), 'k');
    EXPECT_EQ(other.size(), 0u);
    EXPECT_EQ(other.data(), nullptr);

}
#include <gtest/gtest.h>
#include "chat_core/Message.hpp"
#include "chat_core/OwningBuffer.hpp"
#include "chat_core/ChatStore.hpp"
#include "chat_core/Lexer.hpp"
#include "chat_core/Command.hpp"
#include "chat_core/Errors.hpp"
#include <stdexcept>
#include <random>

// ЭТАП 1

static int g_copies = 0;
static int g_moves  = 0;

struct SpyMessage {
    Message msg;
    SpyMessage(std::string a, std::string t)
        : msg(std::move(a), std::move(t)) {}
    SpyMessage(const SpyMessage& other) : msg(other.msg) { ++g_copies; }
    SpyMessage(SpyMessage&& other) noexcept : msg(std::move(other.msg)) { ++g_moves; }
    SpyMessage& operator=(const SpyMessage& other) {
        if (this != &other) { msg = other.msg; ++g_copies; }
        return *this;
    }
    SpyMessage& operator=(SpyMessage&& other) noexcept {
        if (this != &other) { msg = std::move(other.msg); ++g_moves; }
        return *this;
    }
};

TEST(MessageTest, MoveDoesNotCopy) {
    auto m1 = Message("Alice", std::make_unique<TextMessage>("hello"));
    Message m2 = std::move(m1); // Перемещение
    EXPECT_EQ(m2.author, "Alice");
    EXPECT_EQ(m2.body->get_text(), "hello");
}

TEST(OwningBufferTest, SelfAssignmentIsSafe) {
    OwningBuffer buf(16);
    buf.set(0, 'X');
    buf = buf;
    EXPECT_EQ(buf.size(), 16u);
    EXPECT_EQ(buf.get(0), 'X');
}

TEST(OwningBufferTest, MoveTransfersOwnership) {
    OwningBuffer a(10);
    OwningBuffer b(4);
    b = std::move(a);
    EXPECT_EQ(b.size(), 10u);
    EXPECT_EQ(a.size(), 0u);
    EXPECT_EQ(a.data(), nullptr);
}

TEST(OwningBufferTest, CopyHasIndependentMemory) {
    OwningBuffer original(8);
    for (std::size_t i = 0; i < original.size(); ++i) {
        original.set(i, static_cast<char>('A' + i));
    }
    OwningBuffer copy = original;
    EXPECT_NE(original.data(), copy.data());
    copy.set(0, 'Z');
    EXPECT_EQ(original.get(0), 'A');
    EXPECT_EQ(copy.get(0), 'Z');
}

// ЭТАП 2

TEST(LexerTest, SimpleTokens) {
    std::string s = "/msg alice hello";
    auto tokens = Lexer::lex(s);
    ASSERT_EQ(tokens.size(), 3u);
    EXPECT_EQ(tokens[0], "/msg");
    EXPECT_EQ(tokens[1], "alice");
    EXPECT_EQ(tokens[2], "hello");
}

TEST(LexerTest, ExtraWhitespaceAndEmptyInput) {
    std::string s = "   /join    general   ";
    auto tokens = Lexer::lex(s);
    ASSERT_EQ(tokens.size(), 2u);
    EXPECT_EQ(tokens[0], "/join");
    EXPECT_EQ(tokens[1], "general");

    std::string empty = "";
    EXPECT_TRUE(Lexer::lex(empty).empty());

    std::string spaces = "    \t\n ";
    EXPECT_TRUE(Lexer::lex(spaces).empty());
}

TEST(LexerTest, QuotedTokenIsSingleToken) {
    std::string s = "/topic #general \"обсуждаем курс\"";
    auto tokens = Lexer::lex(s);
    ASSERT_EQ(tokens.size(), 3u);
    EXPECT_EQ(tokens[0], "/topic");
    EXPECT_EQ(tokens[1], "#general");
    EXPECT_EQ(tokens[2], "обсуждаем курс");
}

TEST(LexerTest, NoStringCopiesInsideLexer) {

    std::string s = "/join general";
    auto tokens = Lexer::lex(s);
    ASSERT_EQ(tokens.size(), 2u);
    EXPECT_GE(tokens[0].data(), s.data());
    EXPECT_LE(tokens[0].data() + tokens[0].size(), s.data() + s.size());
    EXPECT_GE(tokens[1].data(), s.data());
    EXPECT_LE(tokens[1].data() + tokens[1].size(), s.data() + s.size());
}

TEST(ParserTest, MsgTailIsWholeRemainder) {
    std::string s = "/msg #alice hello world, how are you?";
    auto cmd = parse_command(Lexer::lex(s));
    ASSERT_TRUE(cmd.has_value());
    ASSERT_TRUE(std::holds_alternative<MsgCommand>(*cmd));
    const auto& m = std::get<MsgCommand>(*cmd);
    ASSERT_FALSE(m.rooms.empty());
    EXPECT_EQ(m.rooms[0], "alice");
    EXPECT_EQ(m.text, "hello world, how are you?");
}

TEST(ParserTest, JoinAndHistory) {
    std::string s1 = "/join general";
    auto c1 = parse_command(Lexer::lex(s1));
    ASSERT_TRUE(c1.has_value());
    EXPECT_TRUE(std::holds_alternative<JoinCommand>(*c1));

    std::string s2 = "/history general";
    auto c2 = parse_command(Lexer::lex(s2));
    ASSERT_TRUE(c2.has_value());
    EXPECT_TRUE(std::holds_alternative<HistoryCommand>(*c2));
}

TEST(ParserTest, EmptyAndMalformed) {
    std::string empty = "";
    EXPECT_FALSE(parse_command(Lexer::lex(empty)).has_value());

    std::string unknown = "/unknown abc";
    EXPECT_FALSE(parse_command(Lexer::lex(unknown)).has_value());

    std::string tooFew = "/join";
    EXPECT_FALSE(parse_command(Lexer::lex(tooFew)).has_value());

    std::string badIndex = "/type general abc";
    EXPECT_FALSE(parse_command(Lexer::lex(badIndex)).has_value());
}

TEST(ChatStoreTest, HistoryUsesOptional) {
    ChatStore store;
    EXPECT_FALSE(store.history("nope").has_value());

    store.create_room("r");
    store.post("r", Message("A", "hello"));
    auto h = store.history("r");
    ASSERT_TRUE(h.has_value());
    EXPECT_EQ(h->get().size(), 1u);
}

TEST(ChatStoreTest, TypeOfReturnsVariantName) {
    ChatStore store;
    store.create_room("r");
    store.post("r", Message("A", "hello"));

    auto t = store.type_of("r", 0);
    ASSERT_TRUE(t.has_value());
    EXPECT_EQ(*t, "TextBody");

    EXPECT_FALSE(store.type_of("r", 5).has_value()); 
    EXPECT_FALSE(store.type_of("nope", 0).has_value()); 
}

TEST(ChatStoreTest, ReactOnlyWorksOnReaction) {
    ChatStore store;
    store.create_room("r");
    store.post("r", Message("A", "text"));      
    store.post("r", Message("sys", SystemNotice{"joined"})); 

    EXPECT_THROW(store.react("r", 0), TypeMismatchError);
    EXPECT_THROW(store.react("r", 1), TypeMismatchError);
    EXPECT_THROW(store.react("r", 10), TypeMismatchError);

    store.post("r", Message("B", Reaction{0}));
    EXPECT_NO_THROW(store.react("r", 2));
    EXPECT_NO_THROW(store.react("r", 2)); 
    auto t = store.type_of("r", 2);
    ASSERT_TRUE(t.has_value());
    EXPECT_EQ(*t, "Reaction");
}

/*==50116==ERROR: AddressSanitizer: stack-use-after-return on address 0x7a776ce994d0 at pc 0x7e776f6ed445 bp 0x7ffd92744280 sp 0x7ffd92743a28
READ of size 5 at 0x7a776ce994d0 thread T0
    #0 0x7e776f6ed444 in MemcmpInterceptorCommon(void*, int (*)(void const*, void const*, unsigned long), void const*, void const*, unsigned long) ../../../../src/libsanitizer/sanitizer_common/sanitizer_common_interceptors.inc:848
    #1 0x7e776f6ed834 in memcmp ../../../../src/libsanitizer/sanitizer_common/sanitizer_common_interceptors.inc:880
    #2 0x7e776f6ed834 in memcmp ../../../../src/libsanitizer/sanitizer_common/sanitizer_common_interceptors.inc:875
    #3 0x629bfe8c2c1c in std::char_traits<char>::compare(char const*, char const*, unsigned long) /usr/include/c++/15/bits/char_traits.h:383
    #4 0x629bfe8d2a5a in std::basic_string_view<char, std::char_traits<char> >::compare(std::basic_string_view<char, std::char_traits<char> >) const /usr/include/c++/15/string_view:350
    #5 0x629bfe8cfaba in bool std::operator==<char, std::char_traits<char> >(std::basic_string_view<char, std::char_traits<char> >, std::__type_identity<std::basic_string_view<char, std::char_traits<char> > >::type) /usr/include/c++/15/string_view:636
    #6 0x629bfe8cbcfe in testing::AssertionResult testing::internal::CmpHelperEQ<std::basic_string_view<char, std::char_traits<char> >, char [6]>(char const*, char const*, std::basic_string_view<char, std::char_traits<char> > const&, char const (&) [6]) /home/vboxuser/Desktop/cpp2/build-asan/_deps/googletest-src/googletest/include/gtest/gtest.h:1358
    #7 0x629bfe8c7685 in testing::AssertionResult testing::internal::EqHelper::Compare<std::basic_string_view<char, std::char_traits<char> >, char [6], (void*)0>(char const*, char const*, std::basic_string_view<char, std::char_traits<char> > const&, char const (&) [6]) /home/vboxuser/Desktop/cpp2/build-asan/_deps/googletest-src/googletest/include/gtest/gtest.h:1377
    #8 0x629bfe8bf6fc in DanglingStringViewTest_ShowsBug_Test::TestBody() /home/vboxuser/Desktop/cpp2/tests/test_chat.cpp:200
    #9 0x629bfe979955 in void testing::internal::HandleSehExceptionsInMethodIfSupported<testing::Test, void>(testing::Test*, void (testing::Test::*)(), char const*) /home/vboxuser/Desktop/cpp2/build-asan/_deps/googletest-src/googletest/src/gtest.cc:2599
    #10 0x629bfe965dfd in void testing::internal::HandleExceptionsInMethodIfSupported<testing::Test, void>(testing::Test*, void (testing::Test::*)(), char const*) /home/vboxuser/Desktop/cpp2/build-asan/_deps/googletest-src/googletest/src/gtest.cc:2635
    #11 0x629bfe915af0 in testing::Test::Run() /home/vboxuser/Desktop/cpp2/build-asan/_deps/googletest-src/googletest/src/gtest.cc:2674
    #12 0x629bfe9171b7 in testing::TestInfo::Run() /home/vboxuser/Desktop/cpp2/build-asan/_deps/googletest-src/googletest/src/gtest.cc:2853
    #13 0x629bfe918447 in testing::TestSuite::Run() /home/vboxuser/Desktop/cpp2/build-asan/_deps/googletest-src/googletest/src/gtest.cc:3012
    #14 0x629bfe93ad1e in testing::internal::UnitTestImpl::RunAllTests() /home/vboxuser/Desktop/cpp2/build-asan/_deps/googletest-src/googletest/src/gtest.cc:5870
    #15 0x629bfe97c58c in bool testing::internal::HandleSehExceptionsInMethodIfSupported<testing::internal::UnitTestImpl, bool>(testing::internal::UnitTestImpl*, bool (testing::internal::UnitTestImpl::*)(), char const*) /home/vboxuser/Desktop/cpp2/build-asan/_deps/googletest-src/googletest/src/gtest.cc:2599
    #16 0x629bfe96876d in bool testing::internal::HandleExceptionsInMethodIfSupported<testing::internal::UnitTestImpl, bool>(testing::internal::UnitTestImpl*, bool (testing::internal::UnitTestImpl::*)(), char const*) /home/vboxuser/Desktop/cpp2/build-asan/_deps/googletest-src/googletest/src/gtest.cc:2635
    #17 0x629bfe93742f in testing::UnitTest::Run() /home/vboxuser/Desktop/cpp2/build-asan/_deps/googletest-src/googletest/src/gtest.cc:5444
    #18 0x629bfe8ea75f in RUN_ALL_TESTS() /home/vboxuser/Desktop/cpp2/build-asan/_deps/googletest-src/googletest/include/gtest/gtest.h:2293
    #19 0x629bfe8ea6ab in main /home/vboxuser/Desktop/cpp2/build-asan/_deps/googletest-src/googletest/src/gtest_main.cc:51
    #20 0x7e776ee2a600 in __libc_start_call_main ../sysdeps/nptl/libc_start_call_main.h:59
    #21 0x7e776ee2a717 in __libc_start_main_impl ../csu/libc-start.c:360
    #22 0x629bfe8af7c4 in _start (/home/vboxuser/Desktop/cpp2/build-asan/chat_tests+0x1f7c4) (BuildId: 3bf8e12d43ace30feb430c0ae6073e1315f6ec7f)

Address 0x7a776ce994d0 is located in stack of thread T0 at offset 80 in frame
    #0 0x629bfe8bf177 in operator() /home/vboxuser/Desktop/cpp2/tests/test_chat.cpp:192

  This frame has 2 object(s):
    [48, 49) '<unknown>'
    [64, 96) 'temp' (line 193) <== Memory access at offset 80 is inside this variable
HINT: this may be a false positive if your program uses some custom stack unwind mechanism, swapcontext or vfork
      (longjmp and C++ exceptions *are* supported)
SUMMARY: AddressSanitizer: stack-use-after-return /home/vboxuser/Desktop/cpp2/build-asan/_deps/googletest-src/googletest/include/gtest/gtest.h:1358 in testing::AssertionResult testing::internal::CmpHelperEQ<std::basic_string_view<char, std::char_traits<char> >, char [6]>(char const*, char const*, std::basic_string_view<char, std::char_traits<char> > const&, char const (&) [6])
Shadow bytes around the buggy address:
  0x7a776ce99200: f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f3 f3 f3 f3
  0x7a776ce99280: f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f3 f3 f3 f3
  0x7a776ce99300: f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f3 f3 f3 f3
  0x7a776ce99380: f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f3 f3 f3 f3
  0x7a776ce99400: f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f3 f3 f3 f3
=>0x7a776ce99480: f5 f5 f5 f5 f5 f5 f5 f5 f5 f5[f5]f5 f5 f5 f5 f5
  0x7a776ce99500: f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f3 f3 f3 f3
  0x7a776ce99580: f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5
  0x7a776ce99600: f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5
  0x7a776ce99680: f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f3 f3 f3 f3
  0x7a776ce99700: f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f3 f3 f3 f3
Shadow byte legend (one shadow byte represents 8 application bytes):
  Addressable:           00
  Partially addressable: 01 02 03 04 05 06 07 
  Heap left redzone:       fa
  Freed heap region:       fd
  Stack left redzone:      f1
  Stack mid redzone:       f2
  Stack right redzone:     f3
  Stack after return:      f5
  Stack use after scope:   f8
  Global redzone:          f9
  Global init order:       f6
  Poisoned by user:        f7
  Container overflow:      fc
  Array cookie:            ac
  Intra object redzone:    bb
  ASan internal:           fe
  Left alloca redzone:     ca
  Right alloca redzone:    cb
==50116==ABORTING
*/

TEST(DanglingStringViewTest, ShowsBug) {
    auto make_tokens = []() -> std::vector<std::string_view> {
        std::string temp = "/join general";   
        return Lexer::lex(temp);                
    };

    auto tokens = make_tokens();
    ASSERT_EQ(tokens.size(), 2u);

    EXPECT_EQ(tokens[0], "/join");
    EXPECT_EQ(tokens[1], "general");
}
// ЭТАП 3
TEST(ChatStoreTest, StrongGuaranteeOnMultiplePost) {
    ChatStore store;
    store.create_room("r1");
    store.create_room("r2");
    store.create_room("r3");

    Message::throw_on_copy = true; 

    EXPECT_THROW({
        store.post_many({"r1", "r2", "r3"}, Message("user", "hello"));
    }, std::runtime_error);

    Message::throw_on_copy = false; 

    EXPECT_EQ(store.get_rooms_data().at("r1").size(), 0);
    EXPECT_EQ(store.get_rooms_data().at("r2").size(), 0);
    EXPECT_EQ(store.get_rooms_data().at("r3").size(), 0);
}

// Тесты исключений
TEST(ChatStoreTest, ReactThrowsTypeMismatch) {
    ChatStore store;
    store.create_room("r");
    store.post("r", Message("a", "text")); // TextBody
    
    EXPECT_THROW(store.react("r", 0), TypeMismatchError);
}

TEST(ChatStoreTest, UnknownRoomThrows) {
    ChatStore store;
    EXPECT_THROW(store.post("nope", Message("a", "b")), UnknownRoomError);
}


// ДОМАШНЕЕ ЗАДАНИЕ

TEST(FuzzTest, RandomBytesDoNotCrash) {
    std::mt19937 rng(42); 
    std::uniform_int_distribution<int> dist(0, 255);

    for (int i = 0; i < 1000; ++i) {
        std::string random_str;
        int len = rng() % 100;
        for (int j = 0; j < len; ++j) {
            random_str.push_back(static_cast<char>(dist(rng)));
        }

        try {
            auto tokens = Lexer::lex(random_str);
            auto cmd = parse_command_expected(tokens);
            if (cmd) {
                (void)cmd.value();
            }
        } catch (const std::exception&) {
        }
    }
}

TEST(ChatStoreTest, WeakPtrMembership) {
    ChatStore store;
    store.register_user("alice");
    store.register_user("bob");
    store.join_room("alice", "general");
    store.join_room("bob", "general");
    
    EXPECT_EQ(store.active_members_count("general"), 2);
    
    store.remove_user("bob"); 
    
    EXPECT_EQ(store.active_members_count("general"), 1);
    store.cleanup_expired_members("general");
    EXPECT_EQ(store.active_members_count("general"), 1);
}

TEST(AppendOnlyLogTest, WritesAndCloses) {
    std::string filename = "test_log.txt";
    std::remove(filename.c_str());
    
    {
        AppendOnlyLog log(filename);
        log.write("Hello");
        log.write("World");
    }

    std::ifstream file(filename);
    std::string line;
    std::getline(file, line);
    EXPECT_EQ(line, "Hello");
    std::getline(file, line);
    EXPECT_EQ(line, "World");
    file.close();
    std::remove(filename.c_str());
}

TEST(AppendOnlyLogTest, ClosesOnException) {
    std::string filename = "test_log_exc.txt";
    std::remove(filename.c_str());
    try {
        AppendOnlyLog log(filename);
        log.write("Testing exception");
        throw std::runtime_error("Simulated crash");
    } catch (...) {}
    
    std::ifstream file(filename);
    EXPECT_TRUE(file.is_open());
    file.close();
    std::remove(filename.c_str());
}
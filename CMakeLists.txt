cmake_minimum_required(VERSION 3.16)
project(chat_project VERSION 0.6 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-Wall -Wextra -Wshadow -fconcepts-diagnostics-depth=2)

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/release-1.12.1.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_library(chat_core
    src/ChatStore.cpp
    src/Lexer.cpp
    src/Parser.cpp
)
target_include_directories(chat_core PUBLIC include)

add_executable(chat_cli src/main.cpp)
target_link_libraries(chat_cli PRIVATE chat_core)

add_executable(chat_tests tests/test_chat.cpp)
target_link_libraries(chat_tests PRIVATE chat_core gtest_main)

enable_testing()
include(GoogleTest)
gtest_discover_tests(chat_tests)


#add_executable(compile_error_test tests/compile_error_test.cpp)
#target_link_libraries(compile_error_test PRIVATE chat_core)
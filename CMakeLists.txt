cmake_minimum_required(VERSION 3.16)
project(chat_project)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Флаги качества
add_compile_options(-Wall -Wextra -Wshadow)

# GoogleTest через FetchContent — версия 1.12.1 нормально живёт с C++14
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/release-1.12.1.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Библиотека с логикой
add_library(chat_core
    src/ChatStore.cpp
)
target_include_directories(chat_core PUBLIC include)

# Консольный клиент
add_executable(chat_cli src/main.cpp)
target_link_libraries(chat_cli PRIVATE chat_core)

# Тесты
add_executable(chat_tests tests/test_chat.cpp)
target_link_libraries(chat_tests PRIVATE chat_core gtest_main)

enable_testing()
include(GoogleTest)
gtest_discover_tests(chat_tests)S
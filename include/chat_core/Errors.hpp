#pragma once

#include <stdexcept>
#include <string>

class ChatError : public std::runtime_error {
public:
    explicit ChatError(const std::string& msg) : std::runtime_error(msg) {}
};

class ParseError : public ChatError {
public:
    explicit ParseError(const std::string& msg) : ChatError("Parse Error: " + msg) {}
};

class UnknownRoomError : public ChatError {
public:
    explicit UnknownRoomError(const std::string& room) 
        : ChatError("Unknown room: " + room) {}
};

class NicknameTakenError : public ChatError {
public:
    explicit NicknameTakenError(const std::string& nick) 
        : ChatError("Nickname is already taken: " + nick) {}
};

class TypeMismatchError : public ChatError {
public:
    TypeMismatchError(const std::string& expected, const std::string& actual) 
        : ChatError("Type mismatch: expected " + expected + ", but got " + actual) {}
};
#pragma once
#include <string>
#include <cstdio>
#include <stdexcept>

class AppendOnlyLog {
private:
    std::FILE* file_;

public:
    explicit AppendOnlyLog(const std::string& filename) {
        file_ = std::fopen(filename.c_str(), "a");
        if (!file_) {
            throw std::runtime_error("Failed to open log file: " + filename);
        }
    }

    ~AppendOnlyLog() {
        if (file_) {
            std::fflush(file_);
            std::fclose(file_);
        }
    }

    AppendOnlyLog(const AppendOnlyLog&) = delete;
    AppendOnlyLog& operator=(const AppendOnlyLog&) = delete;

    // Перемещение разрешено
    AppendOnlyLog(AppendOnlyLog&& other) noexcept : file_(other.file_) {
        other.file_ = nullptr;
    }
    AppendOnlyLog& operator=(AppendOnlyLog&& other) noexcept {
        if (this != &other) {
            if (file_) {
                std::fclose(file_);
            }
            file_ = other.file_;
            other.file_ = nullptr;
        }
        return *this;
    }

    void write(const std::string& msg) {
        if (file_) {
            std::fputs(msg.c_str(), file_);
            std::fputc('\n', file_);
        }
    }
};
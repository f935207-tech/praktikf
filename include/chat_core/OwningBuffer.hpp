#pragma once
#include <memory>
#include <utility>
#include <algorithm>
#include <cstddef>

class OwningBuffer {
private:
    std::unique_ptr<char[]> data_;
    std::size_t size_;

public:
    explicit OwningBuffer(std::size_t size)
        : data_(std::make_unique<char[]>(size)), size_(size) {}

    OwningBuffer(const OwningBuffer& other)
        : data_(std::make_unique<char[]>(other.size_)), size_(other.size_) {
        std::copy(other.data_.get(), other.data_.get() + size_, data_.get());
    }

    OwningBuffer& operator=(const OwningBuffer& other) {
        if (this != &other) {
            OwningBuffer tmp(other);
            swap(tmp);
        }
        return *this;
    }

    OwningBuffer(OwningBuffer&&) noexcept = default;
    OwningBuffer& operator=(OwningBuffer&&) noexcept = default;

    void swap(OwningBuffer& other) noexcept {
        std::swap(data_, other.data_);
        std::swap(size_, other.size_);
    }

    std::size_t size() const { return size_; }
    const char* data() const { return data_.get(); }
    void set(std::size_t index, char value) { if (index < size_) data_[index] = value; }
    char get(std::size_t index) const { return index < size_ ? data_[index] : '\0'; }
};
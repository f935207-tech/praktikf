#pragma once

#include <cstddef>
#include <utility>
#include <algorithm>

class OwningBuffer {
private:
    char* data_;
    std::size_t size_;

public:
    explicit OwningBuffer(std::size_t size)
        : data_(new char[size]), size_(size) {}

    ~OwningBuffer() {
        delete[] data_;
    }

    OwningBuffer(const OwningBuffer& other)
        : data_(new char[other.size_]), size_(other.size_) {
        std::copy(other.data_, other.data_ + size_, data_);
    }

    // Копирующее присваивание через copy-and-swap.
    //
    // Почему это автоматически решает проблему самоприсваивания (a = a)?
    // Аргумент other передаётся ПО ЗНАЧЕНИЮ, то есть к моменту входа
    // в тело оператора уже существует независимая копия правого операнда.
    // Если написать a = a, то компилятор сначала сделает копию объекта a
    // (в эту самую локальную переменную other), и только потом мы
    // обменяем содержимое с этой копией. То есть:
    //   1. other — это уже отдельный объект со своими data_ и size_.
    //   2. swap(other) меняет местами поля this и other. После обмена
    //      this получает корректные данные из копии, а old-данные
    //      this уезжают в other.
    //   3. При выходе из оператора other — локальная переменная —
    //      разрушается, и её деструктор освобождает старую память this.
    // Никакого delete[] собственного указателя до того, как мы прочитали
    // из него данные, не происходит. Поэтому self-assignment безопасен
    // без отдельной проверки `if (this != &other)`.
    OwningBuffer& operator=(OwningBuffer other) {
        swap(other);
        return *this;
    }

    OwningBuffer(OwningBuffer&& other) noexcept
        : data_(other.data_), size_(other.size_) {
        other.data_ = nullptr;
        other.size_ = 0;
    }

    OwningBuffer& operator=(OwningBuffer&& other) noexcept {
        if (this != &other) {
            delete[] data_;
            data_ = other.data_;
            size_ = other.size_;
            other.data_ = nullptr;
            other.size_ = 0;
        }
        return *this;
    }

    void swap(OwningBuffer& other) noexcept {
        std::swap(data_, other.data_);
        std::swap(size_, other.size_);
    }

    std::size_t size() const { return size_; }
    const char* data() const { return data_; }

    void set(std::size_t index, char value) {
        if (index < size_) {
            data_[index] = value;
        }
    }

    char get(std::size_t index) const {
        if (index < size_) {
            return data_[index];
        }
        return '\0';
    }
};
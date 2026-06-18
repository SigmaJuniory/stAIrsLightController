#pragma once

#include <array>
#include <cstddef>
#include <stdexcept>
#include <utility>

template <typename Key, typename Value, std::size_t N> class static_map {
  private:
    std::array<std::pair<Key, Value>, N> data{};
    std::size_t count = 0;

  public:
    constexpr std::size_t size() const noexcept {
        return count;
    }

    constexpr std::size_t capacity() const noexcept {
        return N;
    }

    constexpr bool empty() const noexcept {
        return count == 0;
    }

    bool contains(const Key &key) const noexcept {
        return find(key) != end();
    }

    Value &at(const Key &key) {
        auto it = find(key);
        if (it == end()) {
            throw std::out_of_range("key not found");
        }
        return it->second;
    }

    const Value &at(const Key &key) const {
        auto it = find(key);
        if (it == end()) {
            throw std::out_of_range("key not found");
        }
        return it->second;
    }

    Value &operator[](const Key &key) {
        auto it = find(key);
        if (it != end()) {
            return it->second;
        }

        if (count >= N) {
            throw std::length_error("static_map is full");
        }

        data[count] = {key, Value{}};
        return data[count++].second;
    }

    void insert(const Key &key, const Value &value) {
        auto it = find(key);

        if (it != end()) {
            it->second = value; // aktualizacja istniejącego wpisu
            return;
        }

        if (count >= N) {
            throw std::length_error("static_map is full");
        }

        data[count++] = {key, value};
    }

    void erase(const Key &key) {
        for (std::size_t i = 0; i < count; ++i) {
            if (data[i].first == key) {
                data[i] = std::move(data[count - 1]);
                --count;
                return;
            }
        }
    }

    auto find(const Key &key) noexcept {
        for (std::size_t i = 0; i < count; ++i) {
            if (data[i].first == key) {
                return data.begin() + i;
            }
        }
        return end();
    }

    auto find(const Key &key) const noexcept {
        for (std::size_t i = 0; i < count; ++i) {
            if (data[i].first == key) {
                return data.begin() + i;
            }
        }
        return end();
    }

    auto begin() noexcept {
        return data.begin();
    }

    auto end() noexcept {
        return data.begin() + count;
    }

    auto begin() const noexcept {
        return data.begin();
    }

    auto end() const noexcept {
        return data.begin() + count;
    }

    constexpr static_map() = default;

    constexpr static_map(std::initializer_list<std::pair<Key, Value>> init) {
        if (init.size() > N) {
            throw std::length_error("initializer_list size exceeds static_map capacity");
        }

        for (const auto &item : init) {
            insert(item.first, item.second);
        }
    }
};

#pragma once

#include <array>
#include <cstddef>
#include <stdexcept>
#include <utility>

// Static vector implementation for embedded systems - no dynamic allocation
template <typename T, std::size_t N> class static_vector {
  private:
    std::array<T, N> data;
    std::size_t count = 0;

  public:
    constexpr std::size_t size() const noexcept {
        return count;
    }
    constexpr std::size_t capacity() const noexcept {
        return N;
    }

    // TODO: Probably most functions in static containers can be constexpr.
    constexpr void push_back(const T &v) {
        if (count < N) {
            data[count++] = v;
        } else {
            throw std::length_error("static_vector is full");
        }
    }

    constexpr void push_back(T &&v) {
        if (count < N) {
            data[count++] = std::move(v);
        } else {
            throw std::length_error("static_vector is full");
        }
    }

    constexpr T &operator[](std::size_t i) noexcept {
        return data[i];
    }
    constexpr const T &operator[](std::size_t i) const noexcept {
        return data[i];
    }

    constexpr T *begin() noexcept {
        return data.data();
    }
    constexpr T *end() noexcept {
        return data.data() + count;
    }
    constexpr const T *begin() const noexcept {
        return data.data();
    }
    constexpr const T *end() const noexcept {
        return data.data() + count;
    }
};

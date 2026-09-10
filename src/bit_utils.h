#ifndef BIT_UTILS_H
#define BIT_UTILS_H

#include <cstdint>

namespace gb {

template <typename T>
constexpr bool test_bit(T val, uint8_t bit) noexcept {
    return (val & (static_cast<T>(1) << bit)) != 0;
}

template <typename T>
constexpr T set_bit(T val, uint8_t bit) noexcept {
    return static_cast<T>(val | (static_cast<T>(1) << bit));
}

template <typename T>
constexpr T clear_bit(T val, uint8_t bit) noexcept {
    return static_cast<T>(val & ~(static_cast<T>(1) << bit));
}

}  // namespace gb

#endif

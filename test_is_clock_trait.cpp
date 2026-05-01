#include <type_traits>

#include <gtest/gtest.h>

#include "compatible_chrono.hpp"

// Detect presence of variable-template compatible_chrono::is_clock_v<T>
template <typename T, typename = void>
struct has_is_clock_v : std::false_type {};

template <typename T>
struct has_is_clock_v<T, std::void_t<decltype(compatible_chrono::is_clock_v<T>)>> : std::true_type {};

// Detect presence of class-template compatible_chrono::is_clock<T>::value
template <typename T, typename = void>
struct has_is_clock_trait_class : std::false_type {};

template <typename T>
struct has_is_clock_trait_class<T, std::void_t<decltype(compatible_chrono::is_clock<T>::value)>> : std::true_type {};

TEST(CompatibleChronoTraits, IsClockTraitBehavior) {
    // Prefer variable-template if available, fall back to class trait, otherwise skip.
    if constexpr (has_is_clock_v<std::chrono::system_clock>::value) {
        EXPECT_TRUE(compatible_chrono::is_clock_v<std::chrono::system_clock>);
        EXPECT_TRUE(compatible_chrono::is_clock_v<std::chrono::steady_clock>);
        EXPECT_FALSE(compatible_chrono::is_clock_v<int>);
    } else if constexpr (has_is_clock_trait_class<std::chrono::system_clock>::value) {
        EXPECT_TRUE(compatible_chrono::is_clock<std::chrono::system_clock>::value);
        EXPECT_TRUE(compatible_chrono::is_clock<std::chrono::steady_clock>::value);
        EXPECT_FALSE(compatible_chrono::is_clock<int>::value);
    } else {
        GTEST_SKIP() << "compatible_chrono::is_clock / is_clock_v is not available in this configuration.";
    }
}
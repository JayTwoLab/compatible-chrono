#include "compatible_chrono.hpp"

#include <gtest/gtest.h>

#include <type_traits>

// Helper traits to detect presence of is_clock_v variable template and is_clock class template
template <typename T, typename = void>
struct has_is_clock_v : std::false_type {};
  
// SFINAE test for variable template presence (C++17 and later)
template <typename T>
struct has_is_clock_v<T, std::void_t<decltype(compatible_chrono::is_clock_v<T>)>> : std::true_type {};

// SFINAE test for class template presence (fallback if variable template not available)
template <typename T, typename = void>
struct has_is_clock_trait_class : std::false_type {};

// SFINAE test for class template presence (fallback if variable template not available)
template <typename T>
struct has_is_clock_trait_class<T, std::void_t<decltype(compatible_chrono::is_clock<T>::value)>> : std::true_type {};

// Test to verify that compatible_chrono::is_clock_v (or is_clock trait class) correctly identifies clock types and non-clock types
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

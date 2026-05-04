#include "compatible_chrono.hpp"
#include <gtest/gtest.h>
#include <iostream>

TEST(local_now_gtest, extract_components_and_ranges)
{
    namespace cc = compatible_chrono; // shorten namespace for convenience
    using ms = cc::milliseconds; // alias for milliseconds duration

    auto opt = local_now(); // get local time 
    if (!opt)
        GTEST_SKIP() << "local_now() unavailable; timezone DB may be missing";

    auto now_local = *opt;
    auto c = local_components::decompose_local(now_local);

	std::cout << CHRONO_FORMAT("%F %T", now_local) << '\n'; // print the local time for debugging

    EXPECT_GE(c.year, 1970);                // reasonable lower bound
    EXPECT_GE(c.month, 1u);
    EXPECT_LE(c.month, 12u);
    EXPECT_GE(c.day, 1u);
    EXPECT_LE(c.day, 31u);
    EXPECT_LE(c.hour, 23u);
    EXPECT_LE(c.minute, 59u);
    EXPECT_LE(c.second, 60u);               // allow leap-second upper bound
    EXPECT_LE(c.millisecond, 999u);
}

TEST(local_now_gtest, span_between_local_times)
{
    namespace cc = compatible_chrono; // shorten namespace for convenience
    using ms = cc::milliseconds; // alias for milliseconds duration

    auto opt = local_now();
    if (!opt) 
        GTEST_SKIP() << "local_now() unavailable; timezone DB may be missing";
    auto now_local = *opt;

	auto other_local = now_local - cc::minutes{ 123 }; // before 123 minutes
	auto span = now_local - other_local; // base time - comparing time = span time 
    auto span_minutes = cc::duration_cast<cc::minutes>(span).count();

	EXPECT_EQ(span_minutes, 123); // the difference should be exactly 123 minutes

    // also verify milliseconds conversion
    auto span_ms = cc::duration_cast<ms>(span).count();
	EXPECT_EQ(span_ms, 123 * 60 * 1000); // 123 minutes in milliseconds
}

TEST(local_now_gtest, five_minutes_ago_components_and_difference)
{
    namespace cc = compatible_chrono; // shorten namespace for convenience
    using ms = cc::milliseconds; // alias for milliseconds duration

    auto opt = local_now();
    if (!opt) 
        GTEST_SKIP() << "local_now() unavailable; timezone DB may be missing";
    auto now_local = *opt;

    auto five_min_ago = now_local - cc::minutes{5}; // 5 minutes ago 

    // difference must be exactly 5 minutes
	auto span = now_local - five_min_ago;
    auto dmin = cc::duration_cast<cc::minutes>(span).count();
    EXPECT_EQ(dmin, 5);

    // Decompose both and validate component ranges
    auto now_c = local_components::decompose_local(now_local);
    auto ago_c = local_components::decompose_local(five_min_ago);

    EXPECT_GE(now_c.year, 1970);
    EXPECT_GE(ago_c.year, 1970);

    EXPECT_GE(now_c.month, 1u);
    EXPECT_LE(now_c.month, 12u);
    EXPECT_GE(ago_c.month, 1u);
    EXPECT_LE(ago_c.month, 12u);

    // The minute values should differ by 5 modulo 60, allow hour/day change across midnight
    int minute_diff = static_cast<int>(now_c.minute) - static_cast<int>(ago_c.minute);
    if (minute_diff < 0) 
        minute_diff += 60;
    EXPECT_EQ(minute_diff, 5);
}



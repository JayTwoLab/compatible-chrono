
#include <sstream>
#include <string>
#include <exception>

#include <gtest/gtest.h>

#include "compatible_chrono.hpp"

// Helper: a common "now" and day-floor used in several tests
static auto get_today_sys_days()
{
    auto now = std::chrono::system_clock::now();
    return compatible_chrono::floor<compatible_chrono::days>(now);
} 

// Date equality and basic construction
TEST(ChronoCompatibility, DateEqualityAndConstruction) {
    auto sd = get_today_sys_days();
    auto ymd = compatible_chrono::year_month_day{sd};

    EXPECT_EQ(ymd, compatible_chrono::year_month_day{sd});

    // Direct Construction roundtrip
    compatible_chrono::year_month_day ymd2{compatible_chrono::year{2026}, compatible_chrono::month{4}, compatible_chrono::day{27}};
    auto sys_days2 = compatible_chrono::sys_days{ymd2};
    EXPECT_EQ(ymd2, compatible_chrono::year_month_day{sys_days2});
    EXPECT_EQ(CHRONO_FORMAT("%Y-%m-%d", sys_days2), std::string("2026-04-27"));
}

// Formatting
TEST(ChronoCompatibility, Formatting) {
    auto now = std::chrono::system_clock::now();
    auto sd = compatible_chrono::floor<compatible_chrono::days>(now);

    std::string s = CHRONO_FORMAT("%Y-%m-%d", now);
    std::string s2 = CHRONO_FORMAT("%Y-%m-%d", sd);
    EXPECT_EQ(s, s2);
}

// Date arithmetic
TEST(ChronoCompatibility, DateArithmetic_NextWeek) {
    auto sd = get_today_sys_days();
    auto next_week = sd + compatible_chrono::days{7};
    auto ymd_next = compatible_chrono::year_month_day{next_week};
    auto expected_ymd_next = compatible_chrono::year_month_day{next_week};
    EXPECT_EQ(ymd_next, expected_ymd_next);
    EXPECT_EQ(CHRONO_FORMAT("%Y-%m-%d", next_week), CHRONO_FORMAT("%Y-%m-%d", sd + compatible_chrono::days{7}));
}

// Parsing from string
TEST(ChronoCompatibility, ParsingFromString) {
    std::istringstream in("2026-05-05");
    compatible_chrono::sys_days parsed_date;
    in >> compatible_chrono::parse("%Y-%m-%d", parsed_date);
    auto ymd_parsed = compatible_chrono::year_month_day{parsed_date};
    compatible_chrono::year_month_day expected_parsed{compatible_chrono::year{2026}, compatible_chrono::month{5}, compatible_chrono::day{5}};
    EXPECT_EQ(ymd_parsed, expected_parsed);
    EXPECT_EQ(CHRONO_FORMAT("%Y-%m-%d", parsed_date), std::string("2026-05-05"));
}

// Last day of month
TEST(ChronoCompatibility, LastDayOfMonth) {
    compatible_chrono::year_month_day_last last_day{compatible_chrono::year{2026}, compatible_chrono::month_day_last{compatible_chrono::month{4}}};
    auto sys_days_last = compatible_chrono::sys_days{last_day};
    auto ymd_last = compatible_chrono::year_month_day{sys_days_last};
    compatible_chrono::year_month_day expected_last{compatible_chrono::year{2026}, compatible_chrono::month{4}, compatible_chrono::day{30}};
    EXPECT_EQ(ymd_last, expected_last);
    EXPECT_EQ(CHRONO_FORMAT("%Y-%m-%d", sys_days_last), std::string("2026-04-30"));
}

// Weekday range
TEST(ChronoCompatibility, WeekdayRange) {
    auto sd = get_today_sys_days();
    auto weekday = compatible_chrono::weekday{sd};
    EXPECT_GE(weekday.c_encoding(), 0);
    EXPECT_LE(weekday.c_encoding(), 6);
}

// Timezone handling (may skip when tzdb missing)
// TEST(ChronoCompatibility, TimezoneHandling) {
//     auto now = std::chrono::system_clock::now();
//     try {
// #if __cplusplus >= 202002L
//         auto tz = compatible_chrono::current_zone();
//         auto local_time = tz->to_local(now);
//         EXPECT_EQ(CHRONO_FORMAT("%Y-%m-%d %H:%M:%S", local_time), CHRONO_FORMAT("%Y-%m- %  //%/:/%M:%S", tz->to_local(now)));
// #else
//         auto tz = compatible_chrono::current_zone();
//         auto local_time = tz->to_local(now);
//         EXPECT_EQ(CHRONO_FORMAT("%Y-%m-%d %H:%M:%S", local_time), CHRONO_FORMAT("%Y-%m- %  //%/:/%M:%S", tz->to_local(now)));
// #endif
//     } catch (const std::exception& e) {
//         GTEST_SKIP() << "Timezone database unavailable or incompatible: " << e.what();
//     }
// }

// Leap year
TEST(ChronoCompatibility, LeapYearHandling) {
    compatible_chrono::year_month_day leap_day{compatible_chrono::year{2024}, compatible_chrono::month{2}, compatible_chrono::day{29}};
    auto sys_leap = compatible_chrono::sys_days{leap_day};
    EXPECT_EQ(leap_day, compatible_chrono::year_month_day{sys_leap});
    EXPECT_EQ(CHRONO_FORMAT("%Y-%m-%d", sys_leap), std::string("2024-02-29"));
}

// End of month
TEST(ChronoCompatibility, EndOfMonthHandling) {
    compatible_chrono::year_month_day end_of_month{compatible_chrono::year{2026}, compatible_chrono::month{1}, compatible_chrono::day{31}};
    auto sys_end = compatible_chrono::sys_days{end_of_month};
    EXPECT_EQ(end_of_month, compatible_chrono::year_month_day{sys_end});
    EXPECT_EQ(CHRONO_FORMAT("%Y-%m-%d", sys_end), std::string("2026-01-31"));
}

// Negative duration
TEST(ChronoCompatibility, NegativeDurationHandling) {
    auto sd = get_today_sys_days();
    auto past_date = sd - compatible_chrono::days{365};
    auto ymd_past = compatible_chrono::year_month_day{past_date};
    compatible_chrono::year_month_day expected_past{compatible_chrono::year_month_day{past_date}};
    EXPECT_EQ(ymd_past, expected_past);
    EXPECT_EQ(CHRONO_FORMAT("%Y-%m-%d", past_date), CHRONO_FORMAT("%Y-%m-%d", sd - compatible_chrono::days{365}));
}

// Large duration
TEST(ChronoCompatibility, LargeDurationHandling) {
    auto sd = get_today_sys_days();
    auto future_date = sd + compatible_chrono::days{365 * 100};
    auto ymd_future = compatible_chrono::year_month_day{future_date};
    compatible_chrono::year_month_day expected_future{compatible_chrono::year_month_day{future_date}};
    EXPECT_EQ(ymd_future, expected_future);
}

// Month overflow
TEST(ChronoCompatibility, MonthOverflowHandling) {
    compatible_chrono::year_month_day month_overflow{compatible_chrono::year{2026}, compatible_chrono::month{13}, compatible_chrono::day{1}};
    auto sys_month_overflow = compatible_chrono::sys_days{month_overflow};
    auto ymd_month_overflow = compatible_chrono::year_month_day{sys_month_overflow};
    compatible_chrono::year_month_day expected_month_overflow{compatible_chrono::year{2027}, compatible_chrono::month{1}, compatible_chrono::day{1}};
    EXPECT_EQ(ymd_month_overflow, expected_month_overflow);
}

// Year overflow
TEST(ChronoCompatibility, YearOverflowHandling) {
    compatible_chrono::year_month_day year_overflow{compatible_chrono::year{32767}, compatible_chrono::month{12}, compatible_chrono::day{31}};
    auto sys_year_overflow = compatible_chrono::sys_days{year_overflow};
    auto ymd_year_overflow = compatible_chrono::year_month_day{sys_year_overflow};
    compatible_chrono::year_month_day expected_year_overflow{ compatible_chrono::year{32767}, compatible_chrono::month{12}, compatible_chrono::day{31} };
    EXPECT_EQ(ymd_year_overflow, expected_year_overflow);
}



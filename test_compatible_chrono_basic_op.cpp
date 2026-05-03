#include "compatible_chrono.hpp"

#include <gtest/gtest.h>

#include <sstream>
#include <string>
#include <exception>

// Helper: "today" used across tests and its value floored to days
static auto get_today_sys_days()
{
	auto now = compatible_chrono::system_clock::now(); // Get current system time (UTC)

	return compatible_chrono::floor<compatible_chrono::days>(now); // Floor the current time to days and return as sys_days (compatible_chrono::floor is similar to date::floor, compatible_chrono::days is similar to date::days)
}  
  
// Date equality and basic construction tests
TEST(ChronoCompatibility, DateEqualityAndConstruction) {
    auto sd = get_today_sys_days();
	auto ymd = compatible_chrono::year_month_day{ sd }; // Convert sys_days to year_month_day
    EXPECT_EQ(ymd, compatible_chrono::year_month_day{sd});

    // Construct directly and verify roundtrip
    compatible_chrono::year_month_day ymd2{
        compatible_chrono::year{2026}, 
        compatible_chrono::month{4}, 
        compatible_chrono::day{27} };

	auto sys_days2 = compatible_chrono::sys_days{ ymd2 }; // Convert year_month_day back to sys_days

    EXPECT_EQ(ymd2, compatible_chrono::year_month_day{sys_days2});
    EXPECT_EQ(CHRONO_FORMAT("%Y-%m-%d", sys_days2), std::string("2026-04-27"));
}

// Formatting test
TEST(ChronoCompatibility, Formatting) {
    auto now = compatible_chrono::system_clock::now();
    auto sd = compatible_chrono::floor<compatible_chrono::days>(now);

    std::string s = CHRONO_FORMAT("%Y-%m-%d", now);
    std::string s2 = CHRONO_FORMAT("%Y-%m-%d", sd);
    EXPECT_EQ(s, s2);
}

// Date arithmetic test (next week calculation)
TEST(ChronoCompatibility, DateArithmetic_NextWeek) {
    auto sd = get_today_sys_days();

	auto next_week = sd + compatible_chrono::days{ 7 }; // Compute 7 days later
	auto ymd_next = compatible_chrono::year_month_day{ next_week }; // Convert sys_days to year_month_day for next week

	auto expected_ymd_next = compatible_chrono::year_month_day{ next_week }; // Expected next week as year_month_day

    EXPECT_EQ(ymd_next, expected_ymd_next);
    EXPECT_EQ(CHRONO_FORMAT("%Y-%m-%d", next_week), CHRONO_FORMAT("%Y-%m-%d", sd + compatible_chrono::days{7}));
}

// Parsing from string test
TEST(ChronoCompatibility, ParsingFromString) {
    std::istringstream in("2026-05-05");
	compatible_chrono::sys_days parsed_date; // sys_days variable to hold parsed date

	in >> compatible_chrono::parse("%Y-%m-%d", parsed_date); // Parse "2026-05-05" into parsed_date
	auto ymd_parsed = compatible_chrono::year_month_day{ parsed_date }; // Convert parsed sys_days to year_month_day
    compatible_chrono::year_month_day expected_parsed{
        compatible_chrono::year{2026}, 
        compatible_chrono::month{5}, 
        compatible_chrono::day{5} };

    EXPECT_EQ(ymd_parsed, expected_parsed);
    EXPECT_EQ(CHRONO_FORMAT("%Y-%m-%d", parsed_date), std::string("2026-05-05"));
}

// Last day of month handling test
TEST(ChronoCompatibility, LastDayOfMonth) {
    compatible_chrono::year_month_day_last last_day{compatible_chrono::year{2026}, compatible_chrono::month_day_last{compatible_chrono::month{4}}};
    auto sys_days_last = compatible_chrono::sys_days{last_day};
    auto ymd_last = compatible_chrono::year_month_day{sys_days_last};
    compatible_chrono::year_month_day expected_last{compatible_chrono::year{2026}, compatible_chrono::month{4}, compatible_chrono::day{30}};
    EXPECT_EQ(ymd_last, expected_last);
    EXPECT_EQ(CHRONO_FORMAT("%Y-%m-%d", sys_days_last), std::string("2026-04-30"));
}

// Weekday range check (0..6)
TEST(ChronoCompatibility, WeekdayRange) {
    auto sd = get_today_sys_days();
    auto weekday = compatible_chrono::weekday{sd};
    EXPECT_GE(weekday.c_encoding(), 0);
    EXPECT_LE(weekday.c_encoding(), 6);
}

// Leap year handling test (2024-02-29)
TEST(ChronoCompatibility, LeapYearHandling) {
    compatible_chrono::year_month_day leap_day{compatible_chrono::year{2024}, compatible_chrono::month{2}, compatible_chrono::day{29}};
    auto sys_leap = compatible_chrono::sys_days{leap_day};
    EXPECT_EQ(leap_day, compatible_chrono::year_month_day{sys_leap});
    EXPECT_EQ(CHRONO_FORMAT("%Y-%m-%d", sys_leap), std::string("2024-02-29"));
}

// End-of-month handling test (January 31)
TEST(ChronoCompatibility, EndOfMonthHandling) {
    compatible_chrono::year_month_day end_of_month{compatible_chrono::year{2026}, compatible_chrono::month{1}, compatible_chrono::day{31}};
    auto sys_end = compatible_chrono::sys_days{end_of_month};
    EXPECT_EQ(end_of_month, compatible_chrono::year_month_day{sys_end});
    EXPECT_EQ(CHRONO_FORMAT("%Y-%m-%d", sys_end), std::string("2026-01-31"));
}

// Negative duration handling test (past date calculation)
TEST(ChronoCompatibility, NegativeDurationHandling) {
    auto sd = get_today_sys_days();
    auto past_date = sd - compatible_chrono::days{365};
    auto ymd_past = compatible_chrono::year_month_day{past_date};
    compatible_chrono::year_month_day expected_past{compatible_chrono::year_month_day{past_date}};
    EXPECT_EQ(ymd_past, expected_past);
    EXPECT_EQ(CHRONO_FORMAT("%Y-%m-%d", past_date), CHRONO_FORMAT("%Y-%m-%d", sd - compatible_chrono::days{365}));
}

// Large duration handling test (100 years later)
TEST(ChronoCompatibility, LargeDurationHandling) {
    auto sd = get_today_sys_days();
    auto future_date = sd + compatible_chrono::days{365 * 100};
    auto ymd_future = compatible_chrono::year_month_day{future_date};
    compatible_chrono::year_month_day expected_future{compatible_chrono::year_month_day{future_date}};
    EXPECT_EQ(ymd_future, expected_future);
}

// Month overflow handling test (month 13 -> next year's January)
TEST(ChronoCompatibility, MonthOverflowHandling) {
    compatible_chrono::year_month_day month_overflow{compatible_chrono::year{2026}, compatible_chrono::month{13}, compatible_chrono::day{1}};
    auto sys_month_overflow = compatible_chrono::sys_days{month_overflow};
    auto ymd_month_overflow = compatible_chrono::year_month_day{sys_month_overflow};
    compatible_chrono::year_month_day expected_month_overflow{compatible_chrono::year{2027}, compatible_chrono::month{1}, compatible_chrono::day{1}};
    EXPECT_EQ(ymd_month_overflow, expected_month_overflow);
}

// Year boundary handling test (near-maximum year handling)
TEST(ChronoCompatibility, YearOverflowHandling) {
    compatible_chrono::year_month_day year_overflow{compatible_chrono::year{32767}, compatible_chrono::month{12}, compatible_chrono::day{31}};
    auto sys_year_overflow = compatible_chrono::sys_days{year_overflow};
    auto ymd_year_overflow = compatible_chrono::year_month_day{sys_year_overflow};
    compatible_chrono::year_month_day expected_year_overflow{ compatible_chrono::year{32767}, compatible_chrono::month{12}, compatible_chrono::day{31} };
    EXPECT_EQ(ymd_year_overflow, expected_year_overflow);
}

// --- Tests related to local_days ---

// local_days default construction and roundtrip with year_month_day
TEST(ChronoCompatibility, LocalDaysConstructionAndRoundtrip) {
    compatible_chrono::year_month_day ymd{
        compatible_chrono::year{2026}, 
        compatible_chrono::month{5}, 
        compatible_chrono::day{3} };

	compatible_chrono::local_days ld{ ymd }; // Convert year_month_day to local_days

	auto ymd_from_local = compatible_chrono::year_month_day{ ld }; // Convert local_days back to year_month_day and verify equality

    EXPECT_EQ(ymd_from_local, ymd);
}

// Day arithmetic on local_days (next day)
TEST(ChronoCompatibility, LocalDaysArithmetic_NextDay) {
    compatible_chrono::year_month_day ymd{
        compatible_chrono::year{2026}, 
        compatible_chrono::month{12}, 
        compatible_chrono::day{31} };

	compatible_chrono::local_days ld{ ymd }; // Convert year_month_day to local_days

	auto next_day = ld + compatible_chrono::days{ 1 }; // Add 1 day to local_days

	auto ymd_next = compatible_chrono::year_month_day{ next_day }; // Convert next day to year_month_day and expect 2027-01-01

    compatible_chrono::year_month_day expected_next{
        compatible_chrono::year{2027}, 
        compatible_chrono::month{1}, 
        compatible_chrono::day{1} };

    EXPECT_EQ(ymd_next, expected_next);
}

// When time zone information is available: verify sys_days <-> local_days roundtrip (skip test if unavailable)
#if __cplusplus < 202002L
// In C++17, using Howard Hinnant's date library, date::current_zone() is available only when the tz database is present.
// Skip this test in environments without a timezone database.
TEST(ChronoCompatibility, LocalDaysTimezoneRoundtrip_WhenTimezoneAvailable) {
    try {
        // date::current_zone() is available when using Howard Hinnant's date (C++17 mode)
        auto tz = date::current_zone(); // may throw if tzdb not available
        compatible_chrono::sys_days sd = get_today_sys_days();

        // Convert system-days -> local-time -> local-days
        auto local_tp = tz->to_local(sd);
        compatible_chrono::local_days ld = compatible_chrono::floor<compatible_chrono::days>(local_tp);

        // Convert local-days -> system-time -> back to local-days
        auto back_sys_tp = tz->to_sys(ld);
        compatible_chrono::local_days ld_after = compatible_chrono::floor<compatible_chrono::days>(tz->to_local(back_sys_tp));

        // Local calendar day should be preserved
        EXPECT_EQ(ld_after, ld);
    } catch (const std::exception& e) {
        GTEST_SKIP() << "Time zone database unavailable or incompatible: " << e.what();
    }
}
#else
//TEST(ChronoCompatibility, LocalDaysTimezoneRoundtrip_WhenTimezoneAvailable) {
//	// In C++20 std::chrono::current_zone() is always provided, but platforms without a tzdb may still throw.
// Therefore, skip the timezone roundtrip test in C++20 when the tzdb is not available.
//    GTEST_SKIP() << "Timezone roundtrip test skipped in C++20/std::chrono build on this platform.";
// }
#endif



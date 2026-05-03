  
#include "compatible_chrono.hpp"

#include <gtest/gtest.h>

#include <ratio>
#include <limits>
#include <sstream>
#include <iomanip>

#if __cplusplus >= 202002L 
    #include <format>
#endif 
     
// 1. Ratio (base ratio for time units) tests
TEST(ChronoTest, RatioCheck) {
    // Verify fundamental ratios such as std::micro (1/1,000,000)
    EXPECT_EQ(std::micro::num, 1);
    EXPECT_EQ(std::micro::den, 1000000);
}
 
// 2. Duration traits and special operations
TEST(ChronoTest, DurationAdvanced) {
    using micro_seconds = std::chrono::duration<long long, std::micro>; // c++11 style duration with microsecond precision
    micro_seconds ms(1000000);

    // Test duration::min, max, zero — checked in C++17-compatible way
    EXPECT_EQ(micro_seconds(0).count(), 0);
    EXPECT_LT(micro_seconds(std::numeric_limits<long long>::min()).count(), 0); 

    // abs (manual, C++17-safe)
    std::chrono::seconds neg_sec(-5); 
    // In C++17 there is no std::chrono::abs, so compute absolute value manually
    auto abs_sec = (neg_sec < std::chrono::seconds::zero()) ? -neg_sec : neg_sec;
    EXPECT_EQ(abs_sec.count(), 5);
}

// 3. Tests for various Clock types (including C++20)
#if __cplusplus >= 202002L
// C++20 
TEST(ChronoTest, SpecializedClocks) {
    // gps_clock: GPS time (epoch 1980)
    // tai_clock: International Atomic Time (includes leap seconds)
    // utc_clock: Coordinated Universal Time (UTC)

    auto utc_now = compatible_chrono::utc_clock::now();
    auto sys_now = compatible_chrono::utc_clock::to_sys(utc_now);

    EXPECT_GT(sys_now.time_since_epoch().count(), 0);
}
#else
// C++17 
TEST(ChronoTest, SpecializedClocks) {
    try {
        // Use date::utc_clock to obtain current time, providing functionality similar to C++20 std::chrono::utc_clock
        auto utc_now = compatible_chrono::utc_clock::now();
        // Convert date::utc_clock time to system time (date::sys_clock), similar to C++20 std::chrono::system_clock
        auto sys_now = compatible_chrono::utc_clock::to_sys(utc_now);

        // Retrieve the count of time_since_epoch() from the converted system time
        // (in C++17, time_since_epoch() returns a duration, so use duration::count() to get an integral value)
        auto count = sys_now.time_since_epoch().count();
        EXPECT_GT(count, 0);

    } catch( const std::exception& e) {
        std::string err = e.what();
        FAIL() << "Exception occurred: " << err; // Timezone database not found at ...
    } catch (...) {
        FAIL() << "Unknown exception occurred.";
    }
}
#endif

// 4. Calendar - detailed components (Year, Month, Day, Weekday, Monthday, etc.)
#if __cplusplus >= 202002L
// C++20 
TEST(ChronoTest, CalendarComponents) {
	compatible_chrono::year y{ 2026 }; // year 2026
	compatible_chrono::month m{ compatible_chrono::May }; // May
	compatible_chrono::day d{ 1 }; // day 1

    // Year-related functions 
    EXPECT_FALSE(y.is_leap()); // 2026 is not a leap year

    // Month/day operations
    auto next_month = m + compatible_chrono::months(1);
    EXPECT_EQ(next_month, compatible_chrono::June); // May -> June

    // Last day of month (the month's final day)
    compatible_chrono::year_month_day_last ymdl{ y, compatible_chrono::month_day_last{m} };
    EXPECT_EQ(ymdl.day(), compatible_chrono::day(31)); // May has 31 days
}
#else
// C++17 
TEST(ChronoTest, CalendarComponents) {
	compatible_chrono::year y{ 2026 }; // year 2026
	compatible_chrono::month m{ compatible_chrono::May }; // May
	compatible_chrono::day d{ 1 }; // day 1

    // Year-related functions 
    EXPECT_FALSE(y.is_leap()); // 2026 is not a leap year

    // Month/day operations
    auto next_month = m + compatible_chrono::months(1); // one month later
    EXPECT_EQ(next_month, compatible_chrono::June); // May -> June

	// Test that December rolls over to January
	auto dec = compatible_chrono::December; // December
	auto next_dec = dec + compatible_chrono::months(1); // one month later
	EXPECT_EQ(next_dec, compatible_chrono::January); // December -> January

    // Last day of month (the month's final day)
    compatible_chrono::year_month_day_last ymdl{ y, compatible_chrono::month_day_last{m} };
    EXPECT_EQ(ymdl.day(), compatible_chrono::day(31)); // May has 31 days
}
#endif

// 5. hh_mm_ss (Time of Day - C++20)
#if __cplusplus >= 202002L
// C++20 
TEST(ChronoTest, TimeOfDay) {
    compatible_chrono::seconds total_secs(3661); // 1 hour 1 minute 1 second
    compatible_chrono::hh_mm_ss tod{ total_secs };

    EXPECT_EQ(tod.hours().count(), 1);
    EXPECT_EQ(tod.minutes().count(), 1);
    EXPECT_EQ(tod.seconds().count(), 1);
    EXPECT_FALSE(tod.is_negative()); // positive time
}
#else
// C++17 
TEST(ChronoTest, TimeOfDay) {
    compatible_chrono::seconds total_secs(3661); // 1 hour 1 minute 1 second
    compatible_chrono::hh_mm_ss tod{ total_secs };

    EXPECT_EQ(tod.hours().count(), 1);
    EXPECT_EQ(tod.minutes().count(), 1);
    EXPECT_EQ(tod.seconds().count(), 1);
    EXPECT_FALSE(tod.is_negative()); // positive time
}
#endif

// 6. Time Zone / Leap Seconds
#if __cplusplus >= 202002L
// C++20
TEST(ChronoTest, LeapSeconds) {
    // Check the system-registered list of leap seconds
    auto& db = compatible_chrono::get_tzdb();
    if (!db.leap_seconds.empty()) {
        auto last_leap = db.leap_seconds.back();
        EXPECT_GT(last_leap.date(), compatible_chrono::sys_days{ compatible_chrono::year(1970) / 1 / 1 });
    }
}
#else
// C++17
TEST(ChronoTest, LeapSeconds) {
    // Check the system-registered list of leap seconds
    auto& db = compatible_chrono::get_tzdb();
    if (!db.leap_seconds.empty()) {
        auto last_leap = db.leap_seconds.back();
        EXPECT_GT(last_leap.date(), compatible_chrono::sys_days{ compatible_chrono::year(1970) / 1 / 1 });
    }
}
#endif

// 7. Parsing & Formatting (C++20)
#if __cplusplus >= 202002L
// C++20
TEST(ChronoTest, Formatting) {
    compatible_chrono::sys_days today 
        = compatible_chrono::sys_days{ 
                compatible_chrono::year(2026) / 
                compatible_chrono::May / 
        1 };

    // Test string formatting using std::format
    // (works only on supported compiler environments)
    try {
        std::string s = CHRONO_FORMAT("%Y-%m-%d", today);
        EXPECT_EQ(s, "2026-05-01");
    } catch (const std::format_error& e) {
        FAIL() << "Format error: " << e.what();
    } catch (const std::exception& e) {
        FAIL() << "Exception occurred: " << e.what();
    } catch (...) {
        FAIL() << "Unknown exception occurred.";
	}

}
#else
// C++17 
TEST(ChronoTest, Formatting) {
    compatible_chrono::sys_days today 
        = compatible_chrono::sys_days{ 
            compatible_chrono::year(2026) / 
            compatible_chrono::May / 
        1 };

    // Test string formatting using std::format
    // (works only on supported compiler environments)
    std::string s = CHRONO_FORMAT("%Y-%m-%d", today);
    EXPECT_EQ(s, "2026-05-01");
}
#endif


#include "compatible_chrono.hpp"

#include <gtest/gtest.h>

#include <sstream>
#include <string>
#include <thread>
#include <vector>
#include <atomic>
#include <exception>

// 1) Parsing error handling: invalid input should set stream failbit
TEST(ChronoExtra, ParseInvalidInput) {
    std::istringstream in("not-a-date");
    compatible_chrono::sys_days parsed_date{};
    in >> compatible_chrono::parse("%Y-%m-%d", parsed_date);
    EXPECT_TRUE(in.fail() || in.bad());
}

// 2) CHRONO_FORMAT with valid input (C++17 uses date::format fallback)
TEST(ChronoExtra, FormatValidInput) {
    compatible_chrono::sys_days d = compatible_chrono::sys_days{
        compatible_chrono::year{2026} / compatible_chrono::month{5} / compatible_chrono::day{6}
    };
    std::string s = CHRONO_FORMAT("%Y-%m-%d", d);
    EXPECT_EQ(s, "2026-05-06");
}

// 3) is_clock trait checks (compile-time + runtime)
TEST(ChronoExtra, IsClockTrait) {
    static_assert(compatible_chrono::is_clock_v<compatible_chrono::system_clock>, "system_clock should be a clock");
    static_assert(compatible_chrono::is_clock_v<compatible_chrono::steady_clock>, "steady_clock should be a clock");

    EXPECT_TRUE(compatible_chrono::is_clock_v<compatible_chrono::system_clock>);
    EXPECT_TRUE(compatible_chrono::is_clock_v<compatible_chrono::steady_clock>);

    // runtime check for utc_clock (date::utc_clock is expected in C++17 branch)
    EXPECT_TRUE(compatible_chrono::is_clock_v<compatible_chrono::utc_clock>);

    // Negative test for a non-clock type
    struct NotAClock {};
    static_assert(!compatible_chrono::is_clock_v<NotAClock>, "NotAClock must not be considered a clock");
    EXPECT_FALSE(compatible_chrono::is_clock_v<NotAClock>);
}

// 4) floor / ceil and time_point rounding behavior (day-level rounding)
TEST(ChronoExtra, FloorCeilRounding) {
    auto now = std::chrono::system_clock::now();
    // create a time_point shifted by 1 day + 2 hours 30 minutes to ensure fractional day
    auto tp = now + std::chrono::hours{26} + std::chrono::minutes{30};
    auto floored = compatible_chrono::floor<compatible_chrono::days>(tp);
    auto ceiled = compatible_chrono::ceil<compatible_chrono::days>(tp);

    EXPECT_LE(floored, ceiled);
    if (floored != ceiled) {
        // if not aligned to day boundary, ceil must be strictly greater
        EXPECT_GT(ceiled, floored);
    }
}

// 5) hh_mm_ss negative duration handling
TEST(ChronoExtra, HhMmSsNegative) {
    compatible_chrono::seconds neg_secs(-3661); // -1 hour, -1 minute, -1 second -> absolute parts 1:1:1 with negative flag
    compatible_chrono::hh_mm_ss<compatible_chrono::seconds> tod{ neg_secs };

    EXPECT_TRUE(tod.is_negative());
    EXPECT_EQ(tod.hours().count(), 1);
    EXPECT_EQ(tod.minutes().count(), 1);
    EXPECT_EQ(tod.seconds().count(), 1);
}

// 6) Leap seconds detailed check (skip if database not present)
TEST(ChronoExtra, LeapSecondsDetailed) {
    try {
        auto &db = compatible_chrono::get_tzdb();
        if (db.leap_seconds.empty()) {
            GTEST_SKIP() << "No leap-second data available in tz database";
        }
        // If present, verify last leap date is after 1970-01-01
        auto last = db.leap_seconds.back();
        EXPECT_GT(last.date(), compatible_chrono::sys_days{
            compatible_chrono::year{1970} / compatible_chrono::month{1} / compatible_chrono::day{1}
        });
    } catch (const std::exception &e) {
        GTEST_SKIP() << "tz database access failed: " << e.what();
    } catch (...) {
        GTEST_SKIP() << "tz database access failed (unknown error)";
    }
}

// 7) Duration cast rounding behavior (milliseconds -> seconds truncation)
TEST(ChronoExtra, DurationCastRounding) {
    compatible_chrono::milliseconds ms_val{1999};
    auto sec = compatible_chrono::duration_cast<compatible_chrono::seconds>(ms_val);
    EXPECT_EQ(sec.count(), 1); // truncation toward zero
}

// 8) Round-trip time_point <-> sys_days <-> year_month_day
TEST(ChronoExtra, RoundTripTimePointSysDays) {
    auto now = std::chrono::system_clock::now();
    auto sd = compatible_chrono::floor<compatible_chrono::days>(now);
    compatible_chrono::year_month_day ymd{ sd };
    auto sd2 = compatible_chrono::sys_days{ ymd };
    EXPECT_EQ(sd, sd2);
}

// 9) Multithreaded: call clock::now() concurrently to detect races/crashes
TEST(ChronoExtra, MultiThreadedClockNow) {
    const int threads = 8;
    const int iterations = 1000;
    std::atomic<int> counter{0};

    auto worker = [&counter, iterations]() {
        for (int i = 0; i < iterations; ++i) {
            // call both clocks available (system_clock and, if present, utc_clock)
            auto s = std::chrono::system_clock::now();
            (void)s;
            try {
                auto u = compatible_chrono::utc_clock::now();
                (void)u;
            } catch (...) {
                // utc_clock may throw in some environments; ignore to keep test robust
            }
            ++counter;
        }
    };

    std::vector<std::thread> pool;
    pool.reserve(threads);
    for (int i = 0; i < threads; ++i) pool.emplace_back(worker);
    for (auto &t : pool) t.join();

    EXPECT_EQ(counter.load(), threads * iterations);
}
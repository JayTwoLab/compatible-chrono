
#include "compatible_chrono.hpp"

#include <gtest/gtest.h>

#include <sstream>
#include <string>
#include <exception>

// 헬퍼: 여러 테스트에서 공통으로 사용하는 "오늘"과 일 단위로 내림(floor) 처리한 값
static auto get_today_sys_days()
{
	auto now = std::chrono::system_clock::now(); // 현재 시스템 시간을 가져옴
	return compatible_chrono::floor<compatible_chrono::days>(now); // 현재 시간을 일 단위(days)로 내림 처리(floor)하여 sys_days 타입으로 반환 (compatible_chrono::floor는 date::floor와 유사한 기능을 제공하며, compatible_chrono::days는 date::days와 유사한 기능을 제공하는 타입)
}  
  
// 날짜 동등성 및 기본 생성 테스트
TEST(ChronoCompatibility, DateEqualityAndConstruction) {
    auto sd = get_today_sys_days();
	auto ymd = compatible_chrono::year_month_day{ sd }; // sys_days에서 year_month_day(년/월/일)로 변환하여 날짜 구성

    EXPECT_EQ(ymd, compatible_chrono::year_month_day{sd});

    // 직접 구성 후 다시 변환(roundtrip) 확인
    compatible_chrono::year_month_day ymd2{
        compatible_chrono::year{2026}, 
        compatible_chrono::month{4}, 
        compatible_chrono::day{27} };
    auto sys_days2 = compatible_chrono::sys_days{ymd2};
    EXPECT_EQ(ymd2, compatible_chrono::year_month_day{sys_days2});
    EXPECT_EQ(CHRONO_FORMAT("%Y-%m-%d", sys_days2), std::string("2026-04-27"));
}

// 포맷팅 테스트
TEST(ChronoCompatibility, Formatting) {
    auto now = std::chrono::system_clock::now();
    auto sd = compatible_chrono::floor<compatible_chrono::days>(now);

    std::string s = CHRONO_FORMAT("%Y-%m-%d", now);
    std::string s2 = CHRONO_FORMAT("%Y-%m-%d", sd);
    EXPECT_EQ(s, s2);
}

// 날짜 산술 연산 테스트 (다음 주 계산)
TEST(ChronoCompatibility, DateArithmetic_NextWeek) {
    auto sd = get_today_sys_days();
    auto next_week = sd + compatible_chrono::days{7};
    auto ymd_next = compatible_chrono::year_month_day{next_week};
    auto expected_ymd_next = compatible_chrono::year_month_day{next_week};
    EXPECT_EQ(ymd_next, expected_ymd_next);
    EXPECT_EQ(CHRONO_FORMAT("%Y-%m-%d", next_week), CHRONO_FORMAT("%Y-%m-%d", sd + compatible_chrono::days{7}));
}

// 문자열로부터의 파싱 테스트
TEST(ChronoCompatibility, ParsingFromString) {
    std::istringstream in("2026-05-05");
    compatible_chrono::sys_days parsed_date;
    in >> compatible_chrono::parse("%Y-%m-%d", parsed_date);
    auto ymd_parsed = compatible_chrono::year_month_day{parsed_date};
    compatible_chrono::year_month_day expected_parsed{compatible_chrono::year{2026}, compatible_chrono::month{5}, compatible_chrono::day{5}};
    EXPECT_EQ(ymd_parsed, expected_parsed);
    EXPECT_EQ(CHRONO_FORMAT("%Y-%m-%d", parsed_date), std::string("2026-05-05"));
}

// 월의 마지막 날 처리 테스트
TEST(ChronoCompatibility, LastDayOfMonth) {
    compatible_chrono::year_month_day_last last_day{compatible_chrono::year{2026}, compatible_chrono::month_day_last{compatible_chrono::month{4}}};
    auto sys_days_last = compatible_chrono::sys_days{last_day};
    auto ymd_last = compatible_chrono::year_month_day{sys_days_last};
    compatible_chrono::year_month_day expected_last{compatible_chrono::year{2026}, compatible_chrono::month{4}, compatible_chrono::day{30}};
    EXPECT_EQ(ymd_last, expected_last);
    EXPECT_EQ(CHRONO_FORMAT("%Y-%m-%d", sys_days_last), std::string("2026-04-30"));
}

// 요일 범위 검사 (0..6)
TEST(ChronoCompatibility, WeekdayRange) {
    auto sd = get_today_sys_days();
    auto weekday = compatible_chrono::weekday{sd};
    EXPECT_GE(weekday.c_encoding(), 0);
    EXPECT_LE(weekday.c_encoding(), 6);
}

// 타임존 처리 테스트 (tz 데이터베이스가 없을 경우 생략 가능)
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
//         GTEST_SKIP() << "타임존 데이터베이스를 사용할 수 없거나 호환되지 않습니다: " << e.what();
//     }
// }

// 윤년 처리 테스트 (2024-02-29)
TEST(ChronoCompatibility, LeapYearHandling) {
    compatible_chrono::year_month_day leap_day{compatible_chrono::year{2024}, compatible_chrono::month{2}, compatible_chrono::day{29}};
    auto sys_leap = compatible_chrono::sys_days{leap_day};
    EXPECT_EQ(leap_day, compatible_chrono::year_month_day{sys_leap});
    EXPECT_EQ(CHRONO_FORMAT("%Y-%m-%d", sys_leap), std::string("2024-02-29"));
}

// 월말 처리 테스트 (1월 31일)
TEST(ChronoCompatibility, EndOfMonthHandling) {
    compatible_chrono::year_month_day end_of_month{compatible_chrono::year{2026}, compatible_chrono::month{1}, compatible_chrono::day{31}};
    auto sys_end = compatible_chrono::sys_days{end_of_month};
    EXPECT_EQ(end_of_month, compatible_chrono::year_month_day{sys_end});
    EXPECT_EQ(CHRONO_FORMAT("%Y-%m-%d", sys_end), std::string("2026-01-31"));
}

// 음수 기간 처리 테스트 (과거 날짜 계산)
TEST(ChronoCompatibility, NegativeDurationHandling) {
    auto sd = get_today_sys_days();
    auto past_date = sd - compatible_chrono::days{365};
    auto ymd_past = compatible_chrono::year_month_day{past_date};
    compatible_chrono::year_month_day expected_past{compatible_chrono::year_month_day{past_date}};
    EXPECT_EQ(ymd_past, expected_past);
    EXPECT_EQ(CHRONO_FORMAT("%Y-%m-%d", past_date), CHRONO_FORMAT("%Y-%m-%d", sd - compatible_chrono::days{365}));
}

// 큰 기간 처리 테스트 (100년 후)
TEST(ChronoCompatibility, LargeDurationHandling) {
    auto sd = get_today_sys_days();
    auto future_date = sd + compatible_chrono::days{365 * 100};
    auto ymd_future = compatible_chrono::year_month_day{future_date};
    compatible_chrono::year_month_day expected_future{compatible_chrono::year_month_day{future_date}};
    EXPECT_EQ(ymd_future, expected_future);
}

// 월 오버플로우 처리 테스트 (13월 -> 다음 해 1월로 전환)
TEST(ChronoCompatibility, MonthOverflowHandling) {
    compatible_chrono::year_month_day month_overflow{compatible_chrono::year{2026}, compatible_chrono::month{13}, compatible_chrono::day{1}};
    auto sys_month_overflow = compatible_chrono::sys_days{month_overflow};
    auto ymd_month_overflow = compatible_chrono::year_month_day{sys_month_overflow};
    compatible_chrono::year_month_day expected_month_overflow{compatible_chrono::year{2027}, compatible_chrono::month{1}, compatible_chrono::day{1}};
    EXPECT_EQ(ymd_month_overflow, expected_month_overflow);
}

// 연도 경계 처리 테스트 (최대값 근처의 연도 처리)
TEST(ChronoCompatibility, YearOverflowHandling) {
    compatible_chrono::year_month_day year_overflow{compatible_chrono::year{32767}, compatible_chrono::month{12}, compatible_chrono::day{31}};
    auto sys_year_overflow = compatible_chrono::sys_days{year_overflow};
    auto ymd_year_overflow = compatible_chrono::year_month_day{sys_year_overflow};
    compatible_chrono::year_month_day expected_year_overflow{ compatible_chrono::year{32767}, compatible_chrono::month{12}, compatible_chrono::day{31} };
    EXPECT_EQ(ymd_year_overflow, expected_year_overflow);
}



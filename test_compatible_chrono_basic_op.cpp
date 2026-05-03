#include "compatible_chrono.hpp"

#include <gtest/gtest.h>

#include <sstream>
#include <string>
#include <exception>

// 헬퍼: 여러 테스트에서 공통으로 사용하는 "오늘"과 일 단위로 내림(floor) 처리한 값
static auto get_today_sys_days()
{
	auto now = compatible_chrono::system_clock::now(); // 현재 시스템 시간(UTC)을 가져옴 

	return compatible_chrono::floor<compatible_chrono::days>(now); // 현재 시간을 일 단위(days)로 내림 처리(floor)하여 sys_days(시스템 시간 일 단위) 타입으로 반환 (compatible_chrono::floor는 date::floor와 유사한 기능을 제공하며, compatible_chrono::days는 date::days와 유사한 기능을 제공하는 타입)
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

	auto sys_days2 = compatible_chrono::sys_days{ ymd2 }; // year_month_day에서 sys_days로 변환하여 날짜 구성 (compatible_chrono::sys_days는 date::sys_days와 유사한 기능을 제공하는 타입)

    EXPECT_EQ(ymd2, compatible_chrono::year_month_day{sys_days2});
    EXPECT_EQ(CHRONO_FORMAT("%Y-%m-%d", sys_days2), std::string("2026-04-27"));
}

// 포맷팅 테스트
TEST(ChronoCompatibility, Formatting) {
    auto now = compatible_chrono::system_clock::now();
    auto sd = compatible_chrono::floor<compatible_chrono::days>(now);

    std::string s = CHRONO_FORMAT("%Y-%m-%d", now);
    std::string s2 = CHRONO_FORMAT("%Y-%m-%d", sd);
    EXPECT_EQ(s, s2);
}

// 날짜 산술 연산 테스트 (다음 주 계산)
TEST(ChronoCompatibility, DateArithmetic_NextWeek) {
    auto sd = get_today_sys_days();

	auto next_week = sd + compatible_chrono::days{ 7 }; // 7일 후 계산
	auto ymd_next = compatible_chrono::year_month_day{ next_week }; // sys_days에서 year_month_day로 변환하여 다음 주 날짜 구성

	auto expected_ymd_next = compatible_chrono::year_month_day{ next_week }; // 다음 주 날짜를 year_month_day로 변환하여 예상 결과 구성 (compatible_chrono::year_month_day는 date::year_month_day와 유사한 기능을 제공하는 타입)

    EXPECT_EQ(ymd_next, expected_ymd_next);
    EXPECT_EQ(CHRONO_FORMAT("%Y-%m-%d", next_week), CHRONO_FORMAT("%Y-%m-%d", sd + compatible_chrono::days{7}));
}

// 문자열로부터의 파싱 테스트
TEST(ChronoCompatibility, ParsingFromString) {
    std::istringstream in("2026-05-05");
	compatible_chrono::sys_days parsed_date; // 시스템 시간(UTC) 기준의 날자(Days) 타입의 변수 

	in >> compatible_chrono::parse("%Y-%m-%d", parsed_date); // 문자열 "2026-05-05"을 "%Y-%m-%d" 형식으로 파싱하여 sys_days 타입의 parsed_date에 저장 (compatible_chrono::parse는 date::parse와 유사한 기능을 제공하는 함수)

	auto ymd_parsed = compatible_chrono::year_month_day{ parsed_date }; // 파싱된 sys_days를 year_month_day로 변환하여 ymd_parsed에 저장 (compatible_chrono::year_month_day는 date::year_month_day와 유사한 기능을 제공하는 타입)
    compatible_chrono::year_month_day expected_parsed{
        compatible_chrono::year{2026}, 
        compatible_chrono::month{5}, 
        compatible_chrono::day{5} };

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

// --- local_days 관련 테스트들 ---

// local_days 기본 생성 및 year_month_day와의 roundtrip 테스트
TEST(ChronoCompatibility, LocalDaysConstructionAndRoundtrip) {
    compatible_chrono::year_month_day ymd{
        compatible_chrono::year{2026}, 
        compatible_chrono::month{5}, 
        compatible_chrono::day{3} };

	compatible_chrono::local_days ld{ ymd }; // year_month_day에서 local_days로 변환

	auto ymd_from_local = compatible_chrono::year_month_day{ ld }; // local_days에서 year_month_day로 다시 변환하여 원래 날짜와 일치하는지 확인

    EXPECT_EQ(ymd_from_local, ymd);
}

// local_days에서의 일 단위 산술 연산 테스트
TEST(ChronoCompatibility, LocalDaysArithmetic_NextDay) {
    compatible_chrono::year_month_day ymd{
        compatible_chrono::year{2026}, 
        compatible_chrono::month{12}, 
        compatible_chrono::day{31} };

	compatible_chrono::local_days ld{ ymd }; // year_month_day에서 local_days로 변환

	auto next_day = ld + compatible_chrono::days{ 1 }; // local_days에서 1일 더하기 (다음 날 계산)

	auto ymd_next = compatible_chrono::year_month_day{ next_day }; // 다음 날을 year_month_day로 변환하여 2027-01-01이 되는지 확인

    compatible_chrono::year_month_day expected_next{
        compatible_chrono::year{2027}, 
        compatible_chrono::month{1}, 
        compatible_chrono::day{1} };

    EXPECT_EQ(ymd_next, expected_next);
}

// 타임존 정보가 사용 가능한 경우: sys_days <-> local_days 라운드트립 확인 (불가하면 테스트를 건너뜀)
#if __cplusplus < 202002L
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
TEST(ChronoCompatibility, LocalDaysTimezoneRoundtrip_WhenTimezoneAvailable) {
    GTEST_SKIP() << "Timezone roundtrip test skipped in C++20/std::chrono build on this platform.";
}
#endif



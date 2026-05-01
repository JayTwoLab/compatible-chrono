
#include "compatible_chrono.hpp"

#include <gtest/gtest.h>

#include <ratio>
#include <limits>
#include <sstream>
#include <iomanip>

#if __cplusplus >= 202002L 
#  include <format>
#endif 
   
// 1. Ratio (시간 단위의 기반이 되는 비율) 테스트
TEST(ChronoTest, RatioCheck) {
    // std::micro (1/1,000,000) 등 기본 비율 확인
    EXPECT_EQ(std::micro::num, 1);
    EXPECT_EQ(std::micro::den, 1000000);
}

// 2. Duration Trait 및 특수 연산
TEST(ChronoTest, DurationAdvanced) {
	using micro_seconds = std::chrono::duration<long long, std::micro>; // c++11 style duration with microsecond precision
    micro_seconds ms(1000000);

    // duration::min, max, zero 테스트 — C++17 호환 방식으로 검사
    EXPECT_EQ(micro_seconds(0).count(), 0);
    EXPECT_LT(micro_seconds(std::numeric_limits<long long>::min()).count(), 0); 

    // abs (manual, C++17-safe)
    std::chrono::seconds neg_sec(-5); 
	auto abs_sec = (neg_sec < std::chrono::seconds::zero()) ? -neg_sec : neg_sec; // C++17에서는 std::chrono::abs가 없으므로 수동으로 절댓값 계산
    EXPECT_EQ(abs_sec.count(), 5);
}

// 3. 다양한 Clock 타입 테스트 (C++20 포함)
#if __cplusplus >= 202002L
	// C++20에서는 std::chrono::utc_clock, std::chrono::tai_clock, std::chrono::gps_clock 등이 도입됨
TEST(ChronoTest, SpecializedClocks) {
    // gps_clock: GPS 시간 (1980년 기준)
    // tai_clock: 국제 원자시 (윤초 포함)
    // utc_clock: 협정 세계시

    auto utc_now = compatible_chrono::utc_clock::now();
    auto sys_now = compatible_chrono::utc_clock::to_sys(utc_now);

    EXPECT_GT(sys_now.time_since_epoch().count(), 0);
}
#else
TEST(ChronoTest, SpecializedClocks) {
    // C++17 
    try {
        auto utc_now = compatible_chrono::utc_clock::now(); // date::utc_clock를 이용하여 c++20의 std::chrono::utc_clock과 유사한 기능을 제공하는 date::utc_clock에서 현재 시간을 가져옴
        auto sys_now = compatible_chrono::utc_clock::to_sys(utc_now); // date::utc_clock의 시간을 시스템 시간으로 변환하여 c++20의 std::chrono::system_clock과 유사한 기능을 제공하는 date::sys_clock으로 변환

		auto count = sys_now.time_since_epoch().count(); // 시스템 시간으로 변환된 시간에서 epoch 이후의 시간을 count로 가져옴 (c++17에서는 std::chrono::time_point의 time_since_epoch()가 duration을 반환하므로, duration의 count()를 이용하여 정수형으로 변환)
        EXPECT_GT(count, 0);

    } catch( const std::exception& e) {
        std::string err = e.what();
        FAIL() << "Exception occurred: " << err; // Timezone database not found at ...
    } catch (...) {
        FAIL() << "Unknown exception occurred";
	}
}
#endif

// 4. Calendar - 세부 구성 요소 (Year, Month, Day, Weekday, Monthday 등)
#if __cplusplus >= 202002L
TEST(ChronoTest, CalendarComponents) {
    compatible_chrono::year y{ 2026 };
    compatible_chrono::month m{ compatible_chrono::May };
    compatible_chrono::day d{ 1 };

    // 연도 관련 함수
    EXPECT_FALSE(y.is_leap()); // 2026년은 윤년이 아님

    // 월/일 연산
    auto next_month = m + compatible_chrono::months(1);
	EXPECT_EQ(next_month, compatible_chrono::June); // 5월 다음은 6월

    // Last day of month (해당 월의 마지막 날)
    compatible_chrono::year_month_day_last ymdl{ y, compatible_chrono::month_day_last{m} };
    EXPECT_EQ(ymdl.day(), compatible_chrono::day(31)); // 5월의 마지막 날은 31일
}
#else
TEST(ChronoTest, CalendarComponents) {
    GTEST_SKIP() << "Calendar components require C++20 chrono/calendar support";
}
#endif

// 5. hh_mm_ss (Time of Day - C++20)
#if __cplusplus >= 202002L
TEST(ChronoTest, TimeOfDay) {
    compatible_chrono::seconds total_secs(3661); // 1시간 1분 1초
    compatible_chrono::hh_mm_ss tod{ total_secs };

    EXPECT_EQ(tod.hours().count(), 1);
    EXPECT_EQ(tod.minutes().count(), 1);
    EXPECT_EQ(tod.seconds().count(), 1);
    EXPECT_FALSE(tod.is_negative());
}
#else
TEST(ChronoTest, TimeOfDay) {
    GTEST_SKIP() << "hh_mm_ss requires C++20 chrono support";
}
#endif

// 6. Time Zone / Leap Seconds (윤초)
#if __cplusplus >= 202002L
TEST(ChronoTest, LeapSeconds) {
    // 시스템에 등록된 윤초 리스트 확인
    auto& db = compatible_chrono::get_tzdb();
    if (!db.leap_seconds.empty()) {
        auto last_leap = db.leap_seconds.back();
        EXPECT_GT(last_leap.date(), compatible_chrono::sys_days{ compatible_chrono::year(1970) / 1 / 1 });
    }
}
#else
TEST(ChronoTest, LeapSeconds) {
    GTEST_SKIP() << "Time zone / leap second database requires C++20 tz support";
}
#endif

// 7. Parsing & Formatting (C++20)
#if __cplusplus >= 202002L
TEST(ChronoTest, Formatting) {
    compatible_chrono::sys_days today = compatible_chrono::sys_days{ compatible_chrono::year(2026) / compatible_chrono::May / 1 };

    // std::format을 이용한 문자열 변환 테스트
    // (지원되는 컴파일러 환경에서만 작동)
    std::string s = std::format("{:%Y-%m-%d}", today);
    EXPECT_EQ(s, "2026-05-01");
}
#else
TEST(ChronoTest, Formatting) {
    // Fallback: skip test on C++17 to avoid dependency on std::format/calendar formatting
    GTEST_SKIP() << "Formatting with std::format requires C++20";
}
#endif


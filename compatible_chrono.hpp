#pragma once

#if __cplusplus >= 202002L
    // C++20 and later: using standard chrono

    #include <chrono> // c++20 chrono library header
    #include <format> // c++20 format library header

    // Prefer Howard Hinnant's tz if available even in C++20 (vcpkg usually provides it)
    #if defined(__has_include)
        #if __has_include(<date/tz.h>)
            #include <date/tz.h>
            #define COMPAT_HAS_DATE_TZ 1
        # else
            #define COMPAT_HAS_DATE_TZ 0
        #endif
    #else
        #define COMPAT_HAS_DATE_TZ 0
    #endif

    // Expose a compat namespace that mirrors std::chrono identifiers so we can add helpers.
    namespace compatible_chrono {
        // Pull commonly used chrono types into the compat namespace
        using namespace std::chrono;

        // Calendar/time utilities introduced in C++20
        using std::chrono::year;
        using std::chrono::month;
        using std::chrono::day;
        using std::chrono::year_month_day;
        using std::chrono::hh_mm_ss;
        using std::chrono::weekday;
        using std::chrono::sys_days;
        using std::chrono::local_days;
        using std::chrono::utc_clock;
        using std::chrono::floor;
        using std::chrono::ceil;
        using std::chrono::days;
        using std::chrono::months;
        using std::chrono::years;
    
        using local_time_t = decltype(date::current_zone()->to_local(std::declval<system_clock::time_point>()));
    
        // local_now() returns the canonical local_time_t
        inline local_time_t local_now() {
            auto now = system_clock::now();

            try {
                return date::current_zone()->to_local(now); // returns local_time_t
            } catch (...) {
                // Construct local_time_t from epoch-duration as a safe fallback
                return local_time_t{};
            }
		} // local_now()

    } // namespace compatible_chrono

    // C++20 supports output with std::format
    #define CHRONO_FORMAT(fmt, tp) std::format("{:" fmt "}", tp)

#else
    // C++17 (Less than C++20): Using Howard Hinnant's date library

    #include <date/date.h> // Howard Hinnant's date library header
    #include <date/tz.h>  // time zone support (optional, include if needed)
    #include <date/iso_week.h> // ISO 8601 week date support (year, week number, weekday)

    // Unused calendar systems (comment out if not needed)
    // #include <date/julian.h> // Julian calendar support
    // #include <date/islamic.h> // Islamic calendar support
    // #include <date/solar_hijri.h> // Solar Hijri calendar support

    //////////////////////////////////////////////////
    // namespace compatible_chrono = date; // Alias for date library namespace

    namespace compatible_chrono {
 
        ////////////////////////////////////////////
        // std::chrono standard durations / utilities
        // Not provided by date::, import from std::chrono

        // date:: does not provide hours, minutes, seconds, milliseconds, so import from std::chrono
        using std::chrono::hours; // duration representing hours
        using std::chrono::minutes; // duration representing minutes
        using std::chrono::seconds; // duration representing seconds
        using std::chrono::milliseconds; // duration representing milliseconds
        using std::chrono::microseconds; // duration representing microseconds
        using std::chrono::nanoseconds; // duration representing nanoseconds

        // Additional common std::chrono types/utilities (previously missing)
        using std::chrono::duration; // duration template type
        using std::chrono::duration_cast; // utility to convert durations
        using std::chrono::time_point_cast; // utility to cast time_points
        using std::chrono::high_resolution_clock; // high-resolution clock

        // date:: does not provide system_clock and steady_clock, so import from std::chrono
        using std::chrono::system_clock; // system clock (real-world time)
        using std::chrono::steady_clock; // steady clock (monotonic)

        // date:: does not provide time_point, so import from std::chrono
        using std::chrono::time_point;

        // Import floor/ceil from std::chrono for rounding durations (e.g., floor<days>(tp))
        using std::chrono::floor; // round down duration/time_point to specified unit
        using std::chrono::ceil; // round up duration/time_point to specified unit

        ////////////////////////////////////////////
        // date library durations/types/extensions
        
        using date::years; // duration representing years
        using date::months; // duration representing months
        using date::days; // duration representing days

        using date::weekday; // weekday (Sunday=0, Monday=1, ..., Saturday=6)

        using date::year_month_day; // year/month/day (e.g., 2024-03-15)
        using date::year_month_day_last; // year/month/last_day (e.g., last day of Feb)
        using date::month_day_last; // month/last_day (28/29/30/31 depending on month)
        using date::month_day; // month/day (e.g., March 15)
        using date::hh_mm_ss; // hours:minutes:seconds helper

        using date::sys_days; // : UTC(시스템 시계 기준)의 날짜를 나타내는 타입(내부적으로 time_point<system_clock, days>의 별칭). 날짜의 0시(00:00)는 UTC 기준이다.
        using date::local_days; // 로컬(지역) 시각의 날짜를 나타내는 타입(내부적으로 time_point<local_t, days>의 별칭). 날짜의 0시는 로컬 시각 기준이다.

        using date::utc_clock; // UTC clock

        using date::get_tzdb; // access time zone database (e.g., get_tzdb().zones)
        using date::parse; // parse string to date/time (e.g., parse("%Y-%m-%d", "2024-03-15", ymd))
        using date::format; // format date/time to string (e.g., format("%Y-%m-%d", ymd))

        using date::year; // year type (e.g., 2024)
        using date::month; // month type (e.g., March)
        using date::day; // day type (e.g., 15)

        using date::January; // 1st month
        using date::February; // 2nd month
        using date::March; // 3rd month
        using date::April; // 4th month
        using date::May; // 5th month
        using date::June; // 6th month
        using date::July; // 7th month
        using date::August; // 8th month
        using date::September; // 9th month
        using date::October; // 10th month
        using date::November; // 11th month
        using date::December; // 12th month

        // Provide a fallback is_clock trait for C++17 (C++20 has std::chrono::is_clock)
        template <typename T, typename = void>
        struct is_clock : std::false_type {};

        // A type is considered a clock if it has a static now() function and a nested time_point type
        template <typename T>
        struct is_clock<T, std::void_t<decltype(T::now()), typename T::time_point>> : std::true_type {};

        // Helper variable template for is_clock trait
        template <typename T>
        inline constexpr bool is_clock_v = is_clock<T>::value;

        // canonical local_time_t (date::tz is available in this branch)
        using local_time_t = decltype(date::current_zone()->to_local(std::declval<system_clock::time_point>()));

        inline local_time_t local_now() {
            auto now = system_clock::now();
            try {
                return date::current_zone()->to_local(now);
            } catch (...) {
                return local_time_t{ now.time_since_epoch() };
            }
        }
    }

    //////////////////////////////////////////////////

    // Using the format function of date.h
    #define CHRONO_FORMAT(fmt, tp) date::format(fmt, tp)

#endif


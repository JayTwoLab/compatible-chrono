#pragma once

#if __cplusplus >= 202002L
    // C++20 and later: using standard chrono

    #include <chrono> // c++20 chrono library header
    #include <format> // c++20 format library header

    namespace compatible_chrono = std::chrono; // Alias for std::chrono
    
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
        // std::chrono standard durations / utilities
        using std::chrono::seconds;
        using std::chrono::minutes;
        using std::chrono::hours;
        using std::chrono::milliseconds;
        using std::chrono::system_clock;
        using std::chrono::steady_clock;
        using std::chrono::time_point;
        using std::chrono::floor; // bring chrono::floor into namespace for rounding
        using std::chrono::ceil;

        // date library durations/types/extensions
        using date::days;
        using date::months;
        using date::years;
        using date::weekday;
        using date::year_month_day;
        using date::year_month_day_last;
        using date::month_day_last;
        using date::month_day;
        using date::sys_days;
        using date::hh_mm_ss;
        using date::utc_clock;
        using date::get_tzdb;
        using date::local_days; // if used
        using date::parse;
        using date::format;

        using date::year;
        using date::month;
        using date::day;
        using date::May;
        using date::June;

        // Provide a fallback is_clock trait for C++17 (C++20 has std::chrono::is_clock)
        template <typename T, typename = void>
        struct is_clock : std::false_type {};

        template <typename T>
        struct is_clock<T, std::void_t<decltype(T::now()), typename T::time_point>> : std::true_type {};

        template <typename T>
        inline constexpr bool is_clock_v = is_clock<T>::value;
    }

    //////////////////////////////////////////////////

    // Using the format function of date.h
    #define CHRONO_FORMAT(fmt, tp) date::format(fmt, tp)

#endif


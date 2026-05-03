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
		using date::month_day_last; // month/last_day
		using date::month_day; // month/day (e.g., March 15)

		using date::sys_days; // time_point expressed in days since epoch (system time)
		using date::hh_mm_ss; // hours:minutes:seconds helper
		using date::utc_clock; // UTC clock
		using date::get_tzdb; // access time zone database (e.g., get_tzdb().zones)
		using date::local_days; // time_point in local time expressed in days
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
    }

    //////////////////////////////////////////////////

    // Using the format function of date.h
    #define CHRONO_FORMAT(fmt, tp) date::format(fmt, tp)

#endif


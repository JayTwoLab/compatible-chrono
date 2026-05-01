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

    namespace compatible_chrono = date; // Alias for date library namespace

    // Using the format function of date.h
    #define CHRONO_FORMAT(fmt, tp) date::format(fmt, tp)

#endif


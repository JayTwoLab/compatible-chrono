#include "compatible_chrono.hpp"

#include <gtest/gtest.h>

#include <filesystem>
#include <optional>
#include <string>
#include <cstdlib>
#include <iostream>
#include <type_traits>
#include <utility>
#include <algorithm>

#if defined(_WIN32)
#  include <Windows.h>
#endif

static bool set_env_var(const std::string& name, const std::string& value) {
#ifdef _WIN32
    return _putenv_s(name.c_str(), value.c_str()) == 0;
#else
    return setenv(name.c_str(), value.c_str(), 1) == 0;
#endif
}

static std::optional<std::filesystem::path> find_upwards_tzdata(std::filesystem::path start, int max_up = 8) {
    namespace fs = std::filesystem;
    fs::path p = start;
    for (int i = 0; i <= max_up; ++i) {
        fs::path candidate = p / "tzdata";
        if (fs::exists(candidate) && fs::is_directory(candidate))
            return fs::canonical(candidate);
        if (p.has_parent_path())
            p = p.parent_path();
        else
            break;
    }
    return std::nullopt;
}

static std::optional<std::filesystem::path> get_executable_dir() {
    namespace fs = std::filesystem;
#if defined(_WIN32)
    char buf[MAX_PATH];
    DWORD len = GetModuleFileNameA(NULL, buf, MAX_PATH);
    if (len == 0 || len == MAX_PATH) return std::nullopt;
    fs::path p(buf);
    return p.parent_path();
#else
    try {
        fs::path proc = "/proc/self/exe";
        if (fs::exists(proc)) {
            auto exe = fs::read_symlink(proc);
            return exe.parent_path();
        }
    } catch (...) {
    }
    return std::nullopt;
#endif
}

// Produce a native/preferred string for a filesystem::path.
// On Windows this yields backslashes.
static std::string preferred_path_string(std::filesystem::path p) {
    p.make_preferred();
    return p.string();
}

// SFINAE-detect compatible_chrono::set_install(const std::string&)
template<typename = void>
struct has_set_install : std::false_type {};

template<>
struct has_set_install<std::void_t<decltype(compatible_chrono::set_install(std::declval<const std::string&>()))>>
    : std::true_type {};

// attempt to call compatible_chrono::set_install if available (no-op otherwise)
static void attempt_set_install(const std::filesystem::path& path) {
#if __cplusplus < 202002L
    if constexpr (has_set_install<>::value) {
        try {
            std::string p = preferred_path_string(path);
            compatible_chrono::set_install(p);
        } catch (...) {
            // ignore failures; tests will handle missing tzdb gracefully
        }
    }
#else
    (void)path;
#endif
}

int main(int argc, char** argv) {
    // 1) Respect existing TZDIR / TZDATA
    if (std::getenv("TZDIR") || std::getenv("TZDATA")) {
        ::testing::InitGoogleTest(&argc, argv);
        return RUN_ALL_TESTS();
    }

    // 2) If CMake injected PROJECT_SOURCE_DIR_TZDATA, prefer it
#ifdef PROJECT_SOURCE_DIR_TZDATA
    {
        std::string cmake_tz = PROJECT_SOURCE_DIR_TZDATA;
        if (!cmake_tz.empty()) {
            std::filesystem::path p(cmake_tz);
            std::string pref = preferred_path_string(p);
            set_env_var("TZDIR", pref);
            set_env_var("TZDATA", pref);
            attempt_set_install(p);
            ::testing::InitGoogleTest(&argc, argv);
            return RUN_ALL_TESTS();
        }
    }
#endif

    namespace fs = std::filesystem;

    // 3) Try executable directory
    if (auto exe_dir = get_executable_dir()) {
        if (auto found = find_upwards_tzdata(*exe_dir)) {
            std::string pref = preferred_path_string(*found);
            set_env_var("TZDIR", pref);
            set_env_var("TZDATA", pref);
            attempt_set_install(*found);
            ::testing::InitGoogleTest(&argc, argv);
            return RUN_ALL_TESTS();
        }
    }

    // 4) Try argv[0] candidate (useful if argv[0] contains path)
    if (argc > 0 && argv[0]) {
        try {
            fs::path p(argv[0]);
            if (!p.is_absolute()) p = fs::current_path() / p;
            if (auto found = find_upwards_tzdata(p.parent_path())) {
                std::string pref = preferred_path_string(*found);
                set_env_var("TZDIR", pref);
                set_env_var("TZDATA", pref);
                attempt_set_install(*found);
                ::testing::InitGoogleTest(&argc, argv);
                return RUN_ALL_TESTS();
            }
        } catch (...) {}
    }

    // 5) Finally try current working directory upward
    try {
        if (auto found = find_upwards_tzdata(fs::current_path())) {
            std::string pref = preferred_path_string(*found);
            set_env_var("TZDIR", pref);
            set_env_var("TZDATA", pref);
            attempt_set_install(*found);
        } else {
            std::cerr << "tzdata not found under exe/src/current tree; tests may fail if tzdb required.\n";
        }
    } catch (...) {
        // ignore filesystem exceptions; let tests run and possibly skip on failures
    }

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

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
    #include <Windows.h>
#endif
  
  
// Set environment variables in a platform-specific way.
// Use _putenv_s on Windows, setenv on POSIX
static bool set_env_var(const std::string& name, const std::string& value) {
#ifdef _WIN32
    return _putenv_s(name.c_str(), value.c_str()) == 0;
#else
    return setenv(name.c_str(), value.c_str(), 1) == 0;
#endif
}

// Search upwards from the start directory for a 'tzdata' directory.
// Return the canonical path if found.
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

// Get the executable's directory (branching for Windows and POSIX).
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

// Convert and return the filesystem path to the platform-native format (backslashes on Windows).
// This ensures consistent path formatting when passing paths to environment variables or date::set_install.
static std::string preferred_path_string(std::filesystem::path p) {
    p.make_preferred();
    return p.string();
}

#if defined(_WIN32) && (__cplusplus < 202002L)
// Call date::set_install only for Windows + C++17 builds
// compatible_chrono.hpp includes date/tz.h under C++17, so if date::set_install exists it can be used directly.
static void attempt_set_install(const std::filesystem::path& path) {
    try {
        std::string p = preferred_path_string(path);
        // Register tzdata path explicitly with the date library
        date::set_install(p);
    } catch (...) {
        // Silently ignore failures (environment variables may still work)
    }
}
#else
// No-op on other environments
static void attempt_set_install(const std::filesystem::path& /*path*/) {
    // no-op
}
#endif

// Check whether the tzdata path is a valid directory, then set environment variables and call set_install.
// Return true on success.
static bool configure_tz_from_path(const std::filesystem::path& tzdir) {
    try {
        if (!std::filesystem::exists(tzdir) || !std::filesystem::is_directory(tzdir))
            return false;
        std::string pref = preferred_path_string(tzdir);
        if (!set_env_var("TZDIR", pref)) return false;
        if (!set_env_var("TZDATA", pref)) return false;
        attempt_set_install(tzdir);
        return true;
    } catch (...) {
        return false;
    }
}

// Search upwards from the start path for tzdata and configure it if found.
static bool try_set_from_candidate(const std::filesystem::path& start) {
    if (auto found = find_upwards_tzdata(start)) {
        return configure_tz_from_path(*found);
    }
    return false;
}

int main(int argc, char** argv) {

#if __cplusplus >= 202002L
	std::cout << "Running with C++20 chrono support" << std::endl;
#else 
	std::cout << "Running with C++17 chrono support" << std::endl;
#endif

    // If TZDIR/TZDATA are already set, run tests as-is
    if (std::getenv("TZDIR") || std::getenv("TZDATA")) {
        ::testing::InitGoogleTest(&argc, argv);
        return RUN_ALL_TESTS();
    }

    // If "CMake" injected "PROJECT_SOURCE_DIR_TZDATA, prefer it first
#ifdef PROJECT_SOURCE_DIR_TZDATA
    {
        std::filesystem::path p(PROJECT_SOURCE_DIR_TZDATA);
        if (configure_tz_from_path(p)) {
            ::testing::InitGoogleTest(&argc, argv);
            return RUN_ALL_TESTS();
        }
    }
#endif

    // Try to find and set tzdata based on the executable's directory
    if (auto exe_dir = get_executable_dir()) {
        if (try_set_from_candidate(*exe_dir)) {
            ::testing::InitGoogleTest(&argc, argv);
            return RUN_ALL_TESTS();
        }
    }

    // Try to find and set tzdata based on argv[0] path (useful when argv[0] contains a path)
    if (argc > 0 && argv[0]) {
        namespace fs = std::filesystem;
        try {
            fs::path p(argv[0]);
            if (!p.is_absolute()) p = fs::current_path() / p;
            if (try_set_from_candidate(p.parent_path())) {
                ::testing::InitGoogleTest(&argc, argv);
                return RUN_ALL_TESTS();
            }
        } catch (...) {
			// Ignore filesystem exceptions and proceed with tests
        }
    }

    // Search upwards from the current working directory for tzdata and configure it
    try {
        namespace fs = std::filesystem;
        if (try_set_from_candidate(fs::current_path())) {
            ::testing::InitGoogleTest(&argc, argv);
            return RUN_ALL_TESTS();
        } else {
            std::cerr << "tzdata not found under exe/src/current tree; tests may fail if tzdb required.\n";
        }
    } catch (...) { 
        // Ignore filesystem exceptions and proceed with tests
    }

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

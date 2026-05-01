
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



// 환경변수 설정을 플랫폼별로 수행한다.
// Windows: _putenv_s, POSIX: setenv 사용
static bool set_env_var(const std::string& name, const std::string& value) {
#ifdef _WIN32
    return _putenv_s(name.c_str(), value.c_str()) == 0;
#else
    return setenv(name.c_str(), value.c_str(), 1) == 0;
#endif
}

// 시작 디렉터리에서 상위 방향으로 'tzdata' 디렉터리를 탐색해 찾는다.
// 발견 시 canonical 경로를 반환한다.
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

// 실행 파일의 디렉터리를 얻는다 (Windows와 POSIX용 분기).
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

// 파일시스템 경로를 플랫폼 네이티브 형식(Windows에서는 역슬래시)으로 변환하여 반환한다.
// 이를 통해 환경변수나 date::set_install 등에 전달되는 경로 표기를 일관되게 한다.
static std::string preferred_path_string(std::filesystem::path p) {
    p.make_preferred();
    return p.string();
}

#if __cplusplus < 202002L
// Detect presence of date::set_install(const std::string&) using SFINAE.
// This avoids hard compile-time dependency when the API is not available.
template<typename = void>
struct has_date_set_install : std::false_type {};

template<>
struct has_date_set_install<std::void_t<decltype(date::set_install(std::declval<const std::string&>()))>> : std::true_type {};
#endif

// attempt_set_install: call library-specific installation when available (C++17/date).
static void attempt_set_install(const std::filesystem::path& path) {
#if __cplusplus < 202002L
    if constexpr (has_date_set_install<>::value) {
        try {
            std::string p = preferred_path_string(path);
            // Call the date library hook to register the tzdata location.
            date::set_install(p);
        } catch (...) {
            // ignore errors; fallback is environment variables
        }
    } else {
        (void)path; // API not available; no-op
    }
#else
    (void)path;
#endif
}

// tzdata 경로가 유효한 디렉터리인지 검사하고, 환경변수 설정 및 set_install 호출을 수행한다.
// 성공하면 true 반환.
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

// 시작 경로에서 상위 탐색으로 tzdata를 찾고, 찾으면 구성한다.
static bool try_set_from_candidate(const std::filesystem::path& start) {
    if (auto found = find_upwards_tzdata(start)) {
        return configure_tz_from_path(*found);
    }
    return false;
}

int main(int argc, char** argv) {
    // 우선 이미 설정된 TZDIR/TZDATA가 있으면 그대로 테스트 실행
    if (std::getenv("TZDIR") || std::getenv("TZDATA")) {
        ::testing::InitGoogleTest(&argc, argv);
        return RUN_ALL_TESTS();
    }

    // CMake가 PROJECT_SOURCE_DIR_TZDATA를 주입했다면 우선 사용
#ifdef PROJECT_SOURCE_DIR_TZDATA
    {
        std::filesystem::path p(PROJECT_SOURCE_DIR_TZDATA);
        if (configure_tz_from_path(p)) {
            ::testing::InitGoogleTest(&argc, argv);
            return RUN_ALL_TESTS();
        }
    }
#endif

    namespace fs = std::filesystem;

    // 실행 파일 위치를 기준으로 tzdata 탐색 및 설정 시도
    if (auto exe_dir = get_executable_dir()) {
        if (try_set_from_candidate(*exe_dir)) {
            ::testing::InitGoogleTest(&argc, argv);
            return RUN_ALL_TESTS();
        }
    }

    // argv[0] 경로를 기준으로 tzdata 탐색 및 설정 시도 (argv[0]이 경로를 포함하는 경우 유용)
    if (argc > 0 && argv[0]) {
        try {
            fs::path p(argv[0]);
            if (!p.is_absolute()) p = fs::current_path() / p;
            if (try_set_from_candidate(p.parent_path())) {
                ::testing::InitGoogleTest(&argc, argv);
                return RUN_ALL_TESTS();
            }
        } catch (...) {}
    }

    // 현재 작업 디렉터리에서 상위로 탐색하여 tzdata를 찾고 설정
    try {
        if (try_set_from_candidate(fs::current_path())) {
            ::testing::InitGoogleTest(&argc, argv);
            return RUN_ALL_TESTS();
        } else {
            std::cerr << "tzdata not found under exe/src/current tree; tests may fail if tzdb required.\n";
        }
    } catch (...) {
        // 파일시스템 예외는 무시하고 테스트 진행
    }

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

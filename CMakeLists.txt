cmake_minimum_required(VERSION 3.16)

if(DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
    message(STATUS "Using vcpkg toolchain file: ${CMAKE_TOOLCHAIN_FILE}")

    if (WIN32)
        message("Windows vcpkg")

        set(VCPKG_TARGET_TRIPLET "x64-windows-static") # 테스트 편의를 위해 정적만 사용
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

        # using x64-windows triplet 
        set(date_DIR  "$ENV{VCPKG_ROOT}/installed/x64-windows-static/share/date")
        set(gtest_DIR "$ENV{VCPKG_ROOT}/installed/x64-windows-static/share/gtest")
    elseif(APPLE)
        message("macOS vcpkg")
        # TODO: ... 
    elseif(UNIX)
        message("Linux or Unix vcpkg") 

        set(VCPKG_TARGET_TRIPLET "x64-linux")

        set(date_DIR  "$ENV{VCPKG_ROOT}/installed/x64-linux/share/date")
        set(gtest_DIR "$ENV{VCPKG_ROOT}/installed/x64-linux/share/gtest")
    endif()	
endif() 

project(compatible_chrono_test)

# C++ 표준 설정 (사용자가 -DCMAKE_CXX_STANDARD=17 또는 20으로 선택)
if(NOT CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 17)
endif()

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CXX_EXTENSIONS OFF)

# gtest

if(WIN32)
    message("Windows")
    find_package(gtest CONFIG REQUIRED) # 윈도우에서 gt가 소문자인 경우 있음
elseif(APPLE)
    message("macOS")
    # TODO: ... 
elseif(UNIX)
    message("Linux or Unix")
    find_package(GTest CONFIG REQUIRED) # 리눅스에서 GT가 대문자인 경우 있음 
endif()

# 실행 파일
file(GLOB SOURCES "*.cpp")
add_executable(test_compatible_chrono ${SOURCES})

# include 디렉토리 설정
target_include_directories(test_compatible_chrono PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# C++17 / C++20 분기 처리
if(CMAKE_CXX_STANDARD EQUAL 17)
    message(STATUS "Using C++17 - date library required (via vcpkg)")

    # vcpkg로 설치된 date 라이브러리
    find_package(date CONFIG REQUIRED)

    target_link_libraries(test_compatible_chrono PRIVATE
        date::date
        date::date-tz
    )

elseif(CMAKE_CXX_STANDARD GREATER_EQUAL 20)
    message(STATUS "Using C++20 - std::chrono")
    
    # Even with C++20, the tz database/implementation from the date library is required for timezone APIs.
    # Find and link the date package so the tz implementation is available.
    find_package(date CONFIG REQUIRED)

    target_link_libraries(test_compatible_chrono PRIVATE
        date::date
        date::date-tz
    )

    # target_compile_features(test_compatible_chrono PRIVATE cxx_std_20)
    # target_link_libraries(test_compatible_chrono PRIVATE stdc++exp)

endif()

# gtest 링크
target_link_libraries(test_compatible_chrono PRIVATE
    GTest::gtest_main
)

# 테스트 등록
enable_testing()
add_test(NAME chrono_test COMMAND test_compatible_chrono)

# 설정: 프로젝트 루트의 tzdata 폴더를 사용
set(TZDATA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/tzdata")

if(EXISTS "${TZDATA_DIR}")
    message(STATUS "Using tzdata from: ${TZDATA_DIR}")

    # 1) CMake/CTest 프로세스 환경변수 (런타임에 CTest로 실행될 때 적용)
    # NOTE: set(ENV{...})는 CMake/CTest 프로세스에만 적용됩니다.
    set(ENV{TZDIR} "${TZDATA_DIR}")
    set(ENV{TZDATA} "${TZDATA_DIR}")

    # 2) 각 테스트 등록 시 테스트에 환경변수 전달 (예: 테스트가 add_test 로 등록된 경우)
    # 예시: 단일 테스트 실행 파일 'test_compatible_chrono' 가 있을 때
    if(TARGET test_compatible_chrono)
        # 테스트로 등록되어 있지 않다면 add_test 를 사용해 등록하세요:
        add_test(NAME test_compatible_chrono COMMAND test_compatible_chrono)

        # CTest 실행 시 ENVIRONMENT 설정
        set_tests_properties(test_compatible_chrono PROPERTIES
            ENVIRONMENT "TZDIR=${TZDATA_DIR};TZDATA=${TZDATA_DIR}"
        )

        # Visual Studio 디버거에서 실행할 때 환경변수로 전달 (VS-specific)
        set_target_properties(test_compatible_chrono PROPERTIES
            VS_DEBUGGER_ENVIRONMENT "TZDIR=${TZDATA_DIR};TZDATA=${TZDATA_DIR}"
        )
    endif()

    # 3) 여러 테스트 타겟에 적용하는 예시 (타겟 리스트를 채워 사용)
    set(TEST_TARGETS test_compatible_chrono) # 추가 테스트 이름을 공백으로 구분하여 넣으세요
    foreach(t ${TEST_TARGETS})
        if(TARGET ${t})
            set_target_properties(${t} PROPERTIES
                VS_DEBUGGER_ENVIRONMENT "TZDIR=${TZDATA_DIR};TZDATA=${TZDATA_DIR}"
            )
            # 등록된 테스트가 이미 있다면 set_tests_properties 로 ENVIRONMENT 갱신 가능
            set_tests_properties(${t} PROPERTIES
                ENVIRONMENT "TZDIR=${TZDATA_DIR};TZDATA=${TZDATA_DIR}"
            )
        endif()
    endforeach()
else()
    message(WARNING "tzdata directory not found at ${TZDATA_DIR}; timezone DB tests may fail.")
endif()

target_compile_definitions(test_compatible_chrono PRIVATE PROJECT_SOURCE_DIR_TZDATA="${CMAKE_CURRENT_SOURCE_DIR}/tzdata")




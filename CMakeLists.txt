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
add_executable(test_compatible_chrono
    test_compatible_chrono_basic_op.cpp
)

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
    
    # find_package(date CONFIG REQUIRED) # C++20에서도 date 라이브러리가 필요한 경우가 있을 수 있으므로 일단 찾기는 함.
    #target_link_libraries(test_compatible_chrono PRIVATE
    #    date::date  # C++20에서도 date 라이브러리가 필요한 경우가 있을 수 있으므로 일단 링크는 함.
    #)	

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




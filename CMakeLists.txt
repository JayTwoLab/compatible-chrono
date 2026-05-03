cmake_minimum_required(VERSION 3.16)

# vcpkg toolchain 파일 설정 (환경변수 VCPKG_ROOT이 정의된 경우)
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
    # set(CMAKE_CXX_STANDARD 20)   
endif()

set(CMAKE_CXX_STANDARD_REQUIRED ON) # C++ 표준이 반드시 충족되어야 함을 명시
set(CXX_EXTENSIONS OFF) # C++ 확장 기능을 비활성화하여 표준 준수 모드로 컴파일하도록 설정 (예: MSVC의 경우 /Za 옵션과 유사)

# gtest
if(WIN32)
    message("Windows")
    find_package(gtest CONFIG REQUIRED) # 윈도우에서 gt가 소문자인 경우 있음
elseif(APPLE)
    message("macOS")
    # TODO: 추후 설정
elseif(UNIX)
    message("Linux or Unix")
    find_package(GTest CONFIG REQUIRED) # 리눅스에서 GT가 대문자인 경우 있음 
endif()


file(GLOB SOURCES "*.cpp") # 현재 디렉토리의 모든 .cpp 파일을 SOURCES 변수에 저장
add_executable(test_compatible_chrono ${SOURCES}) # 실행 파일
 
 
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

    if(MSVC)
        # Visual Studio 2019 이상에서 C++20 chrono의 timezone 기능을 사용하려면 /Zc:__cplusplus 옵션이 필요
        # __cplusplus는 C++ 표준 버전을 나타내는 매크로로, C++20에서는 202002L 이상의 값을 가집니다. 이 옵션이 없으면 MSVC는 __cplusplus를 199711L로 정의하여 C++17로 인식하게 됩니다.
        target_compile_options(test_compatible_chrono  PRIVATE /Zc:__cplusplus)
    endif()
    
    find_package(date CONFIG REQUIRED)

    target_link_libraries(test_compatible_chrono PRIVATE
        date::date
        date::date-tz
    )

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

# tzdata 디렉토리가 존재하는 경우, 환경변수로 전달하여 테스트에서 사용할 수 있도록 설정
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

target_compile_definitions(test_compatible_chrono PRIVATE
    "PROJECT_SOURCE_DIR_TZDATA=\"${CMAKE_CURRENT_SOURCE_DIR}/tzdata\""
)

# 테스트에서 date::set_install()을 호출하여 tzdata를 사용할지 여부를 설정하는 옵션
option(USE_DATE_SET_INSTALL "Call date::set_install() at runtime when tzdata provided" OFF)

# 테스트 코드에서 USE_DATE_SET_INSTALL이 정의된 경우, date::set_install()을 호출하여 tzdata를 사용할 수 있도록 설정
if(USE_DATE_SET_INSTALL)
    target_compile_definitions(test_compatible_chrono PRIVATE USE_DATE_SET_INSTALL)
endif()



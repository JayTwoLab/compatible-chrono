cmake_minimum_required(VERSION 3.16)

# Set vcpkg toolchain file (when environment variable VCPKG_ROOT is defined)
if(DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
    message(STATUS "Using vcpkg toolchain file: ${CMAKE_TOOLCHAIN_FILE}")

    if (WIN32)
        message("Windows vcpkg")

        set(VCPKG_TARGET_TRIPLET "x64-windows-static") # Use static only for convenience of testing
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

# C++ standard setting (user may choose -DCMAKE_CXX_STANDARD=17 or 20)
if(NOT CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 17)  
    # set(CMAKE_CXX_STANDARD 20)     
endif()

set(CMAKE_CXX_STANDARD_REQUIRED ON) # Require C++ standard to be met
set(CXX_EXTENSIONS OFF) # Disable C++ extensions to compile in standards-compliant mode (e.g., similar to /Za on MSVC)

# gtest
if(WIN32)
    message("Windows")
    find_package(gtest CONFIG REQUIRED) # On Windows gtest may be lowercase 'gtest'
elseif(APPLE)
    message("macOS")
    # TODO: configure later
elseif(UNIX)
    message("Linux or Unix")
    find_package(GTest CONFIG REQUIRED) # On Linux GTest may be uppercase 'GTest'
endif()


file(GLOB SOURCES "*.cpp") # Store all .cpp files in the current directory in SOURCES variable
add_executable(test_compatible_chrono ${SOURCES}) # Executable
 
 
# Set include directories
target_include_directories(test_compatible_chrono PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# C++17 / C++20 branching
if(CMAKE_CXX_STANDARD EQUAL 17)
    message(STATUS "Using C++17 - date library required (via vcpkg)")

    # date library installed via vcpkg
    find_package(date CONFIG REQUIRED)

    target_link_libraries(test_compatible_chrono PRIVATE
        date::date
        date::date-tz
    )

elseif(CMAKE_CXX_STANDARD GREATER_EQUAL 20)
    message(STATUS "Using C++20 - std::chrono")

    if(MSVC)
        # On Visual Studio 2019+ the /Zc:__cplusplus option is required to use timezone features of C++20 chrono
        # The __cplusplus macro indicates the C++ standard version; for C++20 it has a value >= 202002L.
        # If this option is not present MSVC defines __cplusplus as 199711L, causing it to be recognized as C++17.
        target_compile_options(test_compatible_chrono  PRIVATE /Zc:__cplusplus)
    endif()
    
    find_package(date CONFIG REQUIRED)

    target_link_libraries(test_compatible_chrono PRIVATE
        date::date
        date::date-tz
    )

endif()

# Link gtest
target_link_libraries(test_compatible_chrono PRIVATE
    GTest::gtest_main
)

# Register tests
enable_testing()
add_test(NAME chrono_test COMMAND test_compatible_chrono)

# Configuration: use tzdata folder in project root
set(TZDATA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/tzdata")

# If tzdata directory exists, pass it via environment variables so tests can use it
if(EXISTS "${TZDATA_DIR}")
    message(STATUS "Using tzdata from: ${TZDATA_DIR}")

    # 1) CMake/CTest process environment variables (applies when run by CTest at runtime)
    # NOTE: set(ENV{...}) only applies to the CMake/CTest process.
    set(ENV{TZDIR} "${TZDATA_DIR}")
    set(ENV{TZDATA} "${TZDATA_DIR}")

    # 2) Pass environment variables to each registered test (e.g., when tests are registered with add_test)
    # Example: for a single test executable 'test_compatible_chrono'
    if(TARGET test_compatible_chrono)
        # If not registered as a test, register it with add_test:
        add_test(NAME test_compatible_chrono COMMAND test_compatible_chrono)

        # Set ENVIRONMENT when running CTest
        set_tests_properties(test_compatible_chrono PROPERTIES
            ENVIRONMENT "TZDIR=${TZDATA_DIR};TZDATA=${TZDATA_DIR}"
        )

        # Pass environment variables when running under Visual Studio debugger (VS-specific)
        set_target_properties(test_compatible_chrono PROPERTIES
            VS_DEBUGGER_ENVIRONMENT "TZDIR=${TZDATA_DIR};TZDATA=${TZDATA_DIR}"
        )
    endif()

    # 3) Example applying to multiple test targets (populate the target list)
    set(TEST_TARGETS test_compatible_chrono) # Add additional test names separated by spaces
    foreach(t ${TEST_TARGETS})
        if(TARGET ${t})
            set_target_properties(${t} PROPERTIES
                VS_DEBUGGER_ENVIRONMENT "TZDIR=${TZDATA_DIR};TZDATA=${TZDATA_DIR}"
            )
            # If tests are already registered, you can update ENVIRONMENT via set_tests_properties
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

# Option to control whether tests call date::set_install() to use tzdata
option(USE_DATE_SET_INSTALL "Call date::set_install() at runtime when tzdata provided" OFF)

# If USE_DATE_SET_INSTALL is defined in test code, call date::set_install() to enable tzdata
if(USE_DATE_SET_INSTALL)
    target_compile_definitions(test_compatible_chrono PRIVATE USE_DATE_SET_INSTALL)
endif()



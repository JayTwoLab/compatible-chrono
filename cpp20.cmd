
@REM build gtest
rmdir /s /q build-cpp20
cmake -B build-cpp20 -S . -DCMAKE_CXX_STANDARD=20
cmake --build build-cpp20

@REM run gtest
.\build-cpp20\Debug\test_compatible_chrono



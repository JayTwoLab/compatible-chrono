@REM build gtest
rmdir /s /q build-cpp17
cmake -B build-cpp17 -S . -DCMAKE_CXX_STANDARD=17
cmake --build build-cpp17

@REM run gtest
build-cpp17\Debug\test_compatible_chrono


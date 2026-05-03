# build gtest
rm -rf build-cpp20
cmake -B build-cpp20 -S . -DCMAKE_CXX_STANDARD=20
cmake --build build-cpp20

# execute gtest
./build-cpp20/test_compatible_chrono


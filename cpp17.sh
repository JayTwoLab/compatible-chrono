# build gtest 
rm -rf build-cpp17
cmake -B build-cpp17 -S . -DCMAKE_CXX_STANDARD=17
cmake --build build-cpp17

# execute gtest
./build-cpp17/test_compatible_chrono


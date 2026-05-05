# compatible-chrono

A C++ header-only library providing a compatibility layer for `<chrono>` features across different C++ standards (C++17, C++20, etc). This project aims to make it easier to write portable time and date code that works seamlessly regardless of the C++ standard version.

## Features
- Compatibility wrappers for `<chrono>` features
- Utilities for working with clocks, time points, and durations
- Support for local time and time zone data (see `tzdata/`)
- Header-only: just include `compatible_chrono.hpp`

## Getting Started
1. Clone this repository.
2. Include `compatible_chrono.hpp` in your project.
3. Build your project with your desired C++ standard (C++17, C++20, etc).

Example build (C++17):
```sh
# On Windows
cpp17.cmd
# On Unix
./cpp17.sh
```

## Files
- `compatible_chrono.hpp`: Main header file
- `main.cpp`: Example usage
- `test_*.cpp`: Unit tests and examples
- `tzdata/`: Time zone data files

## License
This project is licensed under the MIT License. See the LICENSE file for details.

## Acknowledgements
- Based on the C++ standard library `<chrono>`
- Includes time zone data from the IANA Time Zone Database

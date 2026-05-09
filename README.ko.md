# compatible-chrono

> [English](README.md)

C++ 표준(C++17, C++20 등) 간 `<chrono>` 기능의 호환성을 제공하는 헤더 전용 라이브러리입니다. 이 프로젝트는 다양한 C++ 표준 버전에서 시간 및 날짜 코드를 이식성 있게 작성할 수 있도록 돕는 것을 목표로 합니다.

## 주요 특징
- `<chrono>` 기능에 대한 호환성 래퍼 제공
- 시계, 시간점, 기간 관련 유틸리티
- 로컬 시간 및 타임존 데이터 지원(`tzdata/` 참고)
- 헤더 전용: `compatible_chrono.hpp`만 포함하면 사용 가능

## 시작하기
1. 이 저장소를 클론합니다.
2. 프로젝트에 `compatible_chrono.hpp`를 포함합니다.
3. 원하는 C++ 표준(C++17, C++20 등)으로 프로젝트를 빌드합니다.

예시 빌드(C++17):
```sh
# Windows
cpp17.cmd
# Unix
./cpp17.sh
```

## 파일 구성
- `compatible_chrono.hpp`: 메인 헤더 파일
- `main.cpp`: 사용 예제
- `test_*.cpp`: 단위 테스트 및 예제
- `tzdata/`: 타임존 데이터 파일

## 라이선스
이 프로젝트는 MIT 라이선스로 배포됩니다. 자세한 내용은 LICENSE 파일을 참고하세요.

## 참고
- C++ 표준 라이브러리 `<chrono>` 기반
- IANA Time Zone Database의 타임존 데이터 포함

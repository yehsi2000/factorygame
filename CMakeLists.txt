cmake_minimum_required(VERSION 3.10)
project(FactoryGame)

# C++17 사용
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 헤더(.h) 파일들이 있는 경로
include_directories(${PROJECT_SOURCE_DIR}/include)

# 소스 파일 리스트 (필요한 만큼 추가)
set(SOURCES
    src/Entity.cpp
    src/Event.cpp
    src/GameState.cpp
    src/Inventory.cpp
    src/main.cpp
    src/Player.cpp
    src/RefineryComponent.cpp
    src/ResourceNodeComponent.cpp
    src/World.cpp
)

# 실행 파일 생성
add_executable(FactoryGame ${SOURCES})

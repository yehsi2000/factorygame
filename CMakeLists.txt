cmake_minimum_required(VERSION 3.11)
project(FactoryGame)

set(CMAKE_CXX_STANDARD 20)

find_package(SDL2 REQUIRED)

# 실행 파일 생성
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
include_directories(${SDL2_INCLUDE_DIRS})

add_executable(FactoryGame ${SRC_FILES})

target_include_directories(FactoryGame PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(FactoryGame ${SDL2_LIBRARIES})
cmake_minimum_required(VERSION 3.30)
project(FactoryGame)

set(CMAKE_CXX_STANDARD 20)

find_package(SDL2 REQUIRED)
find_package(Boost REQUIRED)

# 실행 파일 생성
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_executable(FactoryGame ${SRC_FILES})

target_include_directories(FactoryGame PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_include_directories(FactoryGame PRIVATE
    ${SDL2_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
)

target_link_libraries(FactoryGame PRIVATE
    ${SDL2_LIBRARIES}
)


cmake_minimum_required(VERSION 3.30)
project(FactoryGame)

set(CMAKE_CXX_STANDARD 20)

set(BUILD_STATIC ON)

if(BUILD_STATIC)
    set(BUILD_SHARED_LIBS OFF)
    set(SDL_SHARED OFF)
    set(SDL_STATIC ON)
else()
    set(BUILD_SHARED_LIBS ON)
    set(SDL_SHARED ON)
    set(SDL_STATIC OFF)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
#set(CMAKE_CXX_INCLUDE_WHAT_YOU_USE "include-what-you-use;-Xiwyu;--error_always;-Xiwyu;--no_fwd_decls;")

add_subdirectory(vendored/SDL EXCLUDE_FROM_ALL)
add_subdirectory(vendored/SDL_image EXCLUDE_FROM_ALL)

find_package(Boost REQUIRED)

# 실행 파일 생성
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_executable(FactoryGame ${SRC_FILES})

target_include_directories(FactoryGame PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_include_directories(FactoryGame PRIVATE
    ${Boost_INCLUDE_DIRS}
)

if(TARGET SDL2::SDL2main)
    target_link_libraries(FactoryGame PRIVATE SDL2::SDL2main)
endif()

if(BUILD_STATIC)
    target_link_libraries(FactoryGame PRIVATE SDL2_image::SDL2_image-static SDL2::SDL2-static)
else()
    target_link_libraries(FactoryGame PRIVATE SDL2_image::SDL2_image SDL2::SDL2)
endif()

file(COPY ${CMAKE_SOURCE_DIR}/assets DESTINATION ${CMAKE_BINARY_DIR}/build)
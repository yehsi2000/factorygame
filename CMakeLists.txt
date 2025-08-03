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

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_PDB_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/pdbs")
#set(CMAKE_CXX_INCLUDE_WHAT_YOU_USE "include-what-you-use;-Xiwyu;--error_always;-Xiwyu;--no_fwd_decls;")

add_subdirectory(vendored/SDL EXCLUDE_FROM_ALL)
add_subdirectory(vendored/SDL_image EXCLUDE_FROM_ALL)
add_subdirectory(vendored/SDL_ttf EXCLUDE_FROM_ALL)


find_package(Boost REQUIRED)
find_package(easy_profiler REQUIRED PATHS vendored/easy_profiler/lib/cmake/easy_profiler)

# 실행 파일 생성
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_executable(FactoryGame ${SRC_FILES})

set_target_properties(FactoryGame PROPERTIES PDB_NAME "factorygame_debug_info")

target_include_directories(FactoryGame PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/Lib"
    ${Boost_INCLUDE_DIRS}
)

if(TARGET SDL2::SDL2main)
    target_link_libraries(FactoryGame PRIVATE SDL2::SDL2main)
endif()

if(BUILD_STATIC)
    target_link_libraries(FactoryGame PRIVATE SDL2_ttf::SDL2_ttf-static SDL2_image::SDL2_image-static SDL2::SDL2-static)
else()
    target_link_libraries(FactoryGame PRIVATE SDL2_ttf::SDL2_ttf SDL2_image::SDL2_image SDL2::SDL2)
endif()

target_link_libraries(FactoryGame PRIVATE easy_profiler)

#SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/PROFILE")

#file(COPY ${CMAKE_SOURCE_DIR}/assets DESTINATION ${CMAKE_BINARY_DIR}/build)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD 
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/assets" 
    "$<TARGET_FILE_DIR:${PROJECT_NAME}>/assets"
    COMMENT "Copying assets next to executable"
    VERBATIM
)
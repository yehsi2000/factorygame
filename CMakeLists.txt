cmake_minimum_required(VERSION 3.30)
project(FactoryGame)

set(CMAKE_CXX_STANDARD 23)

set(BUILD_STATIC ON)

if(BUILD_STATIC)
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build static library" FORCE)
    set(SDL_SHARED OFF)
    set(SDL_STATIC ON)
    set(SDL2IMAGE_DEPS_SHARED OFF)
else()
    set(BUILD_SHARED_LIBS ON CACHE BOOL "Build shared library" FORCE)
    set(SDL_SHARED ON)
    set(SDL_STATIC OFF)
endif()

set(VERBOSE true)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(SDL2TTF_VENDORED ON)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_PDB_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
#set(CMAKE_CXX_INCLUDE_WHAT_YOU_USE "include-what-you-use;-Xiwyu;--error_always;-Xiwyu;--no_fwd_decls;")

add_subdirectory(vendored/SDL EXCLUDE_FROM_ALL)
add_subdirectory(vendored/SDL_image EXCLUDE_FROM_ALL)
add_subdirectory(vendored/SDL_ttf EXCLUDE_FROM_ALL)


find_package(Boost REQUIRED)

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_executable(FactoryGame ${SRC_FILES})

set_target_properties(FactoryGame PROPERTIES PDB_NAME "FactoryGame")

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

#SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/PROFILE")

cmake_policy(SET CMP0077 NEW)

add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

if(MSVC)
    target_compile_options(FactoryGame PRIVATE "/wd4819;")
endif()

#file(COPY ${CMAKE_SOURCE_DIR}/assets DESTINATION ${CMAKE_BINARY_DIR}/build)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD 
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/assets" 
    "$<TARGET_FILE_DIR:${PROJECT_NAME}>/assets"
    COMMENT "Copying assets next to executable"
    VERBATIM
)
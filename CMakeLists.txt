cmake_minimum_required(VERSION 3.30)
project(FactoryGame)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Debug)
set(SDL2TTF_VENDORED ON)

# --- Static/Shared Build Configuration ---
set(BUILD_STATIC ON)
if(BUILD_STATIC)
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build static library" FORCE)
else()
    set(BUILD_SHARED_LIBS ON CACHE BOOL "Build shared library" FORCE)
endif()

if(MSVC)
    set(MSVC_WARNINGS "/wd4819") # Disable specific warning
endif()

find_package(Boost REQUIRED)

add_subdirectory(vendored/SDL)
add_subdirectory(vendored/SDL_image)
add_subdirectory(vendored/SDL_ttf)

file(GLOB IMGUI_SOURCES "src/lib/*.cpp")
add_library(imgui_lib STATIC ${IMGUI_SOURCES})
target_include_directories(imgui_lib PUBLIC 
    "${CMAKE_CURRENT_SOURCE_DIR}/include/Lib"
    "${CMAKE_CURRENT_SOURCE_DIR}/vendored/SDL/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/vendored/SDL_image/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/vendored/SDL_ttf"
    "${CMAKE_CURRENT_SOURCE_DIR}/vendored/SDL_ttf/external/freetype/include"
)
if(MSVC)
    target_compile_options(imgui_lib PRIVATE ${MSVC_WARNINGS})
endif()

file(GLOB_RECURSE GAME_LIB_SOURCES CONFIGURE_DEPENDS
    "src/Core/*.cpp"
    "src/GameState/*.cpp"
    "src/System/*.cpp"
    "src/Util/*.cpp"
)
add_library(FactoryGameLib ${GAME_LIB_SOURCES})

target_include_directories(FactoryGameLib PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/vendored/SDL/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/vendored/SDL_image/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/vendored/SDL_ttf"
    "${CMAKE_CURRENT_SOURCE_DIR}/vendored/SDL_ttf/external/freetype/include"
    ${Boost_INCLUDE_DIRS}
)

if(MSVC)
    target_compile_options(FactoryGameLib PUBLIC ${MSVC_WARNINGS})
endif()

# Link dependencies to the core library
if(TARGET SDL2::SDL2main)
    set(SDL_LIBS "")
    list(APPEND SDL_LIBS SDL2::SDL2main)
endif()

if(BUILD_STATIC)
    list(APPEND SDL_LIBS SDL2_ttf::SDL2_ttf-static SDL2_image::SDL2_image-static SDL2::SDL2-static)
else()
    list(APPEND SDL_LIBS SDL2_ttf::SDL2_ttf SDL2_image::SDL2_image SDL2::SDL2)
endif()

target_link_libraries(FactoryGameLib PUBLIC
    imgui_lib
    ${SDL_LIBS}
)

# --- Main Executable ---
add_executable(FactoryGame src/main.cpp)
target_link_libraries(FactoryGame PRIVATE FactoryGameLib)

# Copy assets to build directory
add_custom_command(TARGET FactoryGame POST_BUILD 
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/assets" 
    "$<TARGET_FILE_DIR:FactoryGame>/assets"
    COMMENT "Copying assets next to executable"
    VERBATIM
)

# --- Testing ---
enable_testing()
include(CTest)
add_subdirectory(tests)



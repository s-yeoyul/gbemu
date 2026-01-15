cmake_minimum_required(VERSION 3.20)
project(gbemu LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(gbemu
	src/main.cpp
	src/bus.cpp
	src/cpu.cpp
	src/timer.cpp
	src/ppu.cpp
	src/joypad.cpp
 )

target_include_directories(gbemu PRIVATE include)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)

target_include_directories(gbemu PRIVATE ${SDL2_INCLUDE_DIRS})
target_link_directories(gbemu PRIVATE ${SDL2_LIBRARY_DIRS})
target_link_libraries(gbemu PRIVATE ${SDL2_LIBRARIES})
target_compile_options(gbemu PRIVATE ${SDL2_CFLAGS_OTHER})

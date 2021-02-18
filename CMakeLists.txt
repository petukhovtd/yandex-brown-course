cmake_minimum_required(VERSION 3.16)
project(yandex_brown_course)

set(CMAKE_CXX_STANDARD 20)

add_compile_options( -W )
add_compile_options( -Wall )
#add_compile_options( -Werror )
add_compile_options( -Wpedantic )

add_executable(yandex_brown_course print_stats_2.cpp)

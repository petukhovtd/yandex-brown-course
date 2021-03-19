cmake_minimum_required(VERSION 3.13)
project(yandex_brown_course)

set(CMAKE_CXX_STANDARD 20)

add_compile_options( -W )
add_compile_options( -Wall )
#add_compile_options( -Werror )
add_compile_options( -Wpedantic )

add_executable(yandex_brown_course budget_professional_desktop_version.cpp)
target_link_libraries(yandex_brown_course pthread)

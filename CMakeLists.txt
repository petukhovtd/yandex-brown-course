cmake_minimum_required(VERSION 3.13)
project(yandex_brown_course)

set(CMAKE_CXX_STANDARD 20)

add_compile_options( -W )
add_compile_options( -Wall )
#add_compile_options( -Werror )
add_compile_options( -Wpedantic )

add_executable(yandex_brown_course
          stop.cpp
          bus.cpp
          transport.cpp
          transport_e.cpp
          json.cpp)
target_link_libraries(yandex_brown_course pthread)

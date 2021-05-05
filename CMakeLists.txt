cmake_minimum_required(VERSION 3.5)
project( hornybox )

set(CMAKE_CXX_FLAGS "-O3 -Wall -Wextra")

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

find_package( OpenCV REQUIRED )
find_library(wiringPi_LIB wiringPi)
include_directories(${CURSES_INCLUDE_DIR})
add_executable(hornybox main.cpp )
target_link_libraries( hornybox ${OpenCV_LIBS} ${wiringPi_LIB} Threads::Threads)

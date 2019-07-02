cmake_minimum_required(VERSION 3.14)
project(tcpipstack_cpp)

set(CMAKE_CXX_STANDARD 17)

add_executable(tcpipstack_cpp src/main.cpp src/tap.cpp include/tap.h)
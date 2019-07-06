cmake_minimum_required(VERSION 3.14)
project(tcpipstack_cpp)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

add_executable(tcpipstack_cpp
        src/main.cpp
        src/tap.cpp
        src/buffer.cpp
        src/arp.cpp
        src/ipv4.cpp
        src/icmp.cpp
        src/udp.cpp
        src/tcp.cpp
        src/utility.cpp

        include/packets.h
        include/tap.h
        include/ethernet.h
        include/buffer.h
        include/arp.h
        include/utility.h
        include/ipv4.h
        include/icmp.h
        include/udp.h
        include/tcp.h)

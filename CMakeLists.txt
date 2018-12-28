cmake_minimum_required(VERSION 3.9)
project(Projekt_TS_1_serwer)

set(CMAKE_CXX_STANDARD 17)

add_executable(Serwer serwer.cpp)
add_executable(Klient klient.cpp)

if(WIN32)
    target_link_libraries(Serwer wsock32 ws2_32)
    target_link_libraries(Klient wsock32 ws2_32)
endif()
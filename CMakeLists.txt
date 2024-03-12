cmake_minimum_required(VERSION 3.27)
project(untitled)

set(CMAKE_CXX_STANDARD 20)

add_executable(WebClientCoroutines main.cpp
        WebClient.hpp
        RequestAwaitable.hpp)

target_link_libraries(WebClientCoroutines curl)

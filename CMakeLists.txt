cmake_minimum_required(VERSION 3.0)
project(checker)

set(CMAKE_CXX_STANDARD 14)

include_directories(include)

add_executable(checker
        src/main.cpp src/agent.cpp src/checker.cpp )

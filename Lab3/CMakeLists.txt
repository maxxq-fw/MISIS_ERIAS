cmake_minimum_required(VERSION 3.10)
project(Lab3 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(Lab3 src/main.cpp)
target_include_directories(Lab3 PRIVATE include)
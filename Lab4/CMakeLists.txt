cmake_minimum_required(VERSION 3.10)
project(Lab4 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(Lab4 src/main.cpp)
target_include_directories(Lab4 PRIVATE include)
if(MSVC)
    target_compile_options(Lab4 PRIVATE /utf-8)
endif()
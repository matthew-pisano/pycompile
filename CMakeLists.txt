cmake_minimum_required(VERSION 4.1)
project(pycompile)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_compile_options(-Wall -Wextra -pedantic)

include_directories(include)

add_executable(pycompile src/main.cpp)

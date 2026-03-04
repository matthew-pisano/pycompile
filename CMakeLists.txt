cmake_minimum_required(VERSION 4.1)
project(pycompile)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

include(FetchContent)
FetchContent_Declare(
        cli11
        GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
        GIT_TAG main
)
FetchContent_MakeAvailable(cli11)

add_compile_options(-Wall -Wextra -pedantic)

include_directories(include)

find_package(Python REQUIRED COMPONENTS Interpreter Development)

add_executable(pycompile src/main.cpp
        src/bytecode.cpp
        src/utils.cpp
        src/pythoncode.cpp
        src/python_opcodes.cpp)
target_link_libraries(pycompile PRIVATE Python::Python CLI11::CLI11)

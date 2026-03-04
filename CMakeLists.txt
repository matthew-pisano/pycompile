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

set(PYCOMPILE_SOURCES
        src/bytecode.cpp
        src/utils.cpp
        src/pythoncode.cpp
        src/python_opcodes.cpp)

add_executable(pycompile src/main.cpp ${PYCOMPILE_SOURCES})
target_link_libraries(pycompile PRIVATE Python::Python CLI11::CLI11)


FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.13.0
)
FetchContent_MakeAvailable(Catch2)

add_executable(pycompile-tests
        ${PYCOMPILE_SOURCES}
        test/test_bytecode.cpp
        test/test_main.cpp
)
target_link_libraries(pycompile-tests PRIVATE Python::Python Catch2::Catch2WithMain)

include(CTest)
include(Catch)
catch_discover_tests(pycompile-tests WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")


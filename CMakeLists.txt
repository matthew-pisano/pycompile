cmake_minimum_required(VERSION 3.22)
project(pycompile)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_compile_options(-Wall -Wextra -pedantic)
include_directories(include)

# ================
# Configure Dependencies
# ================

include(FetchContent)
FetchContent_Declare(
        cli11
        GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
        GIT_TAG main
)
FetchContent_MakeAvailable(cli11)

find_package(Python 3.14...<3.15 REQUIRED COMPONENTS Interpreter Development)

find_package(MLIR REQUIRED CONFIG)
if (MLIR_VERSION VERSION_LESS "21.0.0" OR MLIR_VERSION VERSION_GREATER_EQUAL "22.0.0")
    message(FATAL_ERROR "MLIR 21 is required, found ${MLIR_VERSION}")
endif ()

list(APPEND CMAKE_MODULE_PATH "${MLIR_CMAKE_DIR}")
list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(TableGen)
include(AddMLIR)
include(AddLLVM)

add_subdirectory(src)
add_subdirectory(test)

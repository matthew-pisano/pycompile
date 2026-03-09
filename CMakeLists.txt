cmake_minimum_required(VERSION 4.1)
project(pycompile)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)


# Function to extract value from JSON
function(get_json_value JSON_FILE KEY OUTPUT_VAR)
    file(READ ${JSON_FILE} JSON_CONTENT)

    # Simple regex to extract the value for the given key
    string(REGEX MATCH "\"${KEY}\"[ ]*:[ ]*\"([^\"]+)\"" MATCH_RESULT ${JSON_CONTENT})
    if (MATCH_RESULT)
        set(${OUTPUT_VAR} ${CMAKE_MATCH_1} PARENT_SCOPE)
    else ()
        message(FATAL_ERROR "Could not find key '${KEY}' in ${JSON_FILE}")
    endif ()
endfunction()

# Read version from JSON
get_json_value("${CMAKE_CURRENT_SOURCE_DIR}/version.json" "version" PROJECT_VERSION)
message(STATUS "Build ${PROJECT_NAME} version: ${PROJECT_VERSION}")

# Configure the version header
configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/version.h.in"
        "${CMAKE_CURRENT_BINARY_DIR}/version.h"
        @ONLY
)


include(FetchContent)
FetchContent_Declare(
        cli11
        GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
        GIT_TAG main
)
FetchContent_MakeAvailable(cli11)

add_compile_options(-Wall -Wextra -pedantic)

include_directories(include)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

find_package(MLIR REQUIRED CONFIG)

list(APPEND CMAKE_MODULE_PATH "${MLIR_CMAKE_DIR}")
include(AddMLIR)
include(AddLLVM)

set(LLVM_TARGET_DEFINITIONS include/pyir/pyir.td)

# Each of these calls mlir-tblgen with different flags and
# creates a target you can depend on
mlir_tablegen(pyir.h.inc -gen-dialect-decls EXTRA_INCLUDES ${MLIR_INCLUDE_DIRS})
mlir_tablegen(pyir.cpp.inc -gen-dialect-defs EXTRA_INCLUDES ${MLIR_INCLUDE_DIRS})
mlir_tablegen(pyir_ops.h.inc -gen-op-decls EXTRA_INCLUDES ${MLIR_INCLUDE_DIRS})
mlir_tablegen(pyir_ops.cpp.inc -gen-op-defs EXTRA_INCLUDES ${MLIR_INCLUDE_DIRS})
mlir_tablegen(pyir_types.h.inc -gen-typedef-decls EXTRA_INCLUDES ${MLIR_INCLUDE_DIRS})
mlir_tablegen(pyir_types.cpp.inc -gen-typedef-defs EXTRA_INCLUDES ${MLIR_INCLUDE_DIRS})

# Collects all the above into a single target: pyirIncGen
add_public_tablegen_target(pyirIncGen)

# The actual compiled library
add_mlir_library(pyir
        src/pyir/pyir.cpp
        src/pyir/pyir_ops.cpp
        src/pyir/pyir_types.cpp

        DEPENDS
        pyirIncGen   # ensures .inc files exist before compiling

        LINK_LIBS PUBLIC
        MLIRIR
        MLIRSupport
)


find_package(Python REQUIRED COMPONENTS Interpreter Development)

set(PYCOMPILE_SOURCES
        src/bytecode/bytecode.cpp
        src/utils.cpp
        src/bytecode/pythoncode.cpp
        src/bytecode/python_opcodes.cpp
        src/bytecode/python_error.cpp
        src/pyir/pyir_codegen.cpp
        src/pyir/pyir_runtime.cpp
)

set(PYCOMPILE_LIBS
        Python::Python
        pyir
        MLIRFuncDialect
        MLIRControlFlowDialect
        MLIRArithDialect
        MLIRLLVMDialect
        MLIRTransforms
)

add_executable(pycompile src/main.cpp ${PYCOMPILE_SOURCES})
target_link_libraries(pycompile PRIVATE ${PYCOMPILE_LIBS} CLI11::CLI11)


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
        test/test_mlir_codegen.cpp
)
target_link_libraries(pycompile-tests PRIVATE ${PYCOMPILE_LIBS} Catch2::Catch2WithMain)

include(CTest)
include(Catch)
catch_discover_tests(pycompile-tests WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

cmake_minimum_required(VERSION 3.22)
project(pycompile)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_compile_options(-Wall -Wextra -pedantic)
include_directories(include)

# ================
# Versioning
# ================

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

# ================
# Fetch Main Packages
# ================

include(FetchContent)
FetchContent_Declare(
        cli11
        GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
        GIT_TAG main
)
FetchContent_MakeAvailable(cli11)

find_package(Python REQUIRED COMPONENTS Interpreter Development)

find_package(MLIR REQUIRED CONFIG)

list(APPEND CMAKE_MODULE_PATH "${MLIR_CMAKE_DIR}")
list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(TableGen)
include(AddMLIR)
include(AddLLVM)

# ================
# MLIR Tablegen
# ================

set(LLVM_TARGET_DEFINITIONS include/pyir/pyir.td)

# Each of these calls mlir-tblgen with different flags and
# creates a target you can depend on
mlir_tablegen(pyir.h.inc -gen-dialect-decls EXTRA_INCLUDES ${MLIR_INCLUDE_DIRS})
mlir_tablegen(pyir.cpp.inc -gen-dialect-defs EXTRA_INCLUDES ${MLIR_INCLUDE_DIRS})
mlir_tablegen(pyir_ops.h.inc -gen-op-decls EXTRA_INCLUDES ${MLIR_INCLUDE_DIRS})
mlir_tablegen(pyir_ops.cpp.inc -gen-op-defs EXTRA_INCLUDES ${MLIR_INCLUDE_DIRS})
mlir_tablegen(pyir_types.h.inc -gen-typedef-decls EXTRA_INCLUDES ${MLIR_INCLUDE_DIRS})
mlir_tablegen(pyir_types.cpp.inc -gen-typedef-defs EXTRA_INCLUDES ${MLIR_INCLUDE_DIRS})
mlir_tablegen(pyir_attrs.h.inc -gen-attrdef-decls EXTRA_INCLUDES ${MLIR_INCLUDE_DIRS})
mlir_tablegen(pyir_attrs.cpp.inc -gen-attrdef-defs EXTRA_INCLUDES ${MLIR_INCLUDE_DIRS})

# Collects all the above into a single target: pyirIncGen
add_public_tablegen_target(pyirIncGen)

add_mlir_library(pyir
        src/pyir/pyir.cpp
        src/pyir/pyir_ops.cpp
        src/pyir/pyir_types.cpp
        src/pyir/pyir_attrs.cpp

        DEPENDS
        pyirIncGen   # ensures .inc files exist before compiling

        LINK_LIBS PUBLIC
        MLIRIR
        MLIRSupport
)
target_include_directories(pyir SYSTEM PUBLIC
        ${CMAKE_CURRENT_BINARY_DIR}
        ${MLIR_INCLUDE_DIRS}
        ${LLVM_INCLUDE_DIRS}
)

# ================
# Runtime Standard Library
# ================

add_library(pyirmain STATIC
        src/pyruntime/main_runtime.cpp
)
set_target_properties(pyirmain PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

add_library(stdpyir STATIC
        src/pyruntime/builder_runtime.cpp
        src/pyruntime/logical_runtime.cpp
        src/pyruntime/memory_runtime.cpp
        src/pyruntime/builtin_runtime.cpp
        src/pyruntime/runtime_util.cpp
        src/pyruntime/function_runtime.cpp
        src/pyruntime/objects/py_list.cpp
        src/pyruntime/objects/py_set.cpp
        src/pyruntime/objects/py_dict.cpp
        src/pyruntime/objects/py_tuple.cpp
        src/pyruntime/objects/py_str.cpp
        src/pyruntime/objects/py_int.cpp
        src/pyruntime/objects/py_float.cpp
        src/pyruntime/objects/py_bool.cpp
        src/pyruntime/objects/py_object.cpp
        src/pyruntime/objects/py_none.cpp
        src/pyruntime/objects/py_function.cpp
        src/pyruntime/objects/py_method.cpp
        src/pyruntime/control_flow_runtime.cpp
)
set_target_properties(stdpyir PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

# ================
# Source Files and Libraries
# ================

set(PYCOMPILE_SOURCES
        src/bytecode/bytecode.cpp
        src/utils.cpp
        src/bytecode/python_opcodes.cpp
        src/bytecode/python_error.cpp
        src/conversion/pyir_codegen.cpp
        src/bytecode/serialize_bytecode.cpp
        src/conversion/memory_codegen.cpp
        src/conversion/logical_codegen.cpp
        src/conversion/function_codegen.cpp
        src/conversion/control_flow_codegen.cpp
        src/conversion/builder_codegen.cpp
        src/lowering/pyir_lowering.cpp
        src/lowering/llvm_export.cpp
        src/lowering/builder_lowering.cpp
        src/lowering/control_flow_lowering.cpp
        src/lowering/function_lowering.cpp
        src/lowering/logical_lowering.cpp
        src/lowering/memory_lowering.cpp
        src/lowering/pyir_conversion_utils.cpp
)

set(PYCOMPILE_LIBS
        Python::Python
        pyir
        MLIRFuncDialect
        MLIRControlFlowDialect
        MLIRArithDialect
        MLIRLLVMDialect
        MLIRTransforms
        MLIRIR
        MLIRLLVMDialect
        MLIRLLVMToLLVMIRTranslation
        MLIRBuiltinToLLVMIRTranslation
        MLIRFuncToLLVM
        MLIRArithToLLVM
        MLIRArithDialect
        LLVM
)

# ================
# Pycompile
# ================

add_executable(pycompile src/main.cpp ${PYCOMPILE_SOURCES})
target_link_libraries(pycompile PRIVATE ${PYCOMPILE_LIBS} CLI11::CLI11)
target_include_directories(pycompile SYSTEM PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}
        ${MLIR_INCLUDE_DIRS}
        ${LLVM_INCLUDE_DIRS}
)
target_compile_definitions(pycompile PRIVATE
        "PYIR_RUNTIME_LIB_PATH=\"${CMAKE_BINARY_DIR}/lib/libstdpyir.a\""
        "PYIR_RUNTIME_MAIN_PATH=\"${CMAKE_BINARY_DIR}/lib/libpyirmain.a\""
)
add_dependencies(pycompile stdpyir pyirmain)


# ================
# Pycompile Tests
# ================

FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.13.0
)
FetchContent_MakeAvailable(Catch2)

add_executable(pycompile-tests
        ${PYCOMPILE_SOURCES}
        test/bytecode/test_bytecode.cpp
        test/conversion/test_pyir_codegen.cpp
        test/lowering/test_llvm_ir_generation.cpp
        test/execution/test_code_execution.cpp
        test/conversion/test_logical_codegen.cpp
        test/conversion/test_function_codegen.cpp
        test/conversion/test_control_flow_codegen.cpp
        test/conversion/test_builder_codegen.cpp
        test/execution/test_str_execution.cpp
        test/execution/test_list_execution.cpp
        test/execution/test_set_execution.cpp
        test/execution/test_tuple_execution.cpp
        test/execution/test_dict_execution.cpp
        test/execution/test_iter_execution.cpp
        test/execution/test_builtin_execution.cpp
        test/execution/pyir_run_module.cpp
)
target_link_libraries(pycompile-tests PRIVATE ${PYCOMPILE_LIBS} stdpyir Catch2::Catch2WithMain)
target_include_directories(pycompile-tests SYSTEM PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}
        ${MLIR_INCLUDE_DIRS}
        ${LLVM_INCLUDE_DIRS}
)
target_compile_definitions(pycompile-tests PRIVATE
        "PYIR_RUNTIME_LIB_PATH=\"${CMAKE_BINARY_DIR}/lib/libstdpyir.a\""
        "PYIR_RUNTIME_MAIN_PATH=\"${CMAKE_BINARY_DIR}/lib/libpyirmain.a\""
)

include(CTest)
include(Catch)
catch_discover_tests(pycompile-tests WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

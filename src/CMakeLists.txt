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
get_json_value("${CMAKE_SOURCE_DIR}/version.json" "version" PROJECT_VERSION)
message(STATUS "Build ${PROJECT_NAME} version: ${PROJECT_VERSION}")

# Configure the version header
configure_file(
        "${CMAKE_SOURCE_DIR}/version.h.in"
        "${CMAKE_BINARY_DIR}/version.h"
        @ONLY
)

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

add_subdirectory(pyir)
add_subdirectory(pyruntime)

# ================
# Source Files and Libraries
# ================

set(PYCOMPILE_SOURCES
        bytecode/bytecode.cpp
        utils.cpp
        bytecode/python_opcodes.cpp
        bytecode/python_error.cpp
        conversion/pyir_codegen.cpp
        bytecode/serialize_bytecode.cpp
        conversion/memory_codegen.cpp
        conversion/logical_codegen.cpp
        conversion/function_codegen.cpp
        conversion/control_flow_codegen.cpp
        conversion/builder_codegen.cpp
        lowering/pyir_lowering.cpp
        lowering/llvm_export.cpp
        lowering/builder_lowering.cpp
        lowering/control_flow_lowering.cpp
        lowering/function_lowering.cpp
        lowering/logical_lowering.cpp
        lowering/memory_lowering.cpp
        lowering/pyir_conversion_utils.cpp
)
list(TRANSFORM PYCOMPILE_SOURCES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")

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

add_executable(pycompile main.cpp ${PYCOMPILE_SOURCES})
target_link_libraries(pycompile PRIVATE ${PYCOMPILE_LIBS} CLI11::CLI11)
target_include_directories(pycompile SYSTEM PRIVATE
        ${CMAKE_BINARY_DIR}
        ${MLIR_INCLUDE_DIRS}
        ${LLVM_INCLUDE_DIRS}
)
set_target_properties(pycompile PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)
target_compile_definitions(pycompile PRIVATE
        "PYIR_RUNTIME_LIB_PATH=\"${CMAKE_BINARY_DIR}/lib/libstdpyir.a\""
        "PYIR_RUNTIME_MAIN_PATH=\"${CMAKE_BINARY_DIR}/lib/libpyirmain.a\""
)
add_dependencies(pycompile stdpyir pyirmain)

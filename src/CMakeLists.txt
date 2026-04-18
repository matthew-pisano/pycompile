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

# ================
# Add Source Files
# ================

add_subdirectory(bytecode)
add_subdirectory(conversion)
add_subdirectory(lowering)
add_subdirectory(pyir)
add_subdirectory(pyruntime)

set(PYCOMPILE_SOURCES
        utils.cpp
)
list(TRANSFORM PYCOMPILE_SOURCES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")

set(PYCOMPILE_SOURCES
        ${PYCOMPILE_SOURCES}
        ${PYCOMPILE_BYTECODE_SOURCES}
        ${PYCOMPILE_CONVERSION_SOURCES}
        ${PYCOMPILE_LOWERING_SOURCES}
)
set(PYCOMPILE_SOURCES "${PYCOMPILE_SOURCES}" PARENT_SCOPE)

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
set(PYCOMPILE_LIBS "${PYCOMPILE_LIBS}" PARENT_SCOPE)

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
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
target_compile_definitions(pycompile PRIVATE
        "PYIR_RUNTIME_LIB_PATH=\"${CMAKE_BINARY_DIR}/lib/libstdpyir.a\""
        "PYIR_RUNTIME_MAIN_PATH=\"${CMAKE_BINARY_DIR}/lib/libpyirmain.a\""
)
add_dependencies(pycompile stdpyir pyirmain)

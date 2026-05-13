add_subdirectory(src/bytecode)
add_subdirectory(src/conversion)
add_subdirectory(src/lowering)
add_subdirectory(src/pyir EXCLUDE_FROM_ALL) # Ensures that the library is not automatically installed

set(PYCOMPILE_SOURCES
        src/utils.cpp
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

add_executable(pycompile src/main.cpp ${PYCOMPILE_SOURCES})
target_link_libraries(pycompile PRIVATE ${PYCOMPILE_LIBS} CLI11::CLI11)
target_include_directories(pycompile SYSTEM PRIVATE
        src
        ${CMAKE_BINARY_DIR}
        ${MLIR_INCLUDE_DIRS}
        ${LLVM_INCLUDE_DIRS}
)
set_target_properties(pycompile PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        COMPILE_WARNING_AS_ERROR ON
)
target_compile_definitions(pycompile PRIVATE
        "PYIR_RUNTIME_LIB=\"pystd\""
        "PYIR_RUNTIME_MAIN=\"pymain\""
)
add_dependencies(pycompile pystd pymain)

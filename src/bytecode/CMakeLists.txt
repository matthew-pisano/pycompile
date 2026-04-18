set(PYCOMPILE_BYTECODE_SOURCES
        bytecode.cpp
        python_opcodes.cpp
        python_error.cpp
        serialize_bytecode.cpp
)
list(TRANSFORM PYCOMPILE_BYTECODE_SOURCES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")
set(PYCOMPILE_BYTECODE_SOURCES "${PYCOMPILE_BYTECODE_SOURCES}" PARENT_SCOPE)

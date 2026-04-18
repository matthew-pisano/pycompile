set(PYCOMPILE_CONVERSION_SOURCES
        pyir_codegen.cpp
        memory_codegen.cpp
        logical_codegen.cpp
        function_codegen.cpp
        control_flow_codegen.cpp
        builder_codegen.cpp
)
list(TRANSFORM PYCOMPILE_CONVERSION_SOURCES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")
set(PYCOMPILE_CONVERSION_SOURCES "${PYCOMPILE_CONVERSION_SOURCES}" PARENT_SCOPE)

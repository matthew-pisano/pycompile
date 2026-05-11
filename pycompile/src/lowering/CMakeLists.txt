set(PYCOMPILE_LOWERING_SOURCES
        pyir_lowering.cpp
        llvm_export.cpp
        builder_lowering.cpp
        control_flow_lowering.cpp
        function_lowering.cpp
        logical_lowering.cpp
        memory_lowering.cpp
        pyir_conversion_utils.cpp
)
list(TRANSFORM PYCOMPILE_LOWERING_SOURCES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")
set(PYCOMPILE_LOWERING_SOURCES "${PYCOMPILE_LOWERING_SOURCES}" PARENT_SCOPE)

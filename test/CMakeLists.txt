FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.13.0
)
FetchContent_MakeAvailable(Catch2)

add_executable(pycompile-tests
        ${PYCOMPILE_SOURCES}
        bytecode/test_bytecode.cpp
        conversion/test_pyir_codegen.cpp
        lowering/test_llvm_ir_generation.cpp
        execution/test_code_execution.cpp
        conversion/test_logical_codegen.cpp
        conversion/test_function_codegen.cpp
        conversion/test_control_flow_codegen.cpp
        conversion/test_builder_codegen.cpp
        execution/test_str_execution.cpp
        execution/test_list_execution.cpp
        execution/test_set_execution.cpp
        execution/test_tuple_execution.cpp
        execution/test_dict_execution.cpp
        execution/test_iter_execution.cpp
        execution/test_builtin_execution.cpp
        execution/test_error_execution.cpp
        execution/pyir_run_module.cpp
)
target_link_libraries(pycompile-tests PRIVATE ${PYCOMPILE_LIBS} stdpyir Catch2::Catch2WithMain)
target_include_directories(pycompile-tests SYSTEM PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}
        ${MLIR_INCLUDE_DIRS}
        ${LLVM_INCLUDE_DIRS}
)

include(CTest)
include(Catch)
catch_discover_tests(pycompile-tests WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

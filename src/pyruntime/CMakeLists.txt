add_library(pyirmain STATIC
        main_runtime.cpp
)
set_target_properties(pyirmain PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

add_library(stdpyir STATIC
        builder_runtime.cpp
        logical_runtime.cpp
        memory_runtime.cpp
        builtin_runtime.cpp
        runtime_util.cpp
        function_runtime.cpp
        objects/py_list.cpp
        objects/py_set.cpp
        objects/py_dict.cpp
        objects/py_tuple.cpp
        objects/py_str.cpp
        objects/py_int.cpp
        objects/py_float.cpp
        objects/py_bool.cpp
        objects/py_object.cpp
        objects/py_none.cpp
        objects/py_function.cpp
        objects/py_method.cpp
        control_flow_runtime.cpp
)
set_target_properties(stdpyir PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

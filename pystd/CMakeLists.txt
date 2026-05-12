add_library(pymain STATIC
        src/main_runtime.cpp
)
target_include_directories(pymain PRIVATE
        include
        src
)
set_target_properties(pymain PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

add_library(pystd STATIC
        src/builder_runtime.cpp
        src/logical_runtime.cpp
        src/memory_runtime.cpp
        src/builtin_runtime.cpp
        src/runtime_util.cpp
        src/function_runtime.cpp
        src/pytypes/iterables/py_list.cpp
        src/pytypes/iterables/py_set.cpp
        src/pytypes/iterables/py_dict.cpp
        src/pytypes/iterables/py_tuple.cpp
        src/pytypes/primitives/py_str.cpp
        src/pytypes/primitives/py_int.cpp
        src/pytypes/primitives/py_float.cpp
        src/pytypes/primitives/py_bool.cpp
        src/pytypes/py_object.cpp
        src/pytypes/primitives/py_none.cpp
        src/pytypes/py_function.cpp
        src/pytypes/py_method.cpp
        src/control_flow_runtime.cpp
)
target_include_directories(pystd PRIVATE
        include
        src
)
set_target_properties(pystd PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

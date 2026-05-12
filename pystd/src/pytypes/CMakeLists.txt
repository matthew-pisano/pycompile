set(PYSTD_PYTYPES_SOURCES
        iterables/py_list.cpp
        iterables/py_set.cpp
        iterables/py_dict.cpp
        iterables/py_tuple.cpp
        primitives/py_str.cpp
        primitives/py_int.cpp
        primitives/py_float.cpp
        primitives/py_bool.cpp
        py_object.cpp
        primitives/py_none.cpp
        callables/py_function.cpp
        callables/py_method.cpp
)
list(TRANSFORM PYSTD_PYTYPES_SOURCES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")
set(PYSTD_PYTYPES_SOURCES "${PYSTD_PYTYPES_SOURCES}" PARENT_SCOPE)

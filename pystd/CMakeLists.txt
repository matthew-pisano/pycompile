add_library(pymain STATIC
        src/main_runtime.cpp
)
target_include_directories(pymain
        PUBLIC include
        PRIVATE src
)
set_target_properties(pymain PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

add_subdirectory(src/pytypes)

add_library(pystd STATIC
        ${PYSTD_PYTYPES_SOURCES}
        src/builder_runtime.cpp
        src/builtin_runtime.cpp
        src/control_flow_runtime.cpp
        src/function_runtime.cpp
        src/logical_runtime.cpp
        src/memory_runtime.cpp
        src/runtime_util.cpp
)
target_include_directories(pystd
        PUBLIC include
        PRIVATE src
)
set_target_properties(pystd PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

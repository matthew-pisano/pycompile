set(LLVM_TARGET_DEFINITIONS pyir.td)

# Each of these calls mlir-tblgen with different flags and creates a target you can depend on
mlir_tablegen(pyir.h.inc -gen-dialect-decls EXTRA_INCLUDES ${MLIR_INCLUDE_DIRS})
mlir_tablegen(pyir.cpp.inc -gen-dialect-defs EXTRA_INCLUDES ${MLIR_INCLUDE_DIRS})
mlir_tablegen(pyir_ops.h.inc -gen-op-decls EXTRA_INCLUDES ${MLIR_INCLUDE_DIRS})
mlir_tablegen(pyir_ops.cpp.inc -gen-op-defs EXTRA_INCLUDES ${MLIR_INCLUDE_DIRS})
mlir_tablegen(pyir_types.h.inc -gen-typedef-decls EXTRA_INCLUDES ${MLIR_INCLUDE_DIRS})
mlir_tablegen(pyir_types.cpp.inc -gen-typedef-defs EXTRA_INCLUDES ${MLIR_INCLUDE_DIRS})
mlir_tablegen(pyir_attrs.h.inc -gen-attrdef-decls EXTRA_INCLUDES ${MLIR_INCLUDE_DIRS})
mlir_tablegen(pyir_attrs.cpp.inc -gen-attrdef-defs EXTRA_INCLUDES ${MLIR_INCLUDE_DIRS})

# Collects all the above into a single target: pyirIncGen
add_public_tablegen_target(pyirIncGen)

add_mlir_library(pyir
        pyir.cpp
        pyir_ops.cpp
        pyir_types.cpp
        pyir_attrs.cpp

        DEPENDS
        pyirIncGen   # Ensures .inc files exist before compiling

        LINK_LIBS PUBLIC
        MLIRIR
        MLIRSupport
)
target_include_directories(pyir SYSTEM PUBLIC
        ${CMAKE_CURRENT_BINARY_DIR}
        ${MLIR_INCLUDE_DIRS}
        ${LLVM_INCLUDE_DIRS}
)

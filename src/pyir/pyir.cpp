//
// Created by matthew on 3/5/26.
//

#include "pyir/pyir.h"

#include "pyir/pyir_attrs.h"
#include "pyir/pyir_ops.h"
#include "pyir/pyir_types.h"

#include "pyir.cpp.inc"

namespace pyir {
    void PyIRDialect::initialize() {
        addOperations<ToBool, IsTruthy, BinaryOp, Call, LoadConst, LoadDeref, LoadFast, LoadName, StoreName, PopTop,
                      PushNull, Resume, ReturnValue, StoreFast, StoreDeref, UnaryNot, UnaryNegative, UnaryInvert,
                      CompareOp, FormatSimple, BuildString, MakeFunction, PushScope, PopScope, LoadArg, BuildList>();

        addTypes<ByteCodeObjectType>();
        addAttributes<NoneAttr>();
    }
} // namespace pyir

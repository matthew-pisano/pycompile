//
// Created by matthew on 3/5/26.
//

#include "pyir.h"

#include "pyir_attrs.h"
#include "pyir_ops.h"
#include "pyir_types.h"

#include "pyir.cpp.inc"

namespace pyir {
    void PyIRDialect::initialize() {
        addOperations<InitModule, DestroyModule, ToBool, IsTruthy, BinaryOp, Call, LoadConst, LoadDeref, LoadFast,
                      LoadName, StoreName, PopTop, PushNull, Resume, ReturnValue, StoreFast, StoreDeref, UnaryNot,
                      UnaryNegative, UnaryInvert, CompareOp, FormatSimple, BuildString, MakeFunction, PushScope,
                      PopScope, LoadArg, BuildList, ListExtend, ListAppend, LoadAttr, ContainsOp, BuildSet, SetUpdate,
                      SetAdd, BuildMap, MapAdd, StoreSubscr, ForIter, GetIter, PopIter, LoadFastAndClear>();

        addTypes<ByteCodeObjectType>();
        addAttributes<NoneAttr>();
    }
} // namespace pyir

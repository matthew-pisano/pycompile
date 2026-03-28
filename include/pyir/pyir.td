include "mlir/IR/OpBase.td"
include "mlir/IR/AttrTypeBase.td"
include "mlir/Interfaces/SideEffectInterfaces.td"


def PyIR_Dialect : Dialect {
    let name = "pyir";
    let summary = "Python bytecode IR dialect";
    let cppNamespace = "::pyir";
    let useDefaultTypePrinterParser = 1;
    let useDefaultAttributePrinterParser = 1;
}


class PyIR_Type<string name, string typeMnemonic> : TypeDef<PyIR_Dialect, name> {
    let mnemonic = typeMnemonic;
}


def ByteCodeObject_Type : PyIR_Type<"ByteCodeObject", "object"> {
    let summary = "An opaque dynamically-typed Python object";
}


def PyIR_NoneAttr : AttrDef<PyIR_Dialect, "None"> {
    let mnemonic = "none";
    let summary = "Represents the Python None singleton";
}


class PyIR_Op<string mnemonic, list<Trait> traits = []> : Op<PyIR_Dialect, mnemonic, traits>;


def PyIR_Resume : PyIR_Op<"resume"> {
    let summary = "Resume execution of a generator or start a module";
    let arguments = (ins SI64Attr:$arg);
    let assemblyFormat = "$arg attr-dict";
}


def PyIR_PushNull : PyIR_Op<"push_null", [Pure]> {
    let summary = "Push a null value onto the stack (for calls without self)";
    let results = (outs ByteCodeObject_Type:$result);
    let assemblyFormat = "attr-dict";
}


def PyIR_PopTop : PyIR_Op<"pop_top"> {
    let summary = "Discard the top of stack";
    let arguments = (ins ByteCodeObject_Type:$value);
    let assemblyFormat = "$value attr-dict `:` type($value)";
}


def PyIR_LoadName : PyIR_Op<"load_name"> {
    let summary = "Load a name from the current scope";
    let arguments = (ins StrAttr:$var_name);
    let results = (outs ByteCodeObject_Type:$result);
    let assemblyFormat = "$var_name attr-dict";
}


def PyIR_StoreName : PyIR_Op<"store_name"> {
    let summary = "Store a value into a current scope name";
    let arguments = (ins StrAttr:$var_name, ByteCodeObject_Type:$value);
    let assemblyFormat = "$var_name `,` $value attr-dict `:` type($value)";
}


def PyIR_Call : PyIR_Op<"call", [DeclareOpInterfaceMethods<MemoryEffectsOpInterface>]> {
    let summary = "Call a callable with arguments";
    let arguments = (ins ByteCodeObject_Type:$callee,
                        Variadic<ByteCodeObject_Type>:$args);
    let results = (outs ByteCodeObject_Type:$result);
    let assemblyFormat = "$callee `(` $args `)` attr-dict `:` functional-type($args, $result)";
}


def PyIR_LoadFast : PyIR_Op<"load_fast", [Pure]> {
    let summary = "Load a local variable by name";
    let arguments = (ins StrAttr:$var_name);
    let results   = (outs ByteCodeObject_Type:$result);
    let assemblyFormat = "$var_name attr-dict";
}


def PyIR_StoreFast : PyIR_Op<"store_fast"> {
    let summary = "Store a value into a local variable";
    let arguments = (ins StrAttr:$var_name, ByteCodeObject_Type:$value);
    let assemblyFormat = "$var_name `,` $value attr-dict `:` type($value)";
}


def PyIR_ToBool : PyIR_Op<"to_bool", [Pure]> {
    let summary = "Coerce a Python object to a boolean";
    let arguments = (ins ByteCodeObject_Type:$value);
    let results   = (outs ByteCodeObject_Type:$result);
    let assemblyFormat = "$value attr-dict `:` type($value) `->` type($result)";
}


def PyIR_IsTruthy : PyIR_Op<"is_truthy", [Pure]> {
    let summary = "Extract a native i1 boolean from a Python object";
    let arguments = (ins ByteCodeObject_Type:$value);
    let results   = (outs I1:$result);
    let assemblyFormat = "$value attr-dict `:` type($value) `->` type($result)";
}


def PyIR_UnaryNot : PyIR_Op<"unary_not", [Pure]> {
    let summary = "Negates a boolean";
    let arguments = (ins ByteCodeObject_Type:$value);
    let results   = (outs ByteCodeObject_Type:$result);
    let assemblyFormat = "$value attr-dict `:` type($value) `->` type($result)";
}


def PyIR_UnaryNegative : PyIR_Op<"unary_negative", [Pure]> {
    let summary = "Negates a number";
    let arguments = (ins ByteCodeObject_Type:$value);
    let results   = (outs ByteCodeObject_Type:$result);
    let assemblyFormat = "$value attr-dict `:` type($value) `->` type($result)";
}


def PyIR_UnaryInvert : PyIR_Op<"unary_invert", [Pure]> {
    let summary = "Inverts a binary sequence";
    let arguments = (ins ByteCodeObject_Type:$value);
    let results   = (outs ByteCodeObject_Type:$result);
    let assemblyFormat = "$value attr-dict `:` type($value) `->` type($result)";
}


def PyIR_BinaryOp : PyIR_Op<"binary_op"> {
    let summary = "Binary operation on two Python objects";
    let arguments = (ins StrAttr:$op, ByteCodeObject_Type:$lhs, ByteCodeObject_Type:$rhs);
    let results   = (outs ByteCodeObject_Type:$result);
    let assemblyFormat = "$op `,` $lhs `,` $rhs attr-dict `:` type($lhs) `->` type($result)";
}


def PyIR_CompareOp : PyIR_Op<"compare_op"> {
    let summary = "Comparison operation on two Python objects";
    let arguments = (ins StrAttr:$op, ByteCodeObject_Type:$lhs, ByteCodeObject_Type:$rhs);
    let results   = (outs ByteCodeObject_Type:$result);
    let assemblyFormat = "$op `,` $lhs `,` $rhs attr-dict `:` type($lhs) `->` type($result)";
}


def PyIR_LoadConst : PyIR_Op<"load_const", [Pure]> {
    let summary = "Load a constant value";
    let arguments = (ins AnyAttr:$value);
    let results   = (outs ByteCodeObject_Type:$result);
    let assemblyFormat = "$value attr-dict";
}


def PyIR_MakeFunction : PyIR_Op<"make_function", [Pure]> {
    let summary = "Wrap a function pointer into a callable Python object";
    let arguments = (ins StrAttr:$fn_name);
    let results   = (outs ByteCodeObject_Type:$result);
    let assemblyFormat = "$fn_name attr-dict";
}


def PyIR_PushScope : PyIR_Op<"push_scope"> {
    let summary = "Push a new local variable scope onto the scope stack";
    let assemblyFormat = "attr-dict";
}


def PyIR_PopScope : PyIR_Op<"pop_scope"> {
    let summary = "Pop the current local variable scope from the scope stack";
    let assemblyFormat = "attr-dict";
}


def PyIR_LoadArg : PyIR_Op<"load_arg", [Pure]> {
    let summary = "Load the i-th argument from a Value** args array";
    let arguments = (ins ByteCodeObject_Type:$args_ptr,
                         I64Attr:$index);
    let results   = (outs ByteCodeObject_Type:$result);
    let assemblyFormat = "$args_ptr `[` $index `]` attr-dict";
}


def PyIR_ReturnValue : PyIR_Op<"return_value", [Terminator]> {
    let summary = "Return a Value* from a Python function";
    let arguments = (ins ByteCodeObject_Type:$value);
    let assemblyFormat = "$value attr-dict `:` type($value)";
}


def PyIR_LoadDeref : PyIR_Op<"load_deref"> {
    let summary = "Load from a cell or free variable";
    let arguments = (ins StrAttr:$var_name);
    let results   = (outs ByteCodeObject_Type:$result);
    let assemblyFormat = "$var_name attr-dict";
}


def PyIR_StoreDeref : PyIR_Op<"store_deref"> {
    let summary = "Store into a cell or free variable";
    let arguments = (ins StrAttr:$var_name, ByteCodeObject_Type:$value);
    let assemblyFormat = "$var_name `,` $value attr-dict `:` type($value)";
}


def PyIR_FormatSimple : PyIR_Op<"format_simple", [Pure]> {
    let summary = "Convert a value to its string representation";
    let arguments = (ins ByteCodeObject_Type:$value);
    let results   = (outs ByteCodeObject_Type:$result);
    let assemblyFormat = "$value attr-dict `:` type($value)";
}


def PyIR_BuildString : PyIR_Op<"build_string"> {
    let summary = "Concatenate N strings from the stack into one";
    let arguments = (ins Variadic<ByteCodeObject_Type>:$parts);
    let results   = (outs ByteCodeObject_Type:$result);
    let assemblyFormat = "$parts attr-dict `:` functional-type($parts, $result)";
}


def PyIR_BuildList : PyIR_Op<"build_list"> {
    let summary = "Constructs a list using the last N items on the stack";
    let arguments = (ins Variadic<ByteCodeObject_Type>:$parts);
    let results   = (outs ByteCodeObject_Type:$result);
    let assemblyFormat = "$parts attr-dict `:` functional-type($parts, $result)";
}


def PyIR_ListExtend : PyIR_Op<"list_extend"> {
    let summary = "Extends a list (at the given index) with the iterable at the top of the stack";
    let arguments = (ins ByteCodeObject_Type:$list, ByteCodeObject_Type:$items);
    let assemblyFormat = "$list `,` $items attr-dict `:` type($list) `,` type($items)";
}


def PyIR_ListAppend : PyIR_Op<"list_append"> {
    let summary = "Appends to a list (at the given index) with the item at the top of the stack";
    let arguments = (ins ByteCodeObject_Type:$list, ByteCodeObject_Type:$item);
    let assemblyFormat = "$list `,` $item attr-dict `:` type($list) `,` type($item)";
}

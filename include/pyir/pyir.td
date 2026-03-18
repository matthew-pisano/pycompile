include "mlir/IR/OpBase.td"
include "mlir/IR/AttrTypeBase.td"
include "mlir/Interfaces/SideEffectInterfaces.td"


def PyIR_Dialect : Dialect {
    let name = "pyir";
    let summary = "Python bytecode IR dialect";
    let cppNamespace = "::pyir";
    let useDefaultTypePrinterParser = 1;
}


class PyIR_Type<string name, string typeMnemonic>
    : TypeDef<PyIR_Dialect, name> {
    let mnemonic = typeMnemonic;
}


def ByteCodeObject_Type : PyIR_Type<"ByteCodeObject", "object"> {
    let summary = "An opaque dynamically-typed Python object";
}


class PyIR_Op<string mnemonic, list<Trait> traits = []>
    : Op<PyIR_Dialect, mnemonic, traits>;


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
    let assemblyFormat = "$value attr-dict `:` type($value)";
}


def PyIR_IsTruthy : PyIR_Op<"is_truthy", [Pure]> {
    let summary = "Extract a native i1 boolean from a Python object";
    let arguments = (ins ByteCodeObject_Type:$value);
    let results   = (outs I1:$result);
    let assemblyFormat = "$value attr-dict `:` type($value)";
}


def PyIR_UnaryNot : PyIR_Op<"unary_not", [Pure]> {
    let summary = "Negates a boolean";
    let arguments = (ins ByteCodeObject_Type:$value);
    let results   = (outs ByteCodeObject_Type:$result);
    let assemblyFormat = "$value attr-dict `:` type($value)";
}


def PyIR_UnaryNegative : PyIR_Op<"unary_negative", [Pure]> {
    let summary = "Negates a number";
    let arguments = (ins ByteCodeObject_Type:$value);
    let results   = (outs ByteCodeObject_Type:$result);
    let assemblyFormat = "$value attr-dict `:` type($value)";
}


def PyIR_UnaryInvert : PyIR_Op<"unary_invert", [Pure]> {
    let summary = "Inverts a binary sequence";
    let arguments = (ins ByteCodeObject_Type:$value);
    let results   = (outs ByteCodeObject_Type:$result);
    let assemblyFormat = "$value attr-dict `:` type($value)";
}


def PyIR_BinaryOp : PyIR_Op<"binary_op"> {
    let summary = "Binary operation on two Python objects";
    let arguments = (ins StrAttr:$op, ByteCodeObject_Type:$lhs, ByteCodeObject_Type:$rhs);
    let results   = (outs ByteCodeObject_Type:$result);
    let assemblyFormat = "$op `,` $lhs `,` $rhs attr-dict `:` type($lhs)";
}


def PyIR_CompareOp : PyIR_Op<"compare_op"> {
    let summary = "Comparison operation on two Python objects";
    let arguments = (ins StrAttr:$op, ByteCodeObject_Type:$lhs, ByteCodeObject_Type:$rhs);
    let results   = (outs ByteCodeObject_Type:$result);
    let assemblyFormat = "$op `,` $lhs `,` $rhs attr-dict `:` type($lhs)";
}


def PyIR_LoadConst : PyIR_Op<"load_const", [Pure]> {
    let summary = "Load a constant value";
    let arguments = (ins AnyAttr:$value);
    let results   = (outs ByteCodeObject_Type:$result);
    let assemblyFormat = "$value attr-dict";
}


def PyIR_ReturnValue : PyIR_Op<"return_value", [Terminator]> {
    let summary = "Return a value from the current function";
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

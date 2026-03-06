include "mlir/IR/OpBase.td"
include "mlir/IR/AttrTypeBase.td"
include "mlir/Interfaces/SideEffectInterfaces.td"


def PyIR_Dialect : Dialect {
    let name = "pyir";
    let summary = "Python bytecode IR dialect";
    let cppNamespace = "::pyir";

    let useDefaultAttributePrinterParser = 1; // Generate a single merged .h.inc for the dialect class
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


def PyIR_BinaryOp : PyIR_Op<"binary_op"> {
    let summary = "Binary operation on two Python objects";
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

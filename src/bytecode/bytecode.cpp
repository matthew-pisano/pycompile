//
// Created by matthew on 3/1/26.
//


#include <stdexcept>

#include "bytecode/bytecode.h"

#include <filesystem>
#include <iostream>

#include "bytecode/python_error.h"
#include "bytecode/python_raii.h"
#include "utils.h"


/**
 * The maximum depth to evaluate nested Python Callables to
 */
static constexpr int NESTED_FUNCTION_DEPTH = 16;


/**
 *  Helper function to read a variable-length integer from the exception table data.
 *  The varint format uses the lower 6 bits of each byte for data, and the 7th bit as a continuation flag.
 *  The 8th bit is unused.
 *
 *  Each entry is encoded as:
 *  * start:  varint (instruction offset, in units of 2 bytes)
 *  * length: varint (number of instructions covered)
 *  * target: varint (handler offset, in units of 2 bytes)
 *  * depth+lasti: varint where bit 0 = lasti flag, bits 1+ = stack depth
 *
 * @param data The raw bytes of the exception table
 * @param len The length of the data
 * @param pos The current position in the data, which will be updated as bytes are read
 * @return The decoded integer value for the current varint
 */
int readVarint(const unsigned char* data, const Py_ssize_t len, Py_ssize_t& pos) {
    int val = 0;
    int shift = 0;
    while (pos < len) {
        const int byte = data[pos++]; // Read next byte, advance position
        val |= (byte & 0x3f) << shift; // Mask off top 2 bits, shift into place
        shift += 6; // Next group goes 6 bits higher

        if (!(byte & 0x40))
            break; // No continuation bit, done
    }
    return val;
}


/**
 * Decode the exception table from a code object, returning a list of ExceptionTableEntry structs.
 * @param code A Python code object from which to extract the exception table
 * @return A vector of ExceptionTableEntry structs representing the decoded exception table entries.
 */
std::vector<ExceptionTableEntry> decodeExceptionTable(PyObject* code) {
    std::vector<ExceptionTableEntry> entries;

    PyObject* raw = PyObject_GetAttrString(code, "co_exceptiontable");
    if (!raw || !PyBytes_Check(raw)) {
        Py_XDECREF(raw);
        // If there's no exception table, or it's not in the expected format, just return an empty list.
        return entries;
    }

    const unsigned char* data = reinterpret_cast<unsigned char*>(PyBytes_AsString(raw));
    const Py_ssize_t len = PyBytes_Size(raw);
    Py_ssize_t pos = 0;

    while (pos < len) {
        // Extract the fields for each entry using the varint reader.
        // Note that start and target are in units of 2 bytes, so we multiply by 2 to get the actual byte offsets.
        // pos is updated by readVarint to point to the next field after reading.
        const size_t start = readVarint(data, len, pos) * 2;
        const size_t length = readVarint(data, len, pos) * 2;
        const size_t target = readVarint(data, len, pos) * 2;
        const size_t depthLasti = readVarint(data, len, pos);

        entries.push_back({
                start, start + length, target,
                depthLasti >> 1, // depth is the varint value shifted right by 1 (divide by 2)
                (depthLasti & 1) != 0 // lasti flag is the least significant bit
        });
    }

    Py_DECREF(raw);
    return entries;
}


/**
 * Helper function to extract a tuple of strings from a code object's attribute (e.g. co_freevars, co_cellvars).
 * @param code A Python code object from which to extract the tuple of strings
 * @param attr The name of the attribute to extract (e.g. "co_freevars", "co_cellvars")
 * @return A vector of strings extracted from the specified attribute of the code object.
 * @throws std::runtime_error if the specified attribute is not found or is not a tuple of strings.
 */
std::vector<std::string> extractPyTupleStrings(PyObject* code, const char* attr) {
    std::vector<std::string> result;
    // Fetch Python tuple object
    PyObject* tup = PyObject_GetAttrString(code, attr);
    if (!tup || !PyTuple_Check(tup)) {
        Py_XDECREF(tup);
        throw std::runtime_error(std::string("Expected attribute '") + attr + "' to be a tuple");
    }

    for (Py_ssize_t i = 0; i < PyTuple_Size(tup); i++) {
        PyObject* s = PyTuple_GetItem(tup, i); // Borrowed reference, do not decref
        if (PyUnicode_Check(s))
            result.emplace_back(PyUnicode_AsUTF8(s));
    }
    Py_DECREF(tup);
    return result;
}

// Forward declaration
ByteCodeModule generatePythonByteCode(PyObject* code, const std::string& filename, int depth = 0);


/**
 * Translates a Python tuple object into a variant of C++ representations of Python primitives
 */
std::vector<PrimitiveArgvals> decodeTupleStr(PyObject* argval, const std::string& filename,
                                             const ByteCodeInstruction& instr) {
    std::vector<PrimitiveArgvals> tupleStrs;
    const Py_ssize_t n = PyTuple_Size(argval);
    for (Py_ssize_t i = 0; i < n; i++) {
        PyObject* s = PyTuple_GetItem(argval, i); // Borrowed, no need to decref
        if (PyBool_Check(s))
            tupleStrs.emplace_back(s == Py_True);
        else if (PyLong_Check(s))
            tupleStrs.emplace_back(PyLong_AsLong(s));
        else if (PyFloat_Check(s))
            tupleStrs.emplace_back(PyFloat_AsDouble(s));
        else if (PyUnicode_Check(s))
            tupleStrs.emplace_back(PyUnicode_AsUTF8(s));
        else if (Py_IsNone(s))
            tupleStrs.emplace_back(ArgvalNone{});
        else
            throw PyCompileError("Unknown tuple str argval type " + instr.argrepr, filename, instr.lineno);
    }

    return tupleStrs;
}


/**
 * Translates a Python frozen set object into a variant of C++ representations of Python primitives
 */
std::vector<PrimitiveArgvals> decodeFrozenSet(PyObject* argval, const std::string& filename,
                                              const ByteCodeInstruction& instr) {
    std::vector<PrimitiveArgvals> frozenSet;
    PyObject* iter = PyObject_GetIter(argval);
    if (!iter)
        throw PyCompileError("Failed to iterate frozen set " + instr.argrepr, filename, instr.lineno);

    PyObject* s;
    while ((s = PyIter_Next(iter))) {
        if (PyBool_Check(s))
            frozenSet.emplace_back(s == Py_True);
        else if (PyLong_Check(s))
            frozenSet.emplace_back(PyLong_AsLong(s));
        else if (PyFloat_Check(s))
            frozenSet.emplace_back(PyFloat_AsDouble(s));
        else if (PyUnicode_Check(s))
            frozenSet.emplace_back(PyUnicode_AsUTF8(s));
        else if (Py_IsNone(s))
            frozenSet.emplace_back(ArgvalNone{});
        else {
            Py_DECREF(s);
            Py_DECREF(iter);
            throw PyCompileError("Unknown frozen set argval type " + instr.argrepr, filename, instr.lineno);
        }
        Py_DECREF(s); // PyIter_Next returns a new reference
    }

    Py_DECREF(iter);
    return frozenSet;
}


/**
 * Decodes a Python object into a bytecode instruction.
 * @param pyobject The Python object to decode.
 * @param filename The filename for nested code objects.
 * @param lineno The current line number, updated as a reference.
 * @param depth The current recursion depth for nested code objects (default is 0)
 * @return A decoded instruction.
 */
ByteCodeInstruction decodeByteCodeInstruction(PyObject* pyobject, const std::string& filename, int& lineno,
                                              const int depth) {
    ByteCodeInstruction instr{};

    PyObject* offset = PyObject_GetAttrString(pyobject, "offset");
    instr.offset = PyLong_AsLong(offset);
    Py_DECREF(offset);

    PyObject* opcode = PyObject_GetAttrString(pyobject, "opcode");
    instr.opcodeId = PyLong_AsInt(opcode);
    Py_DECREF(opcode);

    PyObject* opname = PyObject_GetAttrString(pyobject, "opname");
    std::string opcodeStr = PyUnicode_AsUTF8(opname);
    instr.opcode = pythonOpcodeFromString(opcodeStr);
    Py_DECREF(opname);

    PyObject* argrepr = PyObject_GetAttrString(pyobject, "argrepr");
    instr.argrepr = PyUnicode_AsUTF8(argrepr);
    Py_DECREF(argrepr);

    // starts_line is a boolean indicating whether this instruction starts a new source line.
    PyObject* startsLine = PyObject_GetAttrString(pyobject, "starts_line");
    bool isStartOfLine = startsLine && PyObject_IsTrue(startsLine) == 1;
    instr.startsLine = isStartOfLine;
    Py_XDECREF(startsLine);

    PyObject* linenoAttr = PyObject_GetAttrString(pyobject, "positions");
    if (linenoAttr && linenoAttr != Py_None) {
        // In Python 3.11+, positions is available
        PyObject* startLine = PyObject_GetAttrString(linenoAttr, "lineno");
        if (startLine && startLine != Py_None)
            lineno = PyLong_AsInt(startLine);

        instr.lineno = lineno;
        Py_XDECREF(startLine);
        Py_DECREF(linenoAttr);
    }
    // Fallback for older Python versions or if positions unavailable
    else
        Py_XDECREF(linenoAttr);

    PyObject* argval = PyObject_GetAttrString(pyobject, "argval"); // borrowed reference
    if (PyBool_Check(argval)) {
        instr.argvalType = ArgvalType::Bool;
        instr.argval = argval == Py_True;
    } else if (PyLong_Check(argval)) {
        instr.argvalType = ArgvalType::Int;
        instr.argval = PyLong_AsLong(argval);
    } else if (PyFloat_Check(argval)) {
        instr.argvalType = ArgvalType::Float;
        instr.argval = PyFloat_AsDouble(argval);
    } else if (PyUnicode_Check(argval)) {
        instr.argvalType = ArgvalType::Str;
        instr.argval = PyUnicode_AsUTF8(argval);
    } else if (PyTuple_Check(argval)) {
        instr.argvalType = ArgvalType::TupleStr;
        instr.argval = decodeTupleStr(argval, filename, instr);
    } else if (PyFrozenSet_Check(argval)) {
        instr.argvalType = ArgvalType::FrozenSet;
        instr.argval = decodeFrozenSet(argval, filename, instr);
    } else if (PyCode_Check(argval)) {
        instr.argvalType = ArgvalType::Code;
        instr.argval = std::make_shared<ByteCodeModule>(generatePythonByteCode(argval, filename, depth + 1));
    } else if (Py_IsNone(argval)) {
        instr.argvalType = ArgvalType::None;
        instr.argval = ArgvalNone{};
    } else
        throw PyCompileError("Unknown argval type " + instr.argrepr, filename, lineno);
    Py_XDECREF(argval);
    return instr;
}


/**
 * Disassemble a Python code object into a ByteCodeModule struct, extracting its metadata and instructions.
 * This function is recursive and will disassemble nested code objects (e.g. lambdas, comprehensions) up to a max depth.
 * @param code A Python code object containing the program to be translated.
 * @param filename The filename of the compiled module.
 * @param depth The current recursion depth for nested code objects (default is 0).
 * @return A ByteCodeModule struct containing the metadata and instructions of the code object.
 * @throws std::runtime_error if the max nested function depth is exceeded, or if there are errors during disassembly.
 */
ByteCodeModule generatePythonByteCode(PyObject* code, const std::string& filename, const int depth) {
    ByteCodeModule result;
    result.filename = filename;
    result.moduleName = std::filesystem::path(filename).stem();

    if (depth > NESTED_FUNCTION_DEPTH)
        throw std::runtime_error("Maximum nested function depth exceeded");

    // Code-level metadata
    result.info.freevars = extractPyTupleStrings(code, "co_freevars");
    result.info.cellvars = extractPyTupleStrings(code, "co_cellvars");
    result.info.varnames = extractPyTupleStrings(code, "co_varnames");
    result.info.exceptionTable = decodeExceptionTable(code);

    PyObject* argcount = PyObject_GetAttrString(code, "co_argcount");
    result.info.argcount = argcount ? PyLong_AsInt(argcount) : 0;
    Py_XDECREF(argcount);

    PyObject* codeName = PyObject_GetAttrString(code, "co_name");
    result.info.codeName = codeName ? PyUnicode_AsUTF8(codeName) : "<unknown>";
    Py_XDECREF(codeName);

    // Instructions
    PyObject* dis = PyImport_ImportModule("dis");
    if (!dis)
        throw std::runtime_error(getPythonErrorTraceback());

    PyObject* instrIt = PyObject_CallMethod(dis, "get_instructions", "O", code);
    Py_DECREF(dis);
    if (!instrIt)
        throw std::runtime_error(getPythonErrorTraceback());

    // Get the first line number from the code object
    PyObject* firstLineno = PyObject_GetAttrString(code, "co_firstlineno");
    int currentLineno = firstLineno ? PyLong_AsInt(firstLineno) : 1;
    Py_XDECREF(firstLineno);

    PyObject* item;
    // Iterate over the instructions returned by dis, extracting their attributes into Instruction structs
    while ((item = PyIter_Next(instrIt))) {
        // Recursive call is devluating a nested code object
        ByteCodeInstruction instr = decodeByteCodeInstruction(item, filename, currentLineno, depth);
        Py_DECREF(item);
        result.instructions.push_back(std::move(instr));
    }

    Py_DECREF(instrIt);
    return result;
}


std::vector<ByteCodeModule> compilePython(const std::vector<std::string>& fileContents,
                                          const std::vector<std::string>& fileNames) {
    PythonInterpreter pyInterp; // Initializes Python via RAII

    // Disassemble PyObjects into Python bytecode
    std::vector<ByteCodeModule> bytecodeModules;
    bytecodeModules.reserve(fileContents.size());
    for (size_t i = 0; i < fileContents.size(); i++)
        try {
            PyObject* code = Py_CompileString(fileContents[0].c_str(), fileNames[i].c_str(), Py_file_input);
            if (!code)
                throw std::runtime_error(getPythonErrorTraceback());

            bytecodeModules.push_back(generatePythonByteCode(code, fileNames[i]));
            Py_DECREF(code); // Destroy object before scope ends
        } catch (const std::runtime_error& e) {
            throw std::runtime_error(std::filesystem::path(fileNames[i]).filename().string() + ": " + e.what());
        }
    return bytecodeModules;
}


ByteCodeModule compilePython(const std::string& fileContent, const std::string& fileName) {
    const std::vector fileContents = {fileContent};
    const std::vector fileNames = {fileName};
    const std::vector<ByteCodeModule> bytecodeModules = compilePython(fileContents, fileNames);
    return bytecodeModules[0];
}

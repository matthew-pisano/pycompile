//
// Created by matthew on 3/1/26.
//


#include <stdexcept>

#include "bytecode/bytecode.h"

#include <filesystem>
#include <iomanip>
#include <iostream>

#include "bytecode/python_error.h"
#include "bytecode/python_raii.h"


inline std::string argvalTypeToString(const ArgvalType type) {
    switch (type) {
        case ArgvalType::None:
            return "None";
        case ArgvalType::Bool:
            return "Bool";
        case ArgvalType::Int:
            return "Int";
        case ArgvalType::Float:
            return "Float";
        case ArgvalType::Str:
            return "Str";
        case ArgvalType::TupleStr:
            return "TupleStr";
        case ArgvalType::Code:
            return "Code";
        default:
            return "Unknown";
    }
}


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
        const int byte = data[pos++]; // read next byte, advance position
        val |= (byte & 0x3f) << shift; // mask off top 2 bits, shift into place
        shift += 6; // next group goes 6 bits higher

        if (!(byte & 0x40))
            break; // no continuation bit → we're done
    }
    return val;
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


void serializeInstruction(const ByteCodeInstruction& instr, std::ostream& os, const int indentLevel) {
    const std::string ind(indentLevel * 4, ' ');

    std::string instRepr;
    if (!instr.argrepr.empty())
        instRepr = instr.argrepr;
    else if (instr.argvalType == ArgvalType::Int)
        instRepr = std::to_string(std::get<int64_t>(instr.argval));
    else if (instr.argvalType == ArgvalType::Float)
        instRepr = std::to_string(std::get<double_t>(instr.argval));
    else if (instr.argvalType == ArgvalType::Str)
        instRepr = std::get<std::string>(instr.argval);
    else if (instr.argvalType == ArgvalType::TupleStr) {
        instRepr = "[tuple]";
    } else if (instr.argvalType == ArgvalType::Code)
        instRepr = "[code object]";
    else
        instRepr = "";

    const std::string argTypeStr = "[" + argvalTypeToString(instr.argvalType) + "]";
    const std::string lineStartStr = instr.startsLine ? "*" : " ";
    const std::string linenoStr = instr.lineno.has_value() ? "L" + std::to_string(*instr.lineno) : "L-";

    os << ind << lineStartStr << linenoStr << " offset " << std::setw(4) << std::left << instr.offset << " | "
            << std::setw(30) << std::left << instr.opcode << " " << std::setw(10) << std::left << argTypeStr << " | "
            << instRepr << std::endl;
}


void serializeByteCodeModule(const ByteCodeModule& code, std::ostream& os, const int depth) {
    const std::string ind(depth * 4, ' ');

    // Print code metadata
    if (!code.info.cellvars.empty()) {
        os << ind << "cellvars: ";
        for (const std::string& v : code.info.cellvars)
            os << v << " ";
        os << "\n";
    }
    if (!code.info.freevars.empty()) {
        os << ind << "freevars: ";
        for (const std::string& v : code.info.freevars)
            os << v << " ";
        os << "\n";
    }
    if (!code.info.exceptionTable.empty()) {
        os << ind << "exception table:\n";
        for (const ExceptionTableEntry& e : code.info.exceptionTable) {
            os << ind << "  [" << e.start << ", " << e.end << ") -> target " << e.target << "  depth " << e.depth
                    << "  lasti " << (e.lasti ? "true" : "false") << "\n";
        }
    }

    // Print instructions
    for (const ByteCodeInstruction& instr : code.instructions) {
        serializeInstruction(instr, os, depth);

        // If the instruction has a nested code object, print it recursively with increased indentation.
        if (instr.argvalType == ArgvalType::Code) {
            os << ind << "  [nested code object]:\n";
            // Wrap nested instructions in a temporary DisassembledCode for printing
            if (const auto* nestedPtr = std::get_if<std::shared_ptr<ByteCodeModule> >(&instr.argval)) {
                if (*nestedPtr)
                    serializeByteCodeModule(**nestedPtr, os, depth + 1);
            } else
                throw std::runtime_error("Expected argval to be a vector of Instructions for nested code object");
        }
    }
}


/**
 * Decodes a Python object into a bytecode instruction.
 * @param pyobject The Python object to decode.
 * @param filename The filename for nested code objects.
 * @param moduleName The module name for nested code objects.
 * @param currentLineno The current line number, updated as a reference.
 * @param depth The current recursion depth for nested code objects (default is 0)
 * @return A decoded instruction.
 */
ByteCodeInstruction decodeByteCodeInstruction(PyObject* pyobject, const std::string& filename,
                                              const std::string& moduleName, int& currentLineno, const int depth) {
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
            currentLineno = PyLong_AsInt(startLine);

        instr.lineno = currentLineno;
        Py_XDECREF(startLine);
        Py_DECREF(linenoAttr);
    }
    // Fallback for older Python versions or if positions unavailable
    else
        Py_XDECREF(linenoAttr);

    PyObject* argval = PyObject_GetAttrString(pyobject, "argval"); // borrowed reference
    if (PyBool_Check(argval)) {
        instr.argvalType = ArgvalType::Bool;
        instr.argval = (argval == Py_True);
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
        std::vector<std::string> tupleStrs;
        const Py_ssize_t n = PyTuple_Size(argval);
        for (Py_ssize_t i = 0; i < n; i++) {
            PyObject* s = PyTuple_GetItem(argval, i); // borrowed
            if (PyUnicode_Check(s))
                tupleStrs.emplace_back(PyUnicode_AsUTF8(s));
        }
        instr.argval = std::move(tupleStrs);
    } else if (PyCode_Check(argval)) {
        instr.argvalType = ArgvalType::Code;
        // Increment refcount so the temporary CompiledModule owns the code object and will decref it when destroyed
        Py_XINCREF(argval);
        CompiledModule nested{filename, moduleName, argval};
        instr.argval = std::make_shared<ByteCodeModule>(generatePythonByteCode(nested, depth + 1));
    } else {
        instr.argvalType = ArgvalType::None; // For any other types, just treat it as None
        instr.argval = ArgvalNone{};
    }
    Py_XDECREF(argval);
    return instr;
}


ByteCodeModule generatePythonByteCode(const CompiledModule& compiledModule, const int depth) {
    ByteCodeModule result;
    result.filename = compiledModule.filename;
    result.moduleName = compiledModule.moduleName;

    if (depth > NESTED_FUNCTION_DEPTH)
        throw std::runtime_error("Maximum nested function depth exceeded");

    PyObject* code = compiledModule.codeObject; // Borrowed reference, do not decref

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
        ByteCodeInstruction instr = decodeByteCodeInstruction(item, compiledModule.filename, compiledModule.moduleName,
                                                              currentLineno, depth);
        Py_DECREF(item);
        result.instructions.push_back(std::move(instr));
    }

    Py_DECREF(instrIt);
    return result;
}


std::vector<ByteCodeModule> compilePython(const std::vector<std::string>& fileContents,
                                          const std::vector<std::string>& fileNames) {
    PythonInterpreter pyInterp; // Initializes Python via RAII
    std::vector<CompiledModule> compiledModules;
    compiledModules.reserve(fileContents.size());
    for (size_t i = 0; i < fileContents.size(); i++)
        try {
            compiledModules.push_back(compilePythonSource(fileContents[0], fileNames[i], fileNames[i]));
        } catch (const std::runtime_error& e) {
            throw std::runtime_error(std::filesystem::path(fileNames[i]).filename().string() + ": " + e.what());
        }

    // Disassemble PyObjects into Python bytecode
    std::vector<ByteCodeModule> bytecodeModules;
    bytecodeModules.reserve(compiledModules.size());
    for (size_t i = 0; i < compiledModules.size(); i++)
        try {
            bytecodeModules.push_back(generatePythonByteCode(compiledModules[i]));
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

//
// Created by matthew on 3/1/26.
//


#include <stdexcept>

#include "bytecode.h"
#include "utils.h"


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

    const auto* data = reinterpret_cast<unsigned char*>(PyBytes_AsString(raw));
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
                start,
                start + length,
                target,
                depthLasti >> 1, // depth is the varint value shifted right by 1 (divide by 2)
                (depthLasti & 1) != 0 // lasti flag is the least significant bit
        });
    }

    Py_DECREF(raw);
    return entries;
}


void printInstruction(const Instruction& instr, const int indentLevel) {
    const std::string ind(indentLevel * 4, ' ');
    printf("%s%s offset %4lu | %-30s | %s\n",
           ind.c_str(),
           instr.lineno.has_value()
               ? ("L" + std::to_string(*instr.lineno) + " ").c_str()
               : "    ",
           instr.offset,
           instr.opname.c_str(),
           instr.argrepr.c_str());
}


void printDisassembly(const DisassembledCode& code, const int depth) {
    const std::string ind(depth * 4, ' ');

    // Print code metadata
    if (!code.info.cellvars.empty()) {
        printf("%scellvars: ", ind.c_str());
        for (const auto& v : code.info.cellvars)
            printf("%s ", v.c_str());
        printf("\n");
    }
    if (!code.info.freevars.empty()) {
        printf("%sfreevars: ", ind.c_str());
        for (const auto& v : code.info.freevars)
            printf("%s ", v.c_str());
        printf("\n");
    }
    if (!code.info.exceptionTable.empty()) {
        printf("%sexception table:\n", ind.c_str());
        for (const auto& e : code.info.exceptionTable) {
            printf("%s  [%lu, %lu) -> target %lu  depth %lu  lasti %d\n",
                   ind.c_str(), e.start, e.end, e.target, e.depth, static_cast<int>(e.lasti));
        }
    }

    // Print instructions
    for (const auto& instr : code.instructions) {
        printInstruction(instr, depth);

        // If the instruction has a nested code object, print it recursively with increased indentation.
        if (instr.argvalType == ArgvalType::Code) {
            printf("%s  [nested code object]:\n", ind.c_str());
            // Wrap nested instructions in a temporary DisassembledCode for printing
            if (const std::vector<Instruction>* nestedCode = std::get_if<std::vector<Instruction> >(&instr.argval)) {
                DisassembledCode nested;
                nested.instructions = *nestedCode;
                printDisassembly(nested, depth + 1);
            } else
                throw std::runtime_error("Expected argval to be a vector of Instructions for nested code object");
        }
    }
}


DisassembledCode disassemble(PyObject* code, const int depth) {
    DisassembledCode result;

    if (depth > NESTED_FUNCTION_DEPTH)
        throw std::runtime_error("Maximum nested function depth exceeded");

    // Code-level metadata
    result.info.freevars = extractPyTupleStrings(code, "co_freevars");
    result.info.cellvars = extractPyTupleStrings(code, "co_cellvars");
    result.info.exceptionTable = decodeExceptionTable(code);

    // Instructions
    PyObject* dis = PyImport_ImportModule("dis");
    if (!dis)
        throw std::runtime_error(getPythonErrorTraceback());

    PyObject* instrIt = PyObject_CallMethod(dis, "get_instructions", "O", code);
    Py_DECREF(dis);
    if (!instrIt)
        throw std::runtime_error(getPythonErrorTraceback());

    PyObject* item;
    // Iterate over the instructions returned by dis, extracting their attributes into Instruction structs.
    while ((item = PyIter_Next(instrIt))) {
        Instruction instr{};

        PyObject* offset = PyObject_GetAttrString(item, "offset");
        instr.offset = PyLong_AsLong(offset);
        Py_DECREF(offset);

        PyObject* opcode = PyObject_GetAttrString(item, "opcode");
        instr.opcode = PyLong_AsInt(opcode);
        Py_DECREF(opcode);

        PyObject* opname = PyObject_GetAttrString(item, "opname");
        instr.opname = PyUnicode_AsUTF8(opname);
        Py_DECREF(opname);

        PyObject* argrepr = PyObject_GetAttrString(item, "argrepr");
        instr.argrepr = PyUnicode_AsUTF8(argrepr);
        Py_DECREF(argrepr);

        // startsLine is int | None
        PyObject* startsLine = PyObject_GetAttrString(item, "starts_line");
        if (startsLine && startsLine != Py_None)
            instr.lineno = PyLong_AsLong(startsLine);
        Py_XDECREF(startsLine);

        // argval: int | str | tuple | code object | other
        PyObject* argval = PyObject_GetAttrString(item, "argval");
        if (PyLong_Check(argval)) {
            instr.argvalType = ArgvalType::Int;
            instr.argval = PyLong_AsInt(argval);
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
            instr.argval = disassemble(argval, depth + 1).instructions;
        } else {
            instr.argvalType = ArgvalType::None; // for any other types, we just treat it as None
            instr.argval = ArgvalNone{};
        }

        Py_DECREF(argval);
        Py_DECREF(item);
        result.instructions.push_back(std::move(instr));
    }

    Py_DECREF(instrIt);
    return result;
}

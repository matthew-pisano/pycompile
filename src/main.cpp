#include <Python.h>
#include <string>
#include <vector>
#include <optional>
#include <iostream>

/**
 * The maximum depth to evaluate nested Python Callables to
 */
static constexpr int NESTED_FUNCTION_DEPTH = 16;


/**
 * Python 3.11+ uses an exception table to determine which exception handler to jump to when an
 * exception is raised. Each entry in the table corresponds to a protected region of Python code.
 */
struct ExceptionTableEntry {
    size_t start; // First covered instruction offset
    size_t end; // Last covered instruction offset (exclusive)
    size_t target; // Handler block offset
    size_t depth; // Stack depth at handler entry
    bool lasti; // Whether to push the faulting instruction offset
};


/**
 * Program metadata extracted from a code object, relevant to bytecode evaluation and disassembly.
 */
struct CodeInfo {
    /**
     * Free variables that are defined in outer functions, but used by inners.
     * Used in closures.
     */
    std::vector<std::string> freevars;

    /**
     * Cell (captured) variables that are defined in outer functions, but used by inners.
     * Used in closures.
     */
    std::vector<std::string> cellvars;

    /**
     * Variable names defined in the code object, used for debugging and disassembly purposes.
     */
    std::vector<std::string> varnames;

    /**
    * Names of local variables, arguments, and other identifiers used in the code object.
    * Used for debugging and disassembly purposes.
    */
    std::vector<std::string> names;

    /**
     * Exception table entries, which define protected regions of code and their handlers.
     */
    std::vector<ExceptionTableEntry> exceptionTable;
};


/**
 * A single Python bytecode instruction, as returned by dis.get_instructions().
 */
struct Instruction {
    size_t offset; // Byte offset of this instruction in the code object
    int opcode; // The numeric opcode
    std::string opname; // The human-readable opcode name
    std::string argrepr; // Human-readable description of the instruction argument, if any
    std::optional<size_t> lineno; // The line number, set when starts_line is not None
    int argvalInt; // If the instruction argument is an integer, its value
    std::string argvalStr; // If the instruction argument is a string, its value
    std::vector<std::string> argvalTuple; // If the instruction argument is a tuple of strings
    std::vector<Instruction> argvalCode; // For nested code objects (lambda, comprehension, etc.)
};


/**
 * A disassembled code object, containing its metadata and the list of instructions.
 */
struct DisassembledCode {
    CodeInfo info; // Metadata about the code object
    std::vector<Instruction> instructions; // The list of bytecode instructions
};


/**
 * Helper function to extract the full traceback of a Python exception as a string.
 * @return A string containing the traceback of the most recently raised Python exception.
 * @throws std::runtime_error if there is no active Python exception.
 */
std::string getPythonErrorTraceback() {
    PyObject* exc = PyErr_GetRaisedException();
    if (!exc)
        throw std::runtime_error("No active Python exception");

    // Import the traceback module to format the exception
    PyObject* tracebackModule = PyImport_ImportModule("traceback");
    std::string msg;

    if (tracebackModule) {
        PyObject* lines = PyObject_CallMethod(tracebackModule, "format_exception",
                                              "OOO",
                                              PyObject_Type(exc), exc,
                                              PyException_GetTraceback(exc));
        if (lines) {
            PyObject* joined = PyUnicode_Join(PyUnicode_FromString(""), lines);
            if (joined)
                msg = PyUnicode_AsUTF8(joined);

            Py_XDECREF(joined);
            Py_DECREF(lines);
        }
        Py_DECREF(tracebackModule);
    } else {
        // If we can't import traceback, fall back to just the exception type and message
        PyObject* str = PyObject_Str(exc);
        if (str)
            msg = PyUnicode_AsUTF8(str);
        Py_XDECREF(str);
    }

    Py_DECREF(exc);
    return msg;
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
 * Disassemble a Python code object into a DisassembledCode struct, extracting its metadata and instructions.
 * This function is recursive and will disassemble nested code objects (e.g. lambdas, comprehensions) up to a max depth.
 * @param code A Python code object to disassemble
 * @param depth The current recursion depth for nested code objects (default is 0)
 * @return A DisassembledCode struct containing the metadata and instructions of the code object
 * @throws std::runtime_error if the max nested function depth is exceeded, or if there are errors during disassembly.
 */
DisassembledCode disassemble(PyObject* code, const int depth = 0) {
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
        if (PyLong_Check(argval))
            instr.argvalInt = PyLong_AsInt(argval);
        else if (PyUnicode_Check(argval))
            instr.argvalStr = PyUnicode_AsUTF8(argval);
        else if (PyTuple_Check(argval)) {
            const Py_ssize_t n = PyTuple_Size(argval);
            for (Py_ssize_t i = 0; i < n; i++) {
                PyObject* s = PyTuple_GetItem(argval, i); // borrowed
                if (PyUnicode_Check(s))
                    instr.argvalTuple.emplace_back(PyUnicode_AsUTF8(s));
            }
        } else if (PyCode_Check(argval))
            instr.argvalCode = disassemble(argval, depth + 1).instructions;

        Py_DECREF(argval);
        Py_DECREF(item);
        result.instructions.push_back(std::move(instr));
    }

    Py_DECREF(instrIt);
    return result;
}


/**
 * Helper function to print a single instruction in a human-readable format, with indentation for nested code.
 * @param instr The Instruction struct to print
 * @param indentLevel The current indentation level (number of spaces) to use for printing the instruction
 */
void printInstruction(const Instruction& instr, const int indentLevel = 0) {
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


/**
 * Helper function to print the disassembled code in a human-readable format, including metadata and instructions.
 * @param code The DisassembledCode struct to print
 * @param depth The current recursion depth for nested code objects, used for indentation (default is 0)
 */
void printDisassembly(const DisassembledCode& code, const int depth = 0) {
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
        if (!instr.argvalCode.empty()) {
            printf("%s  [nested code object]:\n", ind.c_str());
            // Wrap nested instructions in a temporary DisassembledCode for printing
            DisassembledCode nested;
            nested.instructions = instr.argvalCode;
            printDisassembly(nested, depth + 1);
        }
    }
}


int main() {
    Py_Initialize();

    const char* source =
            "def outer():\n"
            "    x = 10\n"
            "    f = lambda n: n * x\n" // closure — exercises freevars/cellvars
            "    try:\n"
            "        squares = [i**2 for i in range(x)]\n" // comprehension
            "    except ValueError as e:\n"
            "        pass\n"
            "    return f, squares\n";

    PyObject* code = Py_CompileString(source, "<embedded>", Py_file_input);
    if (!code) {
        const std::string err = getPythonErrorTraceback();
        Py_Finalize();
        std::cerr << "Error during parsing: " + err << std::endl;
        return 1;
    }

    try {
        const DisassembledCode d = disassemble(code);
        printDisassembly(d);
    } catch (const std::runtime_error& e) {
        Py_DECREF(code);
        Py_Finalize();
        std::cerr << "Error during disassembly: " + std::string(e.what()) << std::endl;
        return 1;
    }

    Py_DECREF(code);
    Py_Finalize();
    return 0;
}

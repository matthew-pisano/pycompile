//
// Created by matthew on 3/1/26.
//

#ifndef PYCOMPILE_BYTECODE_H
#define PYCOMPILE_BYTECODE_H

#define PY_SSIZE_T_CLEAN
#include <memory>
#include <Python.h>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "bytecode/pythoncode.h"
#include "bytecode/python_opcodes.h"


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
     * The name given to the block of code (e.g. a function label)
     */
    std::string codeName;

    /**
     * The number of args passed to the code object.
     */
    int argcount = 0;

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
 * The type of the instruction argument, which can be one of several types depending on the opcode.
 */
enum class ArgvalType {
    None,
    Bool,
    Int,
    Float,
    Str,
    TupleStr,
    Code
};


/**
 * Helper function to convert an ArgvalType enum value to a human-readable string for debugging and printing purposes.
 * @param type The ArgvalType enum value to convert to a string
 * @return A string representation of the ArgvalType value, such as "Int", "Str", "TupleStr", "Code", or "None".
 */
inline std::string argvalTypeToString(ArgvalType type);


/**
 * Dummy struct to represent the absence of an instruction argument, since some instructions have no argument.
 */
struct ArgvalNone {
};


struct ByteCodeInstruction; // Forward declaration of Instruction struct.

struct ByteCodeModule;

/**
 * A variant type to represent the possible types of instruction arguments, which can be ArgvalNone, a bool, an int64_t,
 * a double_t, a string, a tuple of strings, or a nested code object (represented as a bytecode module).
 */
using Argval = std::variant<ArgvalNone,
                            bool,
                            int64_t,
                            double_t,
                            std::string,
                            std::vector<std::string>,
                            std::shared_ptr<ByteCodeModule> >;


/**
 * A single Python bytecode instruction, as returned by dis.get_instructions().
 */
struct ByteCodeInstruction {
    size_t offset; // Byte offset of this instruction in the code object
    int opcodeId; // The numeric opcode
    PythonOpcode opcode; // The human-readable opcode name
    std::string argrepr; // Human-readable description of the instruction argument, if any
    std::optional<size_t> lineno; // The line number, if available
    bool startsLine; // Whether this instruction starts a new source line
    ArgvalType argvalType; // The type of the instruction argument, which determines how to interpret argval
    Argval argval; // The instruction argument, which can be of various types depending on the opcode
};


/**
 * A bytecode module object, containing its metadata and the list of instructions.
 */
struct ByteCodeModule {
    std::string filename; // The filename associated with the code object, used for error messages and debugging
    std::string moduleName; // The module name associated with the code object, used for error messages and debugging
    CodeInfo info; // Metadata about the code object
    std::vector<ByteCodeInstruction> instructions; // The list of bytecode instructions
};


/**
 * Helper function to print a single instruction in a human-readable format, with indentation for nested code.
 * @param instr The Instruction struct to print
 * @param os The stream to write to
 * @param indentLevel The current indentation level (number of spaces) to use for printing the instruction
 */
void serializeInstruction(const ByteCodeInstruction& instr, std::ostream& os, int indentLevel = 0);


/**
 * Helper function to print the bytecode module in a human-readable format, including metadata and instructions.
 * @param code The ByteCodeModule struct to print
 * @param os The stream to write to
 * @param depth The current recursion depth for nested code objects, used for indentation (default is 0)
 */
void serializeByteCodeModule(const ByteCodeModule& code, std::ostream& os, int depth = 0);


/**
 * Disassemble a Python code object into a ByteCodeModule struct, extracting its metadata and instructions.
 * This function is recursive and will disassemble nested code objects (e.g. lambdas, comprehensions) up to a max depth.
 * @param compiledModule A CompiledModule struct containing the compiled code object to disassemble
 * @param depth The current recursion depth for nested code objects (default is 0)
 * @return A ByteCodeModule struct containing the metadata and instructions of the code object
 * @throws std::runtime_error if the max nested function depth is exceeded, or if there are errors during disassembly.
 */
ByteCodeModule generatePythonByteCode(const CompiledModule& compiledModule, int depth = 0);


/**
 * Compiles a vector of Python code strings down into Python bytecode
 * @param fileContents The vector of Python code to compile
 * @param fileNames The file names corresponding to each Python module
 * @return A vector of compiled bytecode modules
 */
std::vector<ByteCodeModule> compilePython(const std::vector<std::string>& fileContents,
                                          const std::vector<std::string>& fileNames);


/**
 * Compiles a string of Python code down into Python bytecode
 * @param fileContent The string of Python code to compile
 * @param fileName The file name corresponding to the Python module
 * @return A compiled bytecode module
 */
ByteCodeModule compilePython(const std::string& fileContent, const std::string& fileName);

#endif //PYCOMPILE_BYTECODE_H

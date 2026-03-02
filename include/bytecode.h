//
// Created by matthew on 3/1/26.
//

#ifndef PYCOMPILE_BYTECODE_H
#define PYCOMPILE_BYTECODE_H

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <optional>
#include <string>
#include <vector>

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
 * Helper function to print a single instruction in a human-readable format, with indentation for nested code.
 * @param instr The Instruction struct to print
 * @param indentLevel The current indentation level (number of spaces) to use for printing the instruction
 */
void printInstruction(const Instruction& instr, int indentLevel = 0);


/**
 * Helper function to print the disassembled code in a human-readable format, including metadata and instructions.
 * @param code The DisassembledCode struct to print
 * @param depth The current recursion depth for nested code objects, used for indentation (default is 0)
 */
void printDisassembly(const DisassembledCode& code, int depth = 0);


/**
 * Disassemble a Python code object into a DisassembledCode struct, extracting its metadata and instructions.
 * This function is recursive and will disassemble nested code objects (e.g. lambdas, comprehensions) up to a max depth.
 * @param code A Python code object to disassemble
 * @param depth The current recursion depth for nested code objects (default is 0)
 * @return A DisassembledCode struct containing the metadata and instructions of the code object
 * @throws std::runtime_error if the max nested function depth is exceeded, or if there are errors during disassembly.
 */
DisassembledCode disassemble(PyObject* code, int depth = 0);

#endif //PYCOMPILE_BYTECODE_H

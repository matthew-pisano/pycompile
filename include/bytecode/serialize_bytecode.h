//
// Created by matthew on 3/23/26.
//

#ifndef SERIALIZE_BYTECODE_H
#define SERIALIZE_BYTECODE_H
#include "bytecode.h"

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

#endif // SERIALIZE_BYTECODE_H

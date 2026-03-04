//
// Created by matthew on 3/3/26.
//


#include <catch2/catch_all.hpp>

#include "bytecode.h"
#include "pythoncode.h"

/**
 * Extracts a vector of opcodes from a given vector of instruction
 * @param instructions The vector of instructions
 * @return A vector of opcodes from the instructions
 */
std::vector<PythonOpcode> extractInstructionOpcodes(const std::vector<Instruction>& instructions) {
    std::vector<PythonOpcode> opcodes;
    for (Instruction inst : instructions)
        opcodes.push_back(inst.opcode);
    return opcodes;
}

TEST_CASE("Empty Bytecode") {
    const std::string source; // Blank source
    const std::vector expectedOpcodes = {PythonOpcode::RESUME, PythonOpcode::LOAD_CONST, PythonOpcode::RETURN_VALUE};
    const CompiledModule compiledModule = compilePythonSource(source, "<embedded>", "<embedded>");
    const ByteCodeModule bytecodeModule = generatePythonBytecode(compiledModule);
    const std::vector<PythonOpcode> opcodes = extractInstructionOpcodes(bytecodeModule.instructions);

    REQUIRE(bytecodeModule.instructions.size() == 3);
    REQUIRE(opcodes == expectedOpcodes);
}

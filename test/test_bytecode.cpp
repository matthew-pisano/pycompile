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
    opcodes.reserve(instructions.size());
    for (const Instruction& inst : instructions)
        opcodes.push_back(inst.opcode);
    return opcodes;
}

TEST_CASE("Test Empty Bytecode") {
    const std::string source; // Blank source
    const std::vector expectedOpcodes = {PythonOpcode::RESUME, PythonOpcode::LOAD_CONST, PythonOpcode::RETURN_VALUE};
    const CompiledModule compiledModule = compilePythonSource(source, "<embedded>", "<embedded>");
    const ByteCodeModule bytecodeModule = generatePythonBytecode(compiledModule);
    const std::vector<PythonOpcode> opcodes = extractInstructionOpcodes(bytecodeModule.instructions);

    REQUIRE(bytecodeModule.instructions.size() == expectedOpcodes.size());
    REQUIRE(opcodes == expectedOpcodes);
}


TEST_CASE("Test Integer Assign Bytecode") {
    SECTION("Test Small Integer") {
        const std::string source = "x = 42";
        const std::vector expectedOpcodes = {PythonOpcode::RESUME, PythonOpcode::LOAD_SMALL_INT,
                                             PythonOpcode::STORE_NAME,
                                             PythonOpcode::LOAD_CONST, PythonOpcode::RETURN_VALUE};
        const CompiledModule compiledModule = compilePythonSource(source, "<embedded>", "<embedded>");
        const ByteCodeModule bytecodeModule = generatePythonBytecode(compiledModule);
        const std::vector<PythonOpcode> opcodes = extractInstructionOpcodes(bytecodeModule.instructions);

        REQUIRE(bytecodeModule.instructions.size() == expectedOpcodes.size());
        REQUIRE(opcodes == expectedOpcodes);

        const int* instructionInt = std::get_if<int>(&(bytecodeModule.instructions[1].argval));
        REQUIRE(instructionInt != nullptr);
        REQUIRE(*instructionInt == 42);

        REQUIRE(bytecodeModule.instructions[2].argrepr == "x");
    }

    SECTION("Test Large Integer") {
        const std::string source = "x = 2147483646";
        const std::vector expectedOpcodes = {PythonOpcode::RESUME, PythonOpcode::LOAD_CONST,
                                             PythonOpcode::STORE_NAME,
                                             PythonOpcode::LOAD_CONST, PythonOpcode::RETURN_VALUE};
        const CompiledModule compiledModule = compilePythonSource(source, "<embedded>", "<embedded>");
        const ByteCodeModule bytecodeModule = generatePythonBytecode(compiledModule);
        const std::vector<PythonOpcode> opcodes = extractInstructionOpcodes(bytecodeModule.instructions);

        REQUIRE(bytecodeModule.instructions.size() == expectedOpcodes.size());
        REQUIRE(opcodes == expectedOpcodes);
        REQUIRE(bytecodeModule.instructions[1].argrepr == "2147483646");
        REQUIRE(bytecodeModule.instructions[2].argrepr == "x");
    }
}


TEST_CASE("Test Function Bytecode") {
    const std::string source = "def func(x): return x + 1\nfunc(2)";
    const std::vector expectedOpcodes = {PythonOpcode::RESUME, PythonOpcode::LOAD_CONST, PythonOpcode::MAKE_FUNCTION,
                                         PythonOpcode::STORE_NAME, PythonOpcode::LOAD_NAME, PythonOpcode::PUSH_NULL,
                                         PythonOpcode::LOAD_SMALL_INT, PythonOpcode::CALL, PythonOpcode::POP_TOP,
                                         PythonOpcode::LOAD_CONST, PythonOpcode::RETURN_VALUE};
    const CompiledModule compiledModule = compilePythonSource(source, "<embedded>", "<embedded>");
    const ByteCodeModule bytecodeModule = generatePythonBytecode(compiledModule);
    const std::vector<PythonOpcode> opcodes = extractInstructionOpcodes(bytecodeModule.instructions);

    REQUIRE(bytecodeModule.instructions.size() == expectedOpcodes.size());
    REQUIRE(opcodes == expectedOpcodes);
    REQUIRE(bytecodeModule.instructions[3].argrepr == "func");

    const int* instructionInt = std::get_if<int>(&(bytecodeModule.instructions[6].argval));
    REQUIRE(instructionInt != nullptr);
    REQUIRE(*instructionInt == 2);
}


TEST_CASE("Test Builtin Bytecode") {
    SECTION("Test Print Builtin") {
        const std::string source = "print('hello')";
        const std::vector expectedOpcodes = {PythonOpcode::RESUME, PythonOpcode::LOAD_NAME,
                                             PythonOpcode::PUSH_NULL,
                                             PythonOpcode::LOAD_CONST, PythonOpcode::CALL, PythonOpcode::POP_TOP,
                                             PythonOpcode::LOAD_CONST, PythonOpcode::RETURN_VALUE};
        const CompiledModule compiledModule = compilePythonSource(source, "<embedded>", "<embedded>");
        const ByteCodeModule bytecodeModule = generatePythonBytecode(compiledModule);
        const std::vector<PythonOpcode> opcodes = extractInstructionOpcodes(bytecodeModule.instructions);

        REQUIRE(bytecodeModule.instructions.size() == expectedOpcodes.size());
        REQUIRE(opcodes == expectedOpcodes);
        REQUIRE(bytecodeModule.instructions[1].argrepr == "print");
        REQUIRE(bytecodeModule.instructions[3].argrepr == "'hello'");
    }

    SECTION("Test Input Builtin") {
        const std::string source = "input('prompt')";
        const std::vector expectedOpcodes = {PythonOpcode::RESUME, PythonOpcode::LOAD_NAME,
                                             PythonOpcode::PUSH_NULL,
                                             PythonOpcode::LOAD_CONST, PythonOpcode::CALL, PythonOpcode::POP_TOP,
                                             PythonOpcode::LOAD_CONST, PythonOpcode::RETURN_VALUE};
        const CompiledModule compiledModule = compilePythonSource(source, "<embedded>", "<embedded>");
        const ByteCodeModule bytecodeModule = generatePythonBytecode(compiledModule);
        const std::vector<PythonOpcode> opcodes = extractInstructionOpcodes(bytecodeModule.instructions);

        REQUIRE(bytecodeModule.instructions.size() == expectedOpcodes.size());
        REQUIRE(opcodes == expectedOpcodes);
        REQUIRE(bytecodeModule.instructions[1].argrepr == "input");
        REQUIRE(bytecodeModule.instructions[3].argrepr == "'prompt'");
    }
}


TEST_CASE("Test List Comprehension Bytecode") {
    const std::string source = "list1 = [1, 2, 3]\nlistnew = [x for x in list1 if x < 3]";
    const std::vector expectedOpcodes = {PythonOpcode::RESUME, PythonOpcode::BUILD_LIST, PythonOpcode::LOAD_CONST,
                                         PythonOpcode::LIST_EXTEND, PythonOpcode::STORE_NAME, PythonOpcode::LOAD_NAME,
                                         PythonOpcode::GET_ITER, PythonOpcode::LOAD_FAST_AND_CLEAR, PythonOpcode::SWAP,
                                         PythonOpcode::BUILD_LIST, PythonOpcode::SWAP, PythonOpcode::FOR_ITER,
                                         PythonOpcode::STORE_FAST_LOAD_FAST, PythonOpcode::LOAD_SMALL_INT,
                                         PythonOpcode::COMPARE_OP, PythonOpcode::POP_JUMP_IF_TRUE,
                                         PythonOpcode::NOT_TAKEN,
                                         PythonOpcode::JUMP_BACKWARD, PythonOpcode::LOAD_FAST_BORROW,
                                         PythonOpcode::LIST_APPEND,
                                         PythonOpcode::JUMP_BACKWARD, PythonOpcode::END_FOR,
                                         PythonOpcode::POP_ITER, PythonOpcode::SWAP,
                                         PythonOpcode::STORE_FAST, PythonOpcode::STORE_NAME,
                                         PythonOpcode::LOAD_CONST,
                                         PythonOpcode::RETURN_VALUE, PythonOpcode::SWAP, PythonOpcode::POP_TOP,
                                         PythonOpcode::SWAP, PythonOpcode::STORE_FAST,
                                         PythonOpcode::RERAISE};
    const CompiledModule compiledModule = compilePythonSource(source, "<embedded>", "<embedded>");
    const ByteCodeModule bytecodeModule = generatePythonBytecode(compiledModule);
    const std::vector<PythonOpcode> opcodes = extractInstructionOpcodes(bytecodeModule.instructions);

    REQUIRE(bytecodeModule.instructions.size() == expectedOpcodes.size());
    REQUIRE(opcodes == expectedOpcodes);
    REQUIRE(bytecodeModule.instructions[2].argrepr == "(1, 2, 3)");
    REQUIRE(bytecodeModule.instructions[2].argvalType == ArgvalType::TupleStr);
    REQUIRE(bytecodeModule.instructions[4].argrepr == "list1");
    REQUIRE(bytecodeModule.instructions[25].argrepr == "listnew");
}

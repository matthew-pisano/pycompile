//
// Created by matthew on 3/3/26.
//


#include <catch2/catch_all.hpp>

#include "bytecode/bytecode.h"

/**
 * Extracts a vector of opcodes from a given vector of instruction
 * @param instructions The vector of instructions
 * @return A vector of opcodes from the instructions
 */
std::vector<PythonOpcode> extractInstructionOpcodes(const std::vector<ByteCodeInstruction>& instructions) {
    std::vector<PythonOpcode> opcodes;
    opcodes.reserve(instructions.size());
    for (const ByteCodeInstruction& inst : instructions)
        opcodes.push_back(inst.opcode);
    return opcodes;
}

TEST_CASE("Test Empty Bytecode") {
    const std::string source; // Blank source
    const std::vector expectedOpcodes = {PythonOpcode::RESUME, PythonOpcode::LOAD_CONST, PythonOpcode::RETURN_VALUE};
    const ByteCodeModule bytecodeModule = compilePython(source, "<embedded>");
    const std::vector<PythonOpcode> opcodes = extractInstructionOpcodes(bytecodeModule.instructions);

    REQUIRE(opcodes == expectedOpcodes);
}


TEST_CASE("Test Integer Assign Bytecode") {
    SECTION("Test Small Integer") {
        const std::string source = "x = 42";
        const std::vector expectedOpcodes = {PythonOpcode::RESUME, PythonOpcode::LOAD_SMALL_INT,
                                             PythonOpcode::STORE_NAME,
                                             PythonOpcode::LOAD_CONST, PythonOpcode::RETURN_VALUE};
        const ByteCodeModule bytecodeModule = compilePython(source, "<embedded>");
        const std::vector<PythonOpcode> opcodes = extractInstructionOpcodes(bytecodeModule.instructions);

        REQUIRE(opcodes == expectedOpcodes);

        const int64_t* instructionInt = std::get_if<int64_t>(&(bytecodeModule.instructions[1].argval));
        REQUIRE(instructionInt != nullptr);
        REQUIRE(*instructionInt == 42);

        REQUIRE(bytecodeModule.instructions[2].argrepr == "x");
    }

    SECTION("Test Large Integer") {
        const std::string source = "x = 2147483646";
        const std::vector expectedOpcodes = {PythonOpcode::RESUME, PythonOpcode::LOAD_CONST,
                                             PythonOpcode::STORE_NAME,
                                             PythonOpcode::LOAD_CONST, PythonOpcode::RETURN_VALUE};
        const ByteCodeModule bytecodeModule = compilePython(source, "<embedded>");
        const std::vector<PythonOpcode> opcodes = extractInstructionOpcodes(bytecodeModule.instructions);

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
    const ByteCodeModule bytecodeModule = compilePython(source, "<embedded>");
    const std::vector<PythonOpcode> opcodes = extractInstructionOpcodes(bytecodeModule.instructions);

    REQUIRE(opcodes == expectedOpcodes);
    REQUIRE(bytecodeModule.instructions[3].argrepr == "func");

    const int64_t* instructionInt = std::get_if<int64_t>(&(bytecodeModule.instructions[6].argval));
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
        const ByteCodeModule bytecodeModule = compilePython(source, "<embedded>");
        const std::vector<PythonOpcode> opcodes = extractInstructionOpcodes(bytecodeModule.instructions);
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
        const ByteCodeModule bytecodeModule = compilePython(source, "<embedded>");
        const std::vector<PythonOpcode> opcodes = extractInstructionOpcodes(bytecodeModule.instructions);

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
    const ByteCodeModule bytecodeModule = compilePython(source, "<embedded>");
    const std::vector<PythonOpcode> opcodes = extractInstructionOpcodes(bytecodeModule.instructions);

    REQUIRE(opcodes == expectedOpcodes);
    REQUIRE(bytecodeModule.instructions[2].argrepr == "(1, 2, 3)");
    REQUIRE(bytecodeModule.instructions[2].argvalType == ArgvalType::TupleStr);
    REQUIRE(bytecodeModule.instructions[4].argrepr == "list1");
    REQUIRE(bytecodeModule.instructions[25].argrepr == "listnew");
}


TEST_CASE("Test Simple Arithmetic Bytecode") {
    const std::string source = "a = 1\nb = 2\nsummed = a + b\nprint(summed)";
    const std::vector expectedOpcodes = {PythonOpcode::RESUME, PythonOpcode::LOAD_SMALL_INT, PythonOpcode::STORE_NAME,
                                         PythonOpcode::LOAD_SMALL_INT, PythonOpcode::STORE_NAME,
                                         PythonOpcode::LOAD_NAME,
                                         PythonOpcode::LOAD_NAME, PythonOpcode::BINARY_OP, PythonOpcode::STORE_NAME,
                                         PythonOpcode::LOAD_NAME, PythonOpcode::PUSH_NULL, PythonOpcode::LOAD_NAME,
                                         PythonOpcode::CALL, PythonOpcode::POP_TOP,
                                         PythonOpcode::LOAD_CONST, PythonOpcode::RETURN_VALUE};
    const ByteCodeModule bytecodeModule = compilePython(source, "<embedded>");
    const std::vector<PythonOpcode> opcodes = extractInstructionOpcodes(bytecodeModule.instructions);

    REQUIRE(opcodes == expectedOpcodes);

    const int64_t* addendOne = std::get_if<int64_t>(&(bytecodeModule.instructions[1].argval));
    REQUIRE(addendOne != nullptr);
    REQUIRE(*addendOne == 1);
    REQUIRE(bytecodeModule.instructions[2].argrepr == "a");

    const int64_t* addendTwo = std::get_if<int64_t>(&(bytecodeModule.instructions[3].argval));
    REQUIRE(addendTwo != nullptr);
    REQUIRE(*addendTwo == 2);

    REQUIRE(bytecodeModule.instructions[4].argrepr == "b");
    REQUIRE(bytecodeModule.instructions[7].argrepr == "+");
    REQUIRE(bytecodeModule.instructions[8].argrepr == "summed");
    REQUIRE(bytecodeModule.instructions[9].argrepr == "print");
    REQUIRE(bytecodeModule.instructions[11].argrepr == "summed");
}

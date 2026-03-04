//
// Created by matthew on 3/3/26.
//


#include <catch2/catch_all.hpp>

#include "bytecode.h"
#include "pythoncode.h"

TEST_CASE("Empty Bytecode") {
    const std::string source;
    const CompiledModule compiledModule = compilePythonSource(source, "<embedded>", "<embedded>");
    const ByteCodeModule bytecodeModule = generatePythonBytecode(compiledModule);

    REQUIRE(bytecodeModule.instructions.size() == 3);
}

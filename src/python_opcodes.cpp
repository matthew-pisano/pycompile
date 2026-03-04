//
// Created by matthew on 3/3/26.
//

#include "python_opcodes.h"

#include <stdexcept>
#include <unordered_map>


PythonOpcode pythonOpcodeFromString(const std::string& opcodeName) {
    static const std::unordered_map<std::string, PythonOpcode> table = {
#define PYTHON_OPCODE(name, str) { str, PythonOpcode::name },
#include "python_opcodes.inc"
#undef PYTHON_OPCODE
    };
    const auto it = table.find(opcodeName);
    return it != table.end() ? it->second : PythonOpcode::Unknown;
}


std::string pythonOpcodeToString(const PythonOpcode opcode) {
    switch (opcode) {
#define PYTHON_OPCODE(name, str) case PythonOpcode::name: return str;
#include "python_opcodes.inc"
#undef PYTHON_OPCODE
        default:
            return "UNKNOWN";
    }
}

//
// Created by matthew on 3/3/26.
//

#ifndef PYCOMPILE_PYTHON_OPCODES_H
#define PYCOMPILE_PYTHON_OPCODES_H
#include <string>


/**
 * An enum of all supported Python opcodes.
 *
 * Opcode integer values are not guaranteed to be stable across Python versions, and should not be used directly.
 */
enum class PythonOpcode {
#define PYTHON_OPCODE(name, str) name,
#include "bytecode/python_opcodes.inc"
#undef PYTHON_OPCODE
    UNKNOWN
};


/**
 * Convert a string opcode name to a PythonOpcode enum value.
 * @param opcodeName The name of the opcode.
 * @return The corresponding PythonOpcode enum value.
 */
PythonOpcode pythonOpcodeFromString(const std::string& opcodeName);

/**
 * Convert a PythonOpcode enum value back to a string
 * @param opcode The enum value.
 * @return The string representation.
 */
std::string pythonOpcodeToString(PythonOpcode opcode);

inline std::ostream& operator<<(std::ostream& os, const PythonOpcode& opcode) {
    os << pythonOpcodeToString(opcode);
    return os;
}

#endif //PYCOMPILE_PYTHON_OPCODES_H

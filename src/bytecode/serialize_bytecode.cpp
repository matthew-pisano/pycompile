//
// Created by matthew on 3/23/26.
//


#include "bytecode/serialize_bytecode.h"

#include <iomanip>


/**
 * Helper function to convert an ArgvalType enum value to a human-readable string for debugging and printing purposes.
 * @param type The ArgvalType enum value to convert to a string
 * @return A string representation of the ArgvalType value, such as "Int", "Str", "TupleStr", "Code", or "None".
 */
inline std::string argvalTypeToString(const ArgvalType type) {
    switch (type) {
        case ArgvalType::None:
            return "None";
        case ArgvalType::Bool:
            return "Bool";
        case ArgvalType::Int:
            return "Int";
        case ArgvalType::Float:
            return "Float";
        case ArgvalType::Str:
            return "Str";
        case ArgvalType::TupleStr:
            return "TupleStr";
        case ArgvalType::Code:
            return "Code";
        default:
            return "Unknown";
    }
}


void serializeInstruction(const ByteCodeInstruction& instr, std::ostream& os, const int indentLevel) {
    const std::string ind(indentLevel * 4, ' ');

    std::string instRepr;
    if (!instr.argrepr.empty())
        instRepr = instr.argrepr;
    else if (instr.argvalType == ArgvalType::Int)
        instRepr = std::to_string(std::get<int64_t>(instr.argval));
    else if (instr.argvalType == ArgvalType::Float)
        instRepr = std::to_string(std::get<double_t>(instr.argval));
    else if (instr.argvalType == ArgvalType::Str)
        instRepr = std::get<std::string>(instr.argval);
    else if (instr.argvalType == ArgvalType::TupleStr) {
        instRepr = "[tuple]";
    } else if (instr.argvalType == ArgvalType::Code)
        instRepr = "[code object]";
    else
        instRepr = "";

    const std::string argTypeStr = "[" + argvalTypeToString(instr.argvalType) + "]";
    const std::string lineStartStr = instr.startsLine ? "*" : " ";
    const std::string linenoStr = instr.lineno.has_value() ? "L" + std::to_string(*instr.lineno) : "L-";

    os << ind << lineStartStr << linenoStr << " offset " << std::setw(4) << std::left << instr.offset << " | "
       << std::setw(30) << std::left << instr.opcode << " " << std::setw(10) << std::left << argTypeStr << " | "
       << instRepr << std::endl;
}


void serializeByteCodeModule(const ByteCodeModule& code, std::ostream& os, const int depth) {
    const std::string ind(depth * 4, ' ');

    // Print code metadata
    if (!code.info.cellvars.empty()) {
        os << ind << "cellvars: ";
        for (const std::string& v : code.info.cellvars)
            os << v << " ";
        os << "\n";
    }
    if (!code.info.freevars.empty()) {
        os << ind << "freevars: ";
        for (const std::string& v : code.info.freevars)
            os << v << " ";
        os << "\n";
    }
    if (!code.info.exceptionTable.empty()) {
        os << ind << "exception table:\n";
        for (const ExceptionTableEntry& e : code.info.exceptionTable) {
            os << ind << "  [" << e.start << ", " << e.end << ") -> target " << e.target << "  depth " << e.depth
               << "  lasti " << (e.lasti ? "true" : "false") << "\n";
        }
    }

    // Print instructions
    for (const ByteCodeInstruction& instr : code.instructions) {
        serializeInstruction(instr, os, depth);

        // If the instruction has a nested code object, print it recursively with increased indentation.
        if (instr.argvalType == ArgvalType::Code) {
            os << ind << "  [nested code object]:\n";
            // Wrap nested instructions in a temporary DisassembledCode for printing
            if (const auto* nestedPtr = std::get_if<std::shared_ptr<ByteCodeModule>>(&instr.argval)) {
                if (*nestedPtr)
                    serializeByteCodeModule(**nestedPtr, os, depth + 1);
            } else
                throw std::runtime_error("Expected argval to be a vector of Instructions for nested code object");
        }
    }
}

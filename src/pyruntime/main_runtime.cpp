//
// Created by matthew on 3/24/26.
//

#include <iostream>
#include <ranges>

#include "pyruntime/builtin_runtime.h"
#include "pyruntime/objects/py_bool.h"
#include "pyruntime/objects/py_none.h"
#include "pyruntime/objects/py_str.h"

extern "C" void __pymodule(); // Provided by the translated MLIR module

int main() {
    try {
        __pymodule();
        
        // Delete immortal objects
        delete PyNone::None;
        delete PyBool::True;
        delete PyBool::False;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Traceback (most recent call last):" << std::endl;
        const PyStr* file = dynamic_cast<PyStr*>(pyir_builtinVarFile());
        const PyStr* name = dynamic_cast<PyStr*>(pyir_builtinVarName());
        std::cerr << std::format("    File \"{}\", in {}", file->data(), name->data()) << std::endl;
        std::cerr << e.what() << std::endl;
        return 1;
    }
}

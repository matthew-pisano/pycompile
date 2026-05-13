//
// Created by matthew on 3/24/26.
//

#include <iostream>
#include <ranges>

#include "builtin_runtime.h"
#include "pytypes/primitives/py_bool.hpp"
#include "pytypes/primitives/py_none.hpp"
#include "pytypes/primitives/py_str.hpp"

extern "C" void __pymodule(); // Provided by the translated MLIR module

/**
 * Destroys immortal objects that are not managed by reference counting. This should be called at the end of main to
 * prevent memory leaks.
 */
void destroyImmortals() {
    delete PyNone::None;
    delete PyBool::True;
    delete PyBool::False;
}

int main() {
    // Use a custom main function to handle uncaught exceptions and print tracebacks in a Python-like format.
    // Otherwise, uncaught exceptions would SIGABRT with little useful information about the error.
    try {
        __pymodule();
        destroyImmortals();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Traceback (most recent call last):" << std::endl;
        PyStr* file = dynamic_cast<PyStr*>(pyir_builtinVarFile());
        PyStr* name = dynamic_cast<PyStr*>(pyir_builtinVarName());
        std::cerr << std::format("    File \"{}\", in {}", file->data(), name->data()) << std::endl;
        std::cerr << e.what() << std::endl;
        (void) file->decref();
        (void) name->decref();
        pyir_destroyModule();
        // Decrefs all known values until deletion to prevent memory leaks
        pyir_clearDanglingScope();
        destroyImmortals();
        return 1;
    }
}

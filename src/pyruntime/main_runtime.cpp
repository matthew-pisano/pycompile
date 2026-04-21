//
// Created by matthew on 3/24/26.
//

#include <iostream>
#include <ranges>

#include "pyruntime/builtin_runtime.h"
#include "pyruntime/objects/py_bool.h"
#include "pyruntime/objects/py_none.h"
#include "pyruntime/objects/py_str.h"
#include "pyruntime/runtime_state.h"

extern "C" void __pymodule(); // Provided by the translated MLIR module

void destroyExceptionModule() {
    pyir_destroyModule();

    // Clear dangling references in module scope
    for (PyObj*& obj : scopeStack.back() | std::views::values) {
        if (!obj)
            continue; // Skip already nulled objects
        while (!obj->decref()) {
        }
        obj = nullptr;
    }
}

void destroyImmortals() {
    delete PyNone::None;
    delete PyBool::True;
    delete PyBool::False;
}

int main() {
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
        destroyExceptionModule();
        destroyImmortals();
        return 1;
    }
}

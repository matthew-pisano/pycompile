//
// Created by matthew on 3/24/26.
//

#include <iostream>
#include <ranges>

#include "pyruntime/builtin_runtime.h"
#include "pyruntime/objects/py_str.h"
#include "pyruntime/runtime_state.h"

extern "C" void __pymodule(); // Provided by the translated MLIR module

void release() {
    std::cout << "Cleaning up module" << std::endl;
    if (!scopeStack.empty())
        throw std::runtime_error("Exiting with dangling scope");

    for (PyObj*& obj : moduleScope | std::views::values)
        if (obj->decref())
            obj = nullptr;
}

int main() {
    try {
        __pymodule();
        release();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Traceback (most recent call last):" << std::endl;
        const PyStr* file = dynamic_cast<PyStr*>(pyir_builtinVarFile());
        const PyStr* name = dynamic_cast<PyStr*>(pyir_builtinVarName());
        std::cerr << std::format("    File \"{}\", in {}", file->data(), name->data()) << std::endl;
        std::cerr << e.what() << std::endl;
        release();
        return 1;
    }
}

//
// Created by matthew on 3/24/26.
//

#include "function_runtime.h"

#include <format>
#include <stdexcept>

#include "runtime_errors.h"
#include "runtime_state.h"
#include "runtime_util.h"

void pyir_pushScope() {
    // Init new scope with global symbols from the module scope
    scopeStack.emplace_back(scopeStack[0].begin(), scopeStack[0].end());
}


void pyir_popScope() {
    if (scopeStack.empty())
        throw std::runtime_error("Attempting to pop from empty scope stack");
    scopeStack.pop_back();
}


PyFunction* pyir_makeFunction(const char* fnName, void* fn_ptr) {
    return new PyFunction(fnName, reinterpret_cast<PyFunctionData>(fn_ptr));
}


PyObj* pyir_call(PyObj* callee, PyObj** args, const int64_t argc) {
    if (const PyFunction* func = dynamic_cast<const PyFunction*>(callee)) {
        PyObjRef result(func->data()(args, argc));
        if (!builtinFuncs.contains(func->funcName())) {
            // Decref arguments and callee as they go out of scope
            for (int64_t i = 0; i < argc; i++)
                if (args[i]->decref())
                    args[i] = nullptr;
            (void) callee->decref();
        }
        // For builtin functions, the callee is immortal and the arguments are expected to be decrefed by the builtin
        return result.release();
    }
    if (const PyMethod* method = dynamic_cast<const PyMethod*>(callee)) {
        PyObjRef result(method->data()(method->selfObj(), args, argc));
        // Decref arguments and callee as they go out of scope
        for (int64_t i = 0; i < argc; i++)
            if (args[i]->decref())
                args[i] = nullptr;
        (void) callee->decref();
        return result.release();
    }

    const std::string typeName = callee->typeName();
    decrefArgs(args, argc);
    (void) callee->decref();

    throw PyTypeError(std::format("'{}' object is not callable", typeName));
}


void pyir_decref(PyObj* v) {
    if (v)
        (void) v->decref();
}


PyObj* pyir_pushNull() { return nullptr; }

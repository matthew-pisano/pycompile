//
// Created by matthew on 4/8/26.
//

#include "pyir_run_module.h"

#include <stdexcept>

static std::string lastRuntimeError;

int pyir_runModule(void (*moduleFn)()) {
    try {
        moduleFn();
        return 0;
    } catch (const std::exception& e) {
        // Avoid throwing errors over JIT boundary
        lastRuntimeError = e.what();
        return 1;
    }
}

const char* pyir_getLastError() { return lastRuntimeError.c_str(); }

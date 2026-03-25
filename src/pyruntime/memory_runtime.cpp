//
// Created by matthew on 3/24/26.
//

#include "pyruntime/memory_runtime.h"

#include <stdexcept>
#include <unordered_map>

#include "pyruntime/runtime_state.h"

extern "C" {

Value* pyir_loadFast(const char* name) {
    if (scopeStack.empty())
        throw std::runtime_error(std::string("No active scope to load '") + name + "'");

    const std::unordered_map<std::string, Value*>& locals = scopeStack.back();

    const auto it = locals.find(name);
    if (it == locals.end())
        throw std::runtime_error(std::string("Local variable '") + name + "' referenced before assignment");

    return it->second;
}

void pyir_storeFast(const char* name, Value* val) {
    if (scopeStack.empty())
        throw std::runtime_error(std::string("No active scope to store '") + name + "'");
    scopeStack.back()[name] = val;
}

Value* pyir_loadName(const char* name) {
    // Check for builtins
    if (const auto it = builtins.find(name); it != builtins.end())
        return new Value(it->second);
    // Check for names in module scope
    if (const auto it = moduleScope.find(name); it != moduleScope.end()) {
        it->second->incref();
        return it->second;
    }
    throw std::runtime_error(std::string("name '") + name + "' is not defined");
}

void pyir_storeName(const char* name, Value* val) {
    if (const auto it = moduleScope.find(name); it != moduleScope.end())
        it->second->decref(); // Release old value
    val->incref();
    moduleScope[name] = val;
}

Value* pyir_loadConstStr(const char* str) {
    return new Value(std::string(str));
}

Value* pyir_loadConstInt(const int64_t val) {
    return new Value(val);
}

Value* pyir_loadConstFloat(const double_t val) {
    return new Value(val);
}

Value* pyir_loadConstBool(const int8_t val) {
    return new Value(val == 1);
}

Value* pyir_loadConstNone() {
    return new Value(NoneType{});
}

}

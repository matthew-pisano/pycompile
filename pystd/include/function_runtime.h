//
// Created by matthew on 3/24/26.
//

#ifndef PYCOMPILE_FUNCTION_RUNTIME_H
#define PYCOMPILE_FUNCTION_RUNTIME_H
#include <cstdint>

extern "C" {

struct PyObj;
struct PyFunction;


/**
 * Push a new scope onto the scope stack. This is used for function calls and other constructs that require a new scope.
 */
void pyir_pushScope();

/**
 * Pop the current scope from the scope stack. This should be called when exiting a function or other construct that
 * required a new scope.
 */
void pyir_popScope();

/**
 * Returns a new PyFunction object with the given name and function pointer. The function pointer should point to a C++
 * function that takes a PyObj** (array of arguments) and an int64_t (argument count) and returns a PyObj* (the result
 * of the function call).
 * @param fnName The name of the function. This is used for error messages and debugging purposes.
 * @param fn_ptr The function pointer to the C++ function that implements the Python function.
 * @return A new PyFunction object that can be called from Python code.
 */
PyFunction* pyir_makeFunction(const char* fnName, void* fn_ptr);

/**
 * Calls the given callee with the provided arguments. The callee can be a PyFunction* or PyMethod*.
 * @param callee The function or method to call.
 * @param args An array of PyObj* representing the arguments to the function call.
 * @param argc The number of arguments in the args array.
 * @return The result of the function call as a PyObj*. The caller is responsible for managing its reference count.
 * @throws PyTypeError if the callee is not a callable object.
 */
PyObj* pyir_call(PyObj* callee, PyObj** args, int64_t argc);


/**
 * Returns a nullptr as a placeholder.
 */
PyObj* pyir_pushNull();


/**
 * Decrements the reference count of the given PyObj. If the reference count reaches zero, the object is deallocated.
 * @param v The PyObj to decref.
 */
void pyir_decref(PyObj* v);
}

#endif // PYCOMPILE_FUNCTION_RUNTIME_H

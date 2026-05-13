//
// Created by matthew on 3/24/26.
//

#ifndef PYCOMPILE_MEMORY_RUNTIME_H
#define PYCOMPILE_MEMORY_RUNTIME_H
#include <cmath>
#include <cstdint>

extern "C" {

struct PyObj;

/**
 * Fetches the value of a local variable.
 * @param name The name of the local variable
 * @return The value held by the local variable
 */
PyObj* pyir_loadFast(const char* name);

/**
 * Fetches the value of a local variable and clears its value in the current scope.
 * @param name The name of the local variable
 * @return The value previously held by the local variable
 */
PyObj* pyir_loadFastAndClear(const char* name);

/**
 * Stores a value in a local variable.
 * @param name The name of the local variable
 * @param val The value to store in the local variable
 */
void pyir_storeFast(const char* name, PyObj* val);

/**
 * Loads the value of a global variable, which may be a built-in or a user-defined global.
 * @param name The name of the global variable
 * @return The value of the global variable
 */
PyObj* pyir_loadName(const char* name);


/**
 * Stores a value in a global variable, which may be a built-in or a user-defined global.
 * @param name The name of the global variable to store
 * @param val The value to store in the global variable
 */
void pyir_storeName(const char* name, PyObj* val);

/**
 * Loads a constant string value.
 * @param str The string literal to load as a constant
 * @return A PyObj representing the constant string value
 */
PyObj* pyir_loadConstStr(const char* str);

/**
 * Loads a constant integer value.
 * @param val The integer literal to load as a constant
 * @return A PyObj representing the constant integer value
 */
PyObj* pyir_loadConstInt(int64_t val);

/**
 * Loads a constant floating-point value.
 * @param val The floating-point literal to load as a constant
 * @return A PyObj representing the constant floating-point value
 */
PyObj* pyir_loadConstFloat(double_t val);

/**
 * Loads a constant boolean value.
 * @param val The boolean literal to load as a constant (0 for False, non-zero for True)
 * @return A PyObj representing the constant boolean value
 */
PyObj* pyir_loadConstBool(int8_t val);

/**
 * Loads a constant None value.
 * @return A PyObj representing the constant None value
 */
PyObj* pyir_loadConstNone();

/**
 * Loads a constant tuple value.
 * @param items An array of PyObj pointers representing the items in the tuple
 * @param count The number of items in the tuple
 * @return A PyObj representing the constant tuple value
 */
PyObj* pyir_loadConstTuple(PyObj** items, int64_t count);

/**
 * Loads an attribute from an object.
 * @param obj The object from which to load the attribute
 * @param name The name of the attribute to load
 * @return The value of the loaded attribute
 */
PyObj* pyir_loadAttr(PyObj* obj, const char* name);

/**
 * Stores a value at the given index in a container object.
 * @param container The container object (e.g., list, dict) in which to store the value
 * @param idx The index or key at which to store the value
 * @param value The value to store at the specified index or key
 */
void pyir_storeSubscr(PyObj* container, PyObj* idx, PyObj* value);
}

#endif // PYCOMPILE_MEMORY_RUNTIME_H

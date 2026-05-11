//
// Created by matthew on 3/24/26.
//

#ifndef PYCOMPILE_BUILTIN_RUNTIME_H
#define PYCOMPILE_BUILTIN_RUNTIME_H
#include <cstdint>

extern "C" {

struct PyObj;

/**
 * Returns the module name as a new PyStr*.
 */
PyObj* pyir_builtinVarName();

/**
 * Returns the file name as a new PyStr*.
 */
PyObj* pyir_builtinVarFile();

/**
 * Initializes the current module with the file and type names along with clearing the scope and instantiating builtins.
 * @param file The Python file name for the module
 * @param name The name of the Python module
 */
void pyir_initModule(const char* file, const char* name);

/**
 * Destroys a module by clearing the remaining scope and builtin functions.
 */
void pyir_destroyModule();

/**
 * Repeatedly decref all remaining references until they are deleted. Used for ensuring that memory is fully cleared.
 */
void pyir_clearDanglingScope();

/**
 * Prints out each of the given PyObj* arguments separated by spaces.
 */
PyObj* pyir_builtinPrint(PyObj** args, int64_t argc);

/**
 * Returns the length of the given object as a new PyInt*.
 */
PyObj* pyir_builtinLen(PyObj** args, int64_t argc);

/**
 * Converts the given object to an integer, if possible, and returns it as a new PyInt*.
 */
PyObj* pyir_builtinInt(PyObj** args, int64_t argc);

/**
 * Converts the given object to a float, if possible, and returns it as a new PyFloat*.
 */
PyObj* pyir_builtinFloat(PyObj** args, int64_t argc);

/**
 * Converts the given object to a string and returns it as a new PyStr*.
 */
PyObj* pyir_builtinStr(PyObj** args, int64_t argc);

/**
 * Converts the given object to a boolean and returns it as a new PyBool*.
 */
PyObj* pyir_builtinBool(PyObj** args, int64_t argc);

/**
 * Converts the given object to a list, if possible, and returns it as a new PyList*.
 */
PyObj* pyir_builtinList(PyObj** args, int64_t argc);

/**
 * Converts the given object to a set, if possible, and returns it as a new PySet*.
 */
PyObj* pyir_builtinSet(PyObj** args, int64_t argc);

/**
 * Converts the given object to a tuple, if possible, and returns it as a new PyTuple*.
 */
PyObj* pyir_builtinTuple(PyObj** args, int64_t argc);

/**
 * Converts the given object to a dict, if possible, and returns it as a new PyDict*.
 */
PyObj* pyir_builtinDict(PyObj** args, int64_t argc);

/**
 * Interprets the given object as an iterable and returns an iterator for it, if possible, as a new PyIter* of the
 * correct type.
 */
PyObj* pyir_builtinIter(PyObj** args, int64_t argc);

/**
 * Returns the next element from the given iterator as a new PyObj*.
 * @throws PyStopIteration if the iterator is exhausted
 */
PyObj* pyir_builtinNext(PyObj** args, int64_t argc);

/**
 * Returns an enumerate object for the given iterable as a new PyList* that contains PyTuple* of (index, element).
 */
PyObj* pyir_builtinEnumerate(PyObj** args, int64_t argc);

/**
 * Returns True if the given instance is an instance of the given type, and False otherwise.
 *
 * The second argument must be a type, if the second argument is not a type, a PyTypeError is thrown.
 */
PyObj* pyir_builtinIsInstance(PyObj** args, int64_t argc);

/**
 * Returns a list of integers from the given start index (inclusive) to the given end index (exclusive).
 *
 * If only one argument is given, it is interpreted as the end index and the start index is set to 0.
 */
PyObj* pyir_builtinRange(PyObj** args, int64_t argc);

/**
 * Returns the type of the given object as a new PyStr*.
 */
PyObj* pyir_builtinType(PyObj** args, int64_t argc);

/**
 * Returns a list of tuples, where the i-th tuple contains the i-th element from each of the argument iterables.
 *
 * The returned list is truncated in length to the length of the shortest argument iterable.
 */
PyObj* pyir_builtinZip(PyObj** args, int64_t argc);

/**
 * Reads a line of input from the user and returns it as a new PyStr*.
 *
 * If an optional prompt string is given, it is printed to the user before reading input.
 */
PyObj* pyir_builtinInput(PyObj** args, int64_t argc);
}

#endif // PYCOMPILE_BUILTIN_RUNTIME_H

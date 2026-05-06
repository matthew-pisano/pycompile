//
// Created by matthew on 3/24/26.
//

#ifndef PYCOMPILE_LOGICAL_RUNTIME_H
#define PYCOMPILE_LOGICAL_RUNTIME_H
#include <cstdint>

extern "C" {

struct PyBool;
struct PyStr;
struct PyFloat;
struct PyObj;
struct PyInt;

/**
 * Determine the truthiness of a value, used by conditional jumps.
 * @param val The value to check the truthiness of.
 * @return 1 if the value is truthy, 0 if the value is falsy.
 */
int8_t pyir_isTruthy(PyObj* val);


/**
 * Performs the + operation between the two given objects.
 */
PyObj* pyir_add(PyObj* lhs, PyObj* rhs);

/**
 * Performs the - operation between the two given objects.
 */
PyObj* pyir_sub(PyObj* lhs, PyObj* rhs);

/**
 * Performs the * operation between the two given objects.
 */
PyObj* pyir_mul(PyObj* lhs, PyObj* rhs);

/**
 * Performs the / operation between the two given objects.
 */
PyFloat* pyir_div(PyObj* lhs, PyObj* rhs);

/**
 * Performs the // operation between the two given objects.
 */
PyObj* pyir_floorDiv(PyObj* lhs, PyObj* rhs);

/**
 * Performs the +** operation between the two given objects.
 */
PyObj* pyir_exp(PyObj* lhs, PyObj* rhs);

/**
 * Performs the % operation between the two given objects.
 */
PyObj* pyir_mod(PyObj* lhs, PyObj* rhs);

/**
 * Performs the ^ operation between the two given objects.
 */
PyObj* pyir_xor(PyObj* lhs, PyObj* rhs);

/**
 * Performs the | operation between the two given objects.
 */
PyObj* pyir_pipe(PyObj* lhs, PyObj* rhs);

/**
 * Performs the & operation between the two given objects.
 */
PyObj* pyir_ampersand(PyObj* lhs, PyObj* rhs);


/**
 * Indexes the given object with the given index.
 */
PyObj* pyir_idx(PyObj* obj, PyObj* idx);


/**
 * Returns a new PyBool* representing whether the given element is in the given container.
 */
PyBool* pyir_in(PyObj* container, PyObj* element);


/**
 * Performs the == operation between the two given objects.
 */
PyBool* pyir_eq(PyObj* lhs, PyObj* rhs);

/**
 * Performs the != operation between the two given objects.
 */
PyBool* pyir_ne(PyObj* lhs, PyObj* rhs);

/**
 * Performs the < operation between the two given objects.
 */
PyBool* pyir_lt(PyObj* lhs, PyObj* rhs);

/**
 * Performs the <= operation between the two given objects.
 */
PyBool* pyir_le(PyObj* lhs, PyObj* rhs);

/**
 * Performs the > operation between the two given objects.
 */
PyBool* pyir_gt(PyObj* lhs, PyObj* rhs);

/**
 * Performs the &>= operation between the two given objects.
 */
PyBool* pyir_ge(PyObj* lhs, PyObj* rhs);

/**
 * Performs the - operation between the given value.
 */
PyObj* pyir_unaryNegative(PyObj* val);

/**
 * Performs the not operation between the given value.
 */
PyObj* pyir_unaryNot(PyObj* val);

/**
 * Performs the ~ operation between the given value.
 */
PyInt* pyir_unaryInvert(PyObj* val);

/**
 * Returns the boolean truthiness of the given value.
 */
PyBool* pyir_toBool(PyObj* val);

/**
 * Formats the given value as a new PyStr* object.
 */
PyStr* pyir_formatSimple(PyObj* val);
}

#endif // PYCOMPILE_LOGICAL_RUNTIME_H

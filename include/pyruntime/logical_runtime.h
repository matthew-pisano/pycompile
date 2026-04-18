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

int8_t pyir_isTruthy(PyObj* val);

// arithmetic
PyObj* pyir_add(PyObj* lhs, PyObj* rhs);

PyObj* pyir_sub(PyObj* lhs, PyObj* rhs);

PyObj* pyir_mul(PyObj* lhs, PyObj* rhs);

PyFloat* pyir_div(PyObj* lhs, PyObj* rhs);

PyObj* pyir_floorDiv(PyObj* lhs, PyObj* rhs);

PyObj* pyir_exp(PyObj* lhs, PyObj* rhs);

PyObj* pyir_mod(PyObj* lhs, PyObj* rhs);

PyObj* pyir_xor(PyObj* lhs, PyObj* rhs);

PyObj* pyir_pipe(PyObj* lhs, PyObj* rhs);

PyObj* pyir_ampersand(PyObj* lhs, PyObj* rhs);

// index
PyObj* pyir_idx(PyObj* obj, PyObj* idx);

// membership
PyBool* pyir_in(PyObj* container, PyObj* element);

// comparison
PyBool* pyir_eq(PyObj* lhs, PyObj* rhs);

PyBool* pyir_ne(PyObj* lhs, PyObj* rhs);

PyBool* pyir_lt(PyObj* lhs, PyObj* rhs);

PyBool* pyir_le(PyObj* lhs, PyObj* rhs);

PyBool* pyir_gt(PyObj* lhs, PyObj* rhs);

PyBool* pyir_ge(PyObj* lhs, PyObj* rhs);

// unary operations
PyObj* pyir_unaryNegative(PyObj* val);

PyObj* pyir_unaryNot(PyObj* val);

PyInt* pyir_unaryInvert(PyObj* val);

// truthiness, used by conditional jumps
PyBool* pyir_toBool(PyObj* val);

PyStr* pyir_formatSimple(PyObj* val);
}

#endif // PYCOMPILE_LOGICAL_RUNTIME_H

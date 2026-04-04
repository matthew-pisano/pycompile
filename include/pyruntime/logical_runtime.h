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

int8_t pyir_isTruthy(const PyObj* val);

// arithmetic
PyObj* pyir_add(const PyObj* lhs, const PyObj* rhs);

PyObj* pyir_sub(const PyObj* lhs, const PyObj* rhs);

PyObj* pyir_mul(const PyObj* lhs, const PyObj* rhs);

PyFloat* pyir_div(const PyObj* lhs, const PyObj* rhs);

PyObj* pyir_floorDiv(const PyObj* lhs, const PyObj* rhs);

PyObj* pyir_exp(const PyObj* lhs, const PyObj* rhs);

PyObj* pyir_mod(const PyObj* lhs, const PyObj* rhs);

PyObj* pyir_xor(const PyObj* lhs, const PyObj* rhs);

PyObj* pyir_pipe(const PyObj* lhs, const PyObj* rhs);

PyObj* pyir_ampersand(const PyObj* lhs, const PyObj* rhs);

// index
PyObj* pyir_idx(const PyObj* obj, const PyObj* idx);

// membership
PyBool* pyir_in(const PyObj* container, const PyObj* element);

// comparison
PyBool* pyir_eq(const PyObj* lhs, const PyObj* rhs);

PyBool* pyir_ne(const PyObj* lhs, const PyObj* rhs);

PyBool* pyir_lt(const PyObj* lhs, const PyObj* rhs);

PyBool* pyir_le(const PyObj* lhs, const PyObj* rhs);

PyBool* pyir_gt(const PyObj* lhs, const PyObj* rhs);

PyBool* pyir_ge(const PyObj* lhs, const PyObj* rhs);

// unary operations
PyObj* pyir_unaryNegative(const PyObj* val);

PyObj* pyir_unaryNot(const PyObj* val);

PyInt* pyir_unaryInvert(const PyObj* val);

// truthiness, used by conditional jumps
PyBool* pyir_toBool(const PyObj* val);

PyStr* pyir_formatSimple(const PyObj* val);
}

#endif // PYCOMPILE_LOGICAL_RUNTIME_H

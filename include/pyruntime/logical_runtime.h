//
// Created by matthew on 3/24/26.
//

#ifndef PYCOMPILE_LOGICAL_RUNTIME_H
#define PYCOMPILE_LOGICAL_RUNTIME_H
#include <cstdint>

extern "C" {

struct PyObj;

int8_t pyir_isTruthy(const PyObj* val);

// arithmetic
PyObj* pyir_add(const PyObj* lhs, const PyObj* rhs);

PyObj* pyir_sub(const PyObj* lhs, const PyObj* rhs);

PyObj* pyir_mul(const PyObj* lhs, const PyObj* rhs);

PyObj* pyir_div(const PyObj* lhs, const PyObj* rhs);

PyObj* pyir_floorDiv(const PyObj* lhs, const PyObj* rhs);

PyObj* pyir_exp(const PyObj* lhs, const PyObj* rhs);

PyObj* pyir_mod(const PyObj* lhs, const PyObj* rhs);

// index
PyObj* pyir_idx(PyObj* obj, const PyObj* idx);

// comparison
PyObj* pyir_eq(const PyObj* lhs, const PyObj* rhs);

PyObj* pyir_ne(const PyObj* lhs, const PyObj* rhs);

PyObj* pyir_lt(const PyObj* lhs, const PyObj* rhs);

PyObj* pyir_le(const PyObj* lhs, const PyObj* rhs);

PyObj* pyir_gt(const PyObj* lhs, const PyObj* rhs);

PyObj* pyir_ge(const PyObj* lhs, const PyObj* rhs);

// unary operations
PyObj* pyir_unaryNegative(const PyObj* val);

PyObj* pyir_unaryNot(const PyObj* val);

PyObj* pyir_unaryInvert(const PyObj* val);

PyObj* pyir_xor(const PyObj* lhs, const PyObj* rhs);

// truthiness, used by conditional jumps
PyObj* pyir_toBool(const PyObj* val);

PyObj* pyir_formatSimple(const PyObj* val);
}

#endif // PYCOMPILE_LOGICAL_RUNTIME_H

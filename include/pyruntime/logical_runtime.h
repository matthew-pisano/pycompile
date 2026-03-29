//
// Created by matthew on 3/24/26.
//

#ifndef PYCOMPILE_LOGICAL_RUNTIME_H
#define PYCOMPILE_LOGICAL_RUNTIME_H
#include "runtime_value.h"

extern "C" {

int8_t pyir_isTruthy(const PyValue* val);

// arithmetic
PyValue* pyir_add(const PyValue* lhs, const PyValue* rhs);

PyValue* pyir_sub(const PyValue* lhs, const PyValue* rhs);

PyValue* pyir_mul(const PyValue* lhs, const PyValue* rhs);

PyValue* pyir_div(const PyValue* lhs, const PyValue* rhs);

PyValue* pyir_floorDiv(const PyValue* lhs, const PyValue* rhs);

PyValue* pyir_exp(const PyValue* lhs, const PyValue* rhs);

PyValue* pyir_mod(const PyValue* lhs, const PyValue* rhs);

// index
PyValue* pyir_idx(PyValue* obj, const PyValue* idx);

// comparison
PyValue* pyir_eq(const PyValue* lhs, const PyValue* rhs);

PyValue* pyir_ne(const PyValue* lhs, const PyValue* rhs);

PyValue* pyir_lt(const PyValue* lhs, const PyValue* rhs);

PyValue* pyir_le(const PyValue* lhs, const PyValue* rhs);

PyValue* pyir_gt(const PyValue* lhs, const PyValue* rhs);

PyValue* pyir_ge(const PyValue* lhs, const PyValue* rhs);

// unary operations
PyValue* pyir_unaryNegative(const PyValue* val);

PyValue* pyir_unaryNot(const PyValue* val);

PyValue* pyir_unaryInvert(const PyValue* val);

PyValue* pyir_xor(const PyValue* lhs, const PyValue* rhs);

// truthiness, used by conditional jumps
PyValue* pyir_toBool(const PyValue* val);

PyValue* pyir_formatSimple(const PyValue* val);
}

#endif // PYCOMPILE_LOGICAL_RUNTIME_H

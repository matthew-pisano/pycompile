//
// Created by matthew on 3/24/26.
//

#ifndef PYCOMPILE_LOGICAL_RUNTIME_H
#define PYCOMPILE_LOGICAL_RUNTIME_H
#include "pyir/pyir_value.h"

extern "C" {

int8_t pyir_isTruthy(const Value* val);

// arithmetic
Value* pyir_add(const Value* lhs, const Value* rhs);

Value* pyir_sub(const Value* lhs, const Value* rhs);

Value* pyir_mul(const Value* lhs, const Value* rhs);

Value* pyir_div(const Value* lhs, const Value* rhs);

Value* pyir_floorDiv(const Value* lhs, const Value* rhs);

Value* pyir_exp(const Value* lhs, const Value* rhs);

Value* pyir_mod(const Value* lhs, const Value* rhs);

// comparison
Value* pyir_eq(const Value* lhs, const Value* rhs);

Value* pyir_ne(const Value* lhs, const Value* rhs);

Value* pyir_lt(const Value* lhs, const Value* rhs);

Value* pyir_le(const Value* lhs, const Value* rhs);

Value* pyir_gt(const Value* lhs, const Value* rhs);

Value* pyir_ge(const Value* lhs, const Value* rhs);

// unary operations
Value* pyir_unaryNegative(const Value* val);

Value* pyir_unaryNot(const Value* val);

Value* pyir_unaryInvert(const Value* val);

Value* pyir_xor(const Value* lhs, const Value* rhs);

// truthiness, used by conditional jumps
Value* pyir_toBool(const Value* val);

Value* pyir_formatSimple(const Value* val);
}

#endif // PYCOMPILE_LOGICAL_RUNTIME_H

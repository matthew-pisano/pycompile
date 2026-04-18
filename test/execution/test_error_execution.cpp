//
// Created by matthew on 3/22/26.
//

#include <catch2/catch_all.hpp>

#include "execution_test_utils.h"


TEST_CASE_METHOD(JITFixture, "Test Unknown Identifier Error") {
    REQUIRE_THROWS_WITH(runCapture("a = b"), "name 'b' is not defined");
}

TEST_CASE_METHOD(JITFixture, "Test Invalid Binary Operands Error") {
    REQUIRE_THROWS_WITH(runCapture("a = 1 - 'hello'"), "Unsupported operand type(s) for -: 'int' and 'str'");
}

TEST_CASE_METHOD(JITFixture, "Test Invalid Unary Operands Error") {
    REQUIRE_THROWS_WITH(runCapture("a = ~'hello'"), "Bad operand type for ~: 'str'");
}

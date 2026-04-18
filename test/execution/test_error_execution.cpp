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

TEST_CASE_METHOD(JITFixture, "Test Bad Conversion Error") {
    REQUIRE_THROWS_WITH(runCapture("a = float('not a number')"), "Could not convert str to float: 'not a number'");
}

TEST_CASE_METHOD(JITFixture, "Test Bad Iterable Error") {
    REQUIRE_THROWS_WITH(runCapture("for i in 1:\n ..."), "Could not convert int to iter: '1'");
}

TEST_CASE_METHOD(JITFixture, "Test Invalid Arguments Error") {
    REQUIRE_THROWS_WITH(runCapture("a = int(1, 1)"), "int() takes exactly one argument");
}

TEST_CASE_METHOD(JITFixture, "Test Non Callable Object Error") {
    REQUIRE_THROWS_WITH(runCapture("a = 1()"), "'int' object is not callable");
}

TEST_CASE_METHOD(JITFixture, "Test Unknown Attribute Error") {
    REQUIRE_THROWS_WITH(runCapture("a = 'str'.attr()"), "'str' object has no attribute 'attr'");
}

TEST_CASE_METHOD(JITFixture, "Test Index Out of Range Error") {
    REQUIRE_THROWS_WITH(runCapture("a = [1, 2][3]"), "list index out of range");

    REQUIRE_THROWS_WITH(runCapture("a = [1, 2][-3]"), "list index out of range");
}

TEST_CASE_METHOD(JITFixture, "Test Bad Index Type Error") {
    REQUIRE_THROWS_WITH(runCapture("a = [1, 2]['bad_idx']"), "list indices must be integers");
}

TEST_CASE_METHOD(JITFixture, "Test Bad Next Type Error") {
    REQUIRE_THROWS_WITH(runCapture("a = next(1)"), "Could not convert int to iter: '1'");
}

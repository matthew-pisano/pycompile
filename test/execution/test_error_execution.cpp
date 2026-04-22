//
// Created by matthew on 3/22/26.
//

#include <catch2/catch_all.hpp>

#include "execution_test_utils.h"


TEST_CASE_METHOD(JITFixture, "Test Unknown Identifier Error") {
    REQUIRE_THROWS_WITH(runCapture("a = b"), "NameError: name 'b' is not defined");
}

TEST_CASE_METHOD(JITFixture, "Test Invalid Binary Operands Error") {
    REQUIRE_THROWS_WITH(runCapture("a = 1 - 'hello'"), "TypeError: Unsupported operand type(s) for -: 'int' and 'str'");
}

TEST_CASE_METHOD(JITFixture, "Test Invalid Unary Operands Error") {
    REQUIRE_THROWS_WITH(runCapture("a = ~'hello'"), "TypeError: Bad operand type for ~: 'str'");
}

TEST_CASE_METHOD(JITFixture, "Test Bad Conversion Error") {
    REQUIRE_THROWS_WITH(runCapture("a = float('not a number')"),
                        "TypeError: Could not convert str to float: 'not a number'");
}

TEST_CASE_METHOD(JITFixture, "Test Bad Iterable Error") {
    REQUIRE_THROWS_WITH(runCapture("for i in 1:\n ..."), "TypeError: Could not convert int to iter: '1'");
}

TEST_CASE_METHOD(JITFixture, "Test Invalid Arguments Error") {
    REQUIRE_THROWS_WITH(runCapture("a = int(1, 1)"), "TypeError: int() takes exactly one argument");
}

TEST_CASE_METHOD(JITFixture, "Test Non Callable Object Error") {
    REQUIRE_THROWS_WITH(runCapture("a = 1()"), "TypeError: 'int' object is not callable");
}

TEST_CASE_METHOD(JITFixture, "Test Unknown Attribute Error") {
    REQUIRE_THROWS_WITH(runCapture("a = 'str'.attr()"), "AttributeError: 'str' object has no attribute 'attr'");

    REQUIRE_THROWS_WITH(runCapture("a = {}.attr()"), "AttributeError: 'dict' object has no attribute 'attr'");
}

TEST_CASE_METHOD(JITFixture, "Test Index Out of Range Error") {
    REQUIRE_THROWS_WITH(runCapture("a = [1, 2][3]"), "IndexError: list index out of range");

    REQUIRE_THROWS_WITH(runCapture("a = [1, 2][-3]"), "IndexError: list index out of range");

    REQUIRE_THROWS_WITH(runCapture("a = [1, 2]\na[3] = 0"), "IndexError: list index out of range");
}

TEST_CASE_METHOD(JITFixture, "Test Bad Index Type Error") {
    REQUIRE_THROWS_WITH(runCapture("a = [1, 2]['bad_idx']"), "TypeError: list indices must be integers, not str");
}

TEST_CASE_METHOD(JITFixture, "Test Bad Next Type Error") {
    REQUIRE_THROWS_WITH(runCapture("a = next(1)"), "TypeError: Could not convert int to iter: '1'");
}

TEST_CASE_METHOD(JITFixture, "Test Dict Key Error") {
    REQUIRE_THROWS_WITH(runCapture("a = {'': 1}[2]"), "KeyError: 2");
}

//
// Created by matthew on 4/6/26.
//

#include <catch2/catch_all.hpp>

#include "execution_test_utils.h"


TEST_CASE_METHOD(JITFixture, "Test JIT Iter Str Creation") {
    const std::string output = runCapture("print(iter('123'))");
    REQUIRE(output == "<iterator>\n");
}

TEST_CASE_METHOD(JITFixture, "Test JIT Iter List Creation") {
    const std::string output = runCapture("print(iter([1, 2, 3]))");
    REQUIRE(output == "<iterator>\n");
}

TEST_CASE_METHOD(JITFixture, "Test JIT Iter Set Creation") {
    const std::string output = runCapture("print(iter({1, 2, 3}))");
    REQUIRE(output == "<iterator>\n");
}

TEST_CASE_METHOD(JITFixture, "Test JIT Iter Dict Creation") {
    const std::string output = runCapture("print(iter({1: 'one', 2: 'two', 3: 'three'}))");
    REQUIRE(output == "<iterator>\n");
}

TEST_CASE_METHOD(JITFixture, "Test JIT Iter Tuple Creation") {
    const std::string output = runCapture("print(iter((1, 2, 3)))");
    REQUIRE(output == "<iterator>\n");
}

TEST_CASE_METHOD(JITFixture, "Test JIT Empty Iterator") {
    REQUIRE_THROWS_WITH(runCapture("a = iter([])\nnext(a)"), "StopIteration()");
}

TEST_CASE_METHOD(JITFixture, "Test JIT Iterator") {
    const std::string output = runCapture("a = iter([1, 2, 3])\nprint(next(a))\nprint(next(a))\nprint(next(a))");
    REQUIRE(output == "1\n2\n3\n");
}

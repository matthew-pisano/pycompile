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

TEST_CASE_METHOD(JITFixture, "Test JIT Str Iteration") {
    const std::string output = runCapture("a = iter('123')\nprint(next(a))\nprint(next(a))\nprint(next(a))");
    REQUIRE(output == "1\n2\n3\n");
}

TEST_CASE_METHOD(JITFixture, "Test JIT List Iteration") {
    const std::string output = runCapture("a = iter([1, 2, 3])\nprint(next(a))\nprint(next(a))\nprint(next(a))");
    REQUIRE(output == "1\n2\n3\n");
}

TEST_CASE_METHOD(JITFixture, "Test JIT Set Iteration") {
    const std::string output = runCapture("a = iter({1, 2, 3})\nprint(next(a))\nprint(next(a))\nprint(next(a))");
    REQUIRE(output.length() == 6);
    REQUIRE(output.contains("1\n"));
    REQUIRE(output.contains("2\n"));
    REQUIRE(output.contains("3\n"));
}

TEST_CASE_METHOD(JITFixture, "Test JIT Dict Iteration") {
    const std::string output =
            runCapture("a = iter({1: 'one', 2: 'two', 3: 'three'})\nprint(next(a))\nprint(next(a))\nprint(next(a))");
    REQUIRE(output == "1\n2\n3\n");
}

TEST_CASE_METHOD(JITFixture, "Test JIT Tuple Iteration") {
    const std::string output = runCapture("a = iter((1, 2, 3))\nprint(next(a))\nprint(next(a))\nprint(next(a))");
    REQUIRE(output == "1\n2\n3\n");
}

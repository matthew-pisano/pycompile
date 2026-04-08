//
// Created by matthew on 4/6/26.
//

#include <catch2/catch_all.hpp>

#include "execution_test_utils.h"


TEST_CASE_METHOD(JITFixture, "Test JIT Load Tuple") {
    const std::string output = runCapture("print(tuple())");
    REQUIRE(output == "()\n");
}

TEST_CASE_METHOD(JITFixture, "Test JIT Single Element Tuple") {
    const std::string output = runCapture("print((2,))");
    REQUIRE(output == "(2,)\n");
}

TEST_CASE_METHOD(JITFixture, "Test JIT Multi Element Tuple") {
    const std::string output = runCapture("print((1, 2, 3))");
    REQUIRE(output == "(1, 2, 3)\n");
}

TEST_CASE_METHOD(JITFixture, "Test JIT Tuple Equality") {
    std::string output = runCapture("print((1, 2, 3) == (1, 2, 3))");
    REQUIRE(output == "True\n");

    output = runCapture("print((1, 3, 2) == (1, 2, 3))");
    REQUIRE(output == "False\n");

    output = runCapture("print((1, 2, 3) == (3,))");
    REQUIRE(output == "False\n");
}

TEST_CASE_METHOD(JITFixture, "Test JIT Tuple Add") {
    const std::string output = runCapture("print((1, 2, 3) + (4,))");
    REQUIRE(output == "(1, 2, 3, 4)\n");
}

TEST_CASE_METHOD(JITFixture, "Test JIT Tuple Membership") {
    const std::string output = runCapture("a = (1, 2)\nprint(1 in a)");
    REQUIRE(output == "True\n");
}

TEST_CASE_METHOD(JITFixture, "Test JIT Tuple Index") {
    const std::string output = runCapture("a = (1, 2)\nprint(a[0])");
    REQUIRE(output == "1\n");
}

TEST_CASE_METHOD(JITFixture, "Test JIT Tuple Length") {
    const std::string output = runCapture("print(len((1, 2)))");
    REQUIRE(output == "2\n");
}

TEST_CASE_METHOD(JITFixture, "Test JIT String as Tuple") {
    const std::string output = runCapture("print(tuple('Hello'))");
    REQUIRE(output == "('H', 'e', 'l', 'l', 'o')\n");
}

TEST_CASE_METHOD(JITFixture, "Test JIT List as Tuple") {
    const std::string output = runCapture("print(tuple([1, 2, 3]))");
    REQUIRE(output == "(1, 2, 3)\n");
}

TEST_CASE_METHOD(JITFixture, "Test JIT Set as Tuple") {
    const std::string output =
            runCapture("a = tuple({1, 2, 3})\nprint(len(a) == 3 and (1 in a) and (2 in a) and (3 in a))");
    REQUIRE(output == "True\n");
}

//
// Created by matthew on 4/6/26.
//

#include <catch2/catch_all.hpp>

#include "execution_test_utils.h"


TEST_CASE_METHOD(JITFixture, "Test JIT Build Set") {
    const std::string output = runCapture("print(set())");
    REQUIRE(output == "set()\n");
}

TEST_CASE_METHOD(JITFixture, "Test JIT Set Update Op") {
    const std::string output = runCapture("print({1, 1, 1})");
    REQUIRE(output == "{1}\n");
}

TEST_CASE_METHOD(JITFixture, "Test JIT Set Add Op") {
    const std::string output = runCapture(
            "print({1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1})");
    REQUIRE(output == "{1}\n");
}

TEST_CASE_METHOD(JITFixture, "Test JIT Set Equality") {
    std::string output = runCapture("print({1, 2, 3} == {1, 2, 3})");
    REQUIRE(output == "True\n");

    output = runCapture("print({1, 3, 2} == {1, 2, 3})");
    REQUIRE(output == "True\n");

    output = runCapture("print({1, 2, 3} == {3})");
    REQUIRE(output == "False\n");
}

TEST_CASE_METHOD(JITFixture, "Test JIT Set Union") {
    const std::string output = runCapture("print({1, 2} | {3} == {1, 2, 3})");
    REQUIRE(output == "True\n");
}

TEST_CASE_METHOD(JITFixture, "Test JIT Set Intersection") {
    const std::string output = runCapture("print({1, 2} & {2, 3} == {2})");
    REQUIRE(output == "True\n");
}

TEST_CASE_METHOD(JITFixture, "Test JIT Set Difference") {
    const std::string output = runCapture("print({1, 2, 3} - {1, 4} == {2, 3})");
    REQUIRE(output == "True\n");
}

TEST_CASE_METHOD(JITFixture, "Test JIT Set Symmetric Difference") {
    const std::string output = runCapture("print({1, 2} ^ {2, 3} == {1, 3})");
    REQUIRE(output == "True\n");
}

TEST_CASE_METHOD(JITFixture, "Test JIT Set Membership") {
    const std::string output = runCapture("a = {1, 2}\nprint(1 in a)");
    REQUIRE(output == "True\n");
}

TEST_CASE_METHOD(JITFixture, "Test JIT Set Length") {
    const std::string output = runCapture("print(len({1, 2}))");
    REQUIRE(output == "2\n");
}

TEST_CASE_METHOD(JITFixture, "Test JIT Set Add") {
    const std::string output = runCapture("a = {1, 2}\na.add(3)\nprint(a == {1, 2, 3})");
    REQUIRE(output == "True\n");
}

TEST_CASE_METHOD(JITFixture, "Test JIT Set Update") {
    const std::string output = runCapture("a = {1, 2}\na.update({3})\nprint(a == {1, 2, 3})");
    REQUIRE(output == "True\n");
}

TEST_CASE_METHOD(JITFixture, "Test JIT String as Set") {
    const std::string output = runCapture("print(set('Hello') == {'H', 'e', 'l', 'o'})");
    REQUIRE(output == "True\n");
}

TEST_CASE_METHOD(JITFixture, "Test JIT List as Set") {
    const std::string output = runCapture("a = set([1, 2, 3])\nprint(a == {1, 2, 3})");
    REQUIRE(output == "True\n");
}

TEST_CASE_METHOD(JITFixture, "Test JIT Tuple as Set") {
    const std::string output = runCapture("a = set((1, 2, 3))\nprint(a == {1, 2, 3})");
    REQUIRE(output == "True\n");
}

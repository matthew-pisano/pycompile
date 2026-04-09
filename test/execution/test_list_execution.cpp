//
// Created by matthew on 4/6/26.
//

#include <catch2/catch_all.hpp>

#include "execution_test_utils.h"


TEST_CASE_METHOD(JITFixture, "Test JIT Build List") {
    std::string output = runCapture("print([])");
    REQUIRE(output == "[]\n");
    output = runCapture("print(list())");
    REQUIRE(output == "[]\n");
}

TEST_CASE_METHOD(JITFixture, "Test JIT List Extend Op") {
    const std::string output = runCapture("print([1, 2, 3])");
    REQUIRE(output == "[1, 2, 3]\n");
}

TEST_CASE_METHOD(JITFixture, "Test JIT List Append Op") {
    std::string listStr =
            "[1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1]";
    const std::string output = runCapture(std::format("print({})", listStr));
    REQUIRE(output == listStr + "\n");
}

TEST_CASE_METHOD(JITFixture, "Test JIT List Equality") {
    std::string output = runCapture("print([1, 2, 3] == [1, 2, 3])");
    REQUIRE(output == "True\n");

    output = runCapture("print([1, 3, 2] == [1, 2, 3])");
    REQUIRE(output == "False\n");

    output = runCapture("print([1, 2, 3] == [3])");
    REQUIRE(output == "False\n");
}

TEST_CASE_METHOD(JITFixture, "Test JIT List Addition") {
    const std::string output = runCapture("print([1, 2] + [3])");
    REQUIRE(output == "[1, 2, 3]\n");
}

TEST_CASE_METHOD(JITFixture, "Test JIT List Index") {
    const std::string output = runCapture("a = [1, 2]\nprint(a[0])");
    REQUIRE(output == "1\n");
}

TEST_CASE_METHOD(JITFixture, "Test JIT List Set Index") {
    const std::string output = runCapture("a = [1, 2]\na[1] = 3\nprint(a)");
    REQUIRE(output == "[1, 3]\n");
}


TEST_CASE_METHOD(JITFixture, "Test JIT List Length") {
    const std::string output = runCapture("print(len([1, 2]))");
    REQUIRE(output == "2\n");
}

TEST_CASE_METHOD(JITFixture, "Test JIT List Membership") {
    const std::string output = runCapture("a = [1, 2]\nprint(1 in a)");
    REQUIRE(output == "True\n");
}

TEST_CASE_METHOD(JITFixture, "Test JIT List Append") {
    const std::string output = runCapture("a = [1, 2]\na.append(3)\nprint(a)");
    REQUIRE(output == "[1, 2, 3]\n");
}

TEST_CASE_METHOD(JITFixture, "Test JIT List Extend") {
    const std::string output = runCapture("a = [1, 2]\na.extend([3])\nprint(a)");
    REQUIRE(output == "[1, 2, 3]\n");
}

TEST_CASE_METHOD(JITFixture, "Test JIT String as List") {
    const std::string output = runCapture("print(list('Hello'))");
    REQUIRE(output == "['H', 'e', 'l', 'l', 'o']\n");
}

TEST_CASE_METHOD(JITFixture, "Test JIT Set as List") {
    const std::string output =
            runCapture("a = list({1, 2, 3})\nprint(len(a) == 3 and (1 in a) and (2 in a) and (3 in a))");
    REQUIRE(output == "True\n");
}

TEST_CASE_METHOD(JITFixture, "Test JIT Tuple as List") {
    const std::string output = runCapture("print(list((1, 2, 3)))");
    REQUIRE(output == "[1, 2, 3]\n");
}

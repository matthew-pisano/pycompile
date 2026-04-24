//
// Created by matthew on 4/6/26.
//

#include <catch2/catch_all.hpp>

#include "execution_test_utils.h"


TEST_CASE_METHOD(JITFixture, "Test JIT Build Dict") {
    std::string output = runCapture("print({})");
    REQUIRE(output == "{}\n");

    output = runCapture("print(dict())");
    REQUIRE(output == "{}\n");

    output = runCapture("print({1: 'one', 2: 'two', 3: 'three'})");
    REQUIRE(output == "{1: 'one', 2: 'two', 3: 'three'}\n");
}

TEST_CASE_METHOD(JITFixture, "Test JIT Dict Equality") {
    std::string output = runCapture("print({1: 'one', 2: 'two', 3: 'three'} == {1: 'one', 2: 'two', 3: 'three'})");
    REQUIRE(output == "True\n");

    output = runCapture("print({1: 'one', 2: 'two', 3: 'three'} == {1: 'one', 3: 'three', 2: 'two'})");
    REQUIRE(output == "True\n");

    output = runCapture("print({1: 'one', 2: 'two', 3: 'three'} == {1: 'one', 2: 'two'})");
    REQUIRE(output == "False\n");
}

TEST_CASE_METHOD(JITFixture, "Test JIT Dict Merge") {
    const std::string output = runCapture("print({2: 'two', 3: 'three'} | {1: 'one'})");
    REQUIRE(output == "{1: 'one', 2: 'two', 3: 'three'}\n");
}

TEST_CASE_METHOD(JITFixture, "Test JIT Dict Membership") {
    const std::string output = runCapture("a = {1: 'one', 2: 'two', 3: 'three'}\nprint(1 in a)");
    REQUIRE(output == "True\n");
}

TEST_CASE_METHOD(JITFixture, "Test JIT Dict Length") {
    const std::string output = runCapture("print(len({1: 'one', 2: 'two', 3: 'three'}))");
    REQUIRE(output == "3\n");
}

TEST_CASE_METHOD(JITFixture, "Test JIT Dict Index") {
    const std::string output = runCapture("print({1: 'one', 2: 'two', 3: 'three'}[2])");
    REQUIRE(output == "two\n");
}

TEST_CASE_METHOD(JITFixture, "Test JIT Dict Set Absent Index") {
    std::string output = runCapture("a = {2: 'two', 3: 'three'}\na[1] = 'one'\nprint(a)");
    REQUIRE(output == "{1: 'one', 2: 'two', 3: 'three'}\n");

    output = runCapture("a = {2: 'two', 3: 'three'}\na['one'] = 1\nprint(a)");
    REQUIRE(output == "{2: 'two', 3: 'three', 'one': 1}\n");
}

TEST_CASE_METHOD(JITFixture, "Test JIT Dict Set Existing Index") {
    const std::string output = runCapture("a = {1: 'one', 2: 'two', 3: 'three'}\na[1] = 'oneagain'\nprint(a)");
    REQUIRE(output == "{1: 'oneagain', 2: 'two', 3: 'three'}\n");
}

TEST_CASE_METHOD(JITFixture, "Test JIT Dict Get") {
    std::string output = runCapture("print({1: 'one', 2: 'two', 3: 'three'}.get(2))");
    REQUIRE(output == "two\n");

    output = runCapture("print({1: 'one', 2: 'two', 3: 'three'}.get('z'))");
    REQUIRE(output == "None\n");
}

TEST_CASE_METHOD(JITFixture, "Test JIT Dict Keys") {
    const std::string output = runCapture("print({1: 'one', 2: 'two', 3: 'three'}.keys())");
    REQUIRE(output == "(1, 2, 3)\n");
}

TEST_CASE_METHOD(JITFixture, "Test JIT Dict Values") {
    const std::string output = runCapture("print({1: 'one', 2: 'two', 3: 'three'}.values())");
    REQUIRE(output == "('one', 'two', 'three')\n");
}

TEST_CASE_METHOD(JITFixture, "Test JIT Dict Items") {
    const std::string output = runCapture("print({1: 'one', 2: 'two', 3: 'three'}.items())");
    REQUIRE(output == "[(1, 'one'), (2, 'two'), (3, 'three')]\n");
}

TEST_CASE_METHOD(JITFixture, "Test JIT Dict Update") {
    const std::string output = runCapture("a = {2: 'two', 3: 'three'}\na.update({1: 'one'})\nprint(a)");
    REQUIRE(output == "{1: 'one', 2: 'two', 3: 'three'}\n");
}

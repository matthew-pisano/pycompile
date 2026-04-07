//
// Created by matthew on 4/6/26.
//

#include <catch2/catch_all.hpp>

#include "execution_test_utils.h"

TEST_CASE_METHOD(JITFixture, "Test JIT F-String") {
    const std::string output = runCapture("x = 42\nprint(f'Value is {x}')");
    REQUIRE(output == "Value is 42\n");
}

TEST_CASE_METHOD(JITFixture, "Test JIT String Index") {
    const std::string output = runCapture("a = 'Hello there'\nprint(a[2])");
    REQUIRE(output == "l\n");
}

TEST_CASE_METHOD(JITFixture, "Test JIT Set as String") {
    const std::string output =
            runCapture("a = str({1, 2, 3})\nprint(len(a) == 9 and ('1' in a) and ('2' in a) and ('3' in a))");
    REQUIRE(output == "True\n");
}

TEST_CASE_METHOD(JITFixture, "Test JIT List as String") {
    const std::string output = runCapture("print(str([1, 2, 3]))");
    REQUIRE(output == "[1, 2, 3]\n");
}

TEST_CASE_METHOD(JITFixture, "Test JIT Tuple as String") {
    const std::string output = runCapture("print(str((1, 2, 3)))");
    REQUIRE(output == "(1, 2, 3)\n");
}

TEST_CASE_METHOD(JITFixture, "Test JIT Dict as String") {
    const std::string output = runCapture("print(str({1: 'one', 2: 'two', 3: 'three'}))");
    REQUIRE(output == "{1: 'one', 2: 'two', 3: 'three'}\n");
}

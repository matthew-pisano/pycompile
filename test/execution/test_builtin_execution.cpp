//
// Created by matthew on 3/22/26.
//

#include <catch2/catch_all.hpp>

#include "execution_test_utils.h"


TEST_CASE_METHOD(JITFixture, "Test JIT Range") {
    std::string output = runCapture("print(range(5))");
    REQUIRE(output == "(0, 1, 2, 3, 4)\n");

    output = runCapture("print(range(3, 5))");
    REQUIRE(output == "(3, 4)\n");
}

TEST_CASE_METHOD(JITFixture, "Test JIT Enumerate") {
    const std::string output = runCapture("print(enumerate(['zero', 'one', 'two']))");
    REQUIRE(output == "[(0, 'zero'), (1, 'one'), (2, 'two')]\n");
}

TEST_CASE_METHOD(JITFixture, "Test JIT Zip") {
    const std::string output = runCapture("print(zip([1, 2, 3], ['a', 'b', 'c']))");
    REQUIRE(output == "[(1, 'a'), (2, 'b'), (3, 'c')]\n");
}

TEST_CASE_METHOD(JITFixture, "Test JIT Type") {
    SECTION("Test Type Str") {
        const std::string output = runCapture("print(type(''))");
        REQUIRE(output == "<class 'str'>\n");
    }

    SECTION("Test Type List") {
        const std::string output = runCapture("print(type([]))");
        REQUIRE(output == "<class 'list'>\n");
    }

    SECTION("Test Type Set") {
        const std::string output = runCapture("print(type(set()))");
        REQUIRE(output == "<class 'set'>\n");
    }

    SECTION("Test Type Dict") {
        const std::string output = runCapture("print(type({}))");
        REQUIRE(output == "<class 'dict'>\n");
    }

    SECTION("Test Type Tuple") {
        const std::string output = runCapture("print(type(()))");
        REQUIRE(output == "<class 'tuple'>\n");
    }

    SECTION("Test Type Iter") {
        const std::string output = runCapture("print(type(iter([])))");
        REQUIRE(output == "<class 'list_iterator'>\n");
    }
}

TEST_CASE_METHOD(JITFixture, "Test JIT Is Instance") {
    std::string output = runCapture("isinstance([], list)");
    REQUIRE(output == "True\n");

    output = runCapture("isinstance({}, list)");
    REQUIRE(output == "False\n");
}

TEST_CASE_METHOD(JITFixture, "Test JIT Name") {
    const std::string output = runCapture("print(__name__)");
    REQUIRE(output == "__main__\n");
}

TEST_CASE_METHOD(JITFixture, "Test JIT File") {
    const std::string output = runCapture("print(__file__)");
    REQUIRE(output == "<embedded>\n");
}

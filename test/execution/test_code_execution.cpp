//
// Created by matthew on 3/22/26.
//

#include <catch2/catch_all.hpp>

#include "execution_test_utils.h"


TEST_CASE_METHOD(JITFixture, "Test JIT Hello World") {
    const std::string output = runCapture("print('Hello world!')");
    REQUIRE(output == "Hello world!\n");
}

TEST_CASE_METHOD(JITFixture, "Test JIT Function Call") {
    const std::string output = runCapture("def foo():\n  print('bar')\nfoo()");
    REQUIRE(output == "bar\n");
}

TEST_CASE_METHOD(JITFixture, "Test JIT Function With Arg") {
    const std::string output = runCapture("def foo(arg):\n  print(arg)\nfoo('bar')");
    REQUIRE(output == "bar\n");
}

TEST_CASE_METHOD(JITFixture, "Test JIT While") {
    const std::string output = runCapture("i = 0\nwhile i < 3:\n  print(i)\n  i = i + 1");
    REQUIRE(output == "0\n1\n2\n");
}

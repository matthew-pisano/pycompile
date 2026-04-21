//
// Created by matthew on 3/22/26.
//

#include <catch2/catch_all.hpp>

#include "execution_test_utils.h"


TEST_CASE_METHOD(JITFixture, "Test JIT Hello World") {
    const std::string output = runCapture("print('Hello world!')");
    REQUIRE(output == "Hello world!\n");
}

TEST_CASE_METHOD(JITFixture, "Test JIT Fibonacci") {
    const std::string code = R"(
def fibonacci(n):
    """Returns the nth Fibonacci number."""
    if n <= 0:
        return 0
    elif n == 1:
        return 1
    else:
        return fibonacci(n - 1) + fibonacci(n - 2)


fib = 10
print(f"The {fib}th Fibonacci number is: {fibonacci(fib)}")
)";
    const std::string output = runCapture(code);
    REQUIRE(output == "The 10th Fibonacci number is: 55\n");
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
    SECTION("Test Single While") {
        const std::string output = runCapture("i = 0\nwhile i < 3:\n  print(i)\n  i += 1");
        REQUIRE(output == "0\n1\n2\n");
    }

    SECTION("Test Nested While") {
        const std::string output = runCapture(
                "i = 0\nj = 0\nwhile i < 3:\n  j = 0\n  while j < 3:\n    print(i, j)\n    j += 1\n  i += 1");
        REQUIRE(output == "0 0\n0 1\n0 2\n1 0\n1 1\n1 2\n2 0\n2 1\n2 2\n");
    }
}

TEST_CASE_METHOD(JITFixture, "Test JIT For") {
    SECTION("Test For Range") {
        const std::string output = runCapture("for i in range(3):\n  print(i)");
        REQUIRE(output == "0\n1\n2\n");
    }

    SECTION("Test For List") {
        const std::string output = runCapture("for i in [0, 1, 2]:\n  print(i)");
        REQUIRE(output == "0\n1\n2\n");
    }

    SECTION("Test For Dict") {
        const std::string output = runCapture("for i in {0: 'zero', 1: 'one', 2: 'two'}:\n  print(i)");
        REQUIRE(output == "0\n1\n2\n");
    }

    SECTION("Test For Tuple") {
        const std::string output = runCapture("for i in (0, 1, 2):\n  print(i)");
        REQUIRE(output == "0\n1\n2\n");
    }

    SECTION("Test For Set") {
        const std::string output = runCapture("for i in {0, 1, 2}:\n  print(i)");
        REQUIRE(output.length() == 6);
        REQUIRE(output.contains("0\n"));
        REQUIRE(output.contains("1\n"));
        REQUIRE(output.contains("2\n"));
    }

    SECTION("Test For String") {
        const std::string output = runCapture("for i in '012':\n  print(i)");
        REQUIRE(output == "0\n1\n2\n");
    }

    SECTION("Test Nested For Range") {
        const std::string output = runCapture("for i in range(3):\n  for j in range(3):\n    print(i, j)");
        REQUIRE(output == "0 0\n0 1\n0 2\n1 0\n1 1\n1 2\n2 0\n2 1\n2 2\n");
    }
}

# pycompile - A Python Source Code Compiler

*pycompile* takes in unmodified Python files and compiles them directly into a system binary instead of needing to execute through an interpreter. This results in faster execution time and compile-time optimizations.

To avoid needing to re-write and maintain a Python interpreter from scratch, the compiler front-end relies directly on CPython to translate Python source code into Python bytecode. On the back-end, this bytecode is then lowered using a custom PrIR MLIR dialect to LLVM IR. From there, it is compiled and linked to a custom builtin standard library to form a full executable. This builtin library implements the Python builtins in C++ that are themselves implemented in C (in CPython's
implementation). For easier portability, the builtin library is statically linked to each output program. This increases binary size slightly, but removes the need of managing a dynamic library.

## Installation

To download *pycompile*, simply navigate to the [latest release](https://github.com/matthew-pisano/pycompile/releases/latest) and download the executable. Currently, this is only available for x86 Linux machines.  *pycompile* links to LLVM 21.1 and Python 3.14, so the program may fail to launch if the system is unable to find the relevant shared libraries.

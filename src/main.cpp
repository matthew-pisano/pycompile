#include <Python.h>
#include <iostream>

#include "bytecode.h"
#include "utils.h"


int main() {
    Py_Initialize();

    const char* source =
            "def outer():\n"
            "    x = 10\n"
            "    f = lambda n: n * x\n" // closure — exercises freevars/cellvars
            "    try:\n"
            "        squares = [i**2 for i in range(x)]\n" // comprehension
            "    except ValueError as e:\n"
            "        pass\n"
            "    return f, squares\n";

    PyObject* code = Py_CompileString(source, "<embedded>", Py_file_input);
    if (!code) {
        const std::string err = getPythonErrorTraceback();
        Py_Finalize();
        std::cerr << "Error during parsing: " + err << std::endl;
        return 1;
    }

    try {
        const DisassembledCode d = disassemble(code);
        printDisassembly(d);
    } catch (const std::runtime_error& e) {
        Py_DECREF(code);
        Py_Finalize();
        std::cerr << "Error during disassembly: " + std::string(e.what()) << std::endl;
        return 1;
    }

    Py_DECREF(code);
    Py_Finalize();
    return 0;
}

//
// Created by matthew on 3/24/26.
//

#include "pyruntime/builtin_runtime.h"

#include <cmath>
#include <stdexcept>
#include <variant>

#include "pyruntime/objects/py_bool.h"
#include "pyruntime/objects/py_float.h"
#include "pyruntime/objects/py_int.h"
#include "pyruntime/objects/py_list.h"
#include "pyruntime/objects/py_none.h"
#include "pyruntime/objects/py_str.h"
#include "pyruntime/runtime_util.h"


PyObj* pyir_builtinPrint(PyObj** args, const int64_t argc) {
    for (int64_t i = 0; i < argc; i++) {
        if (i > 0)
            printf(" ");
        printf("%s", valueToString(args[i]).c_str());
    }
    printf("\n");
    return new PyNone();
}


PyObj* pyir_builtinLen(PyObj** args, const int64_t argc) {
    if (argc != 1)
        throw std::runtime_error("Too many arguments for len()");
    if (const PyStr* str = dynamic_cast<PyStr*>(args[0]))
        return str->len();
    if (const PyStr* list = dynamic_cast<PyStr*>(args[0]))
        return list->len();
    throw std::runtime_error("Object has no len()");
}


PyObj* pyir_builtinInt(PyObj** args, const int64_t argc) {
    if (argc != 1)
        throw std::runtime_error("Too many arguments for int()");

    if (PyInt* i = dynamic_cast<PyInt*>(args[0]))
        return i;
    if (const PyFloat* f = dynamic_cast<PyFloat*>(args[0]))
        return new PyInt(static_cast<int64_t>(f->data()));
    if (const PyBool* b = dynamic_cast<PyBool*>(args[0]))
        return new PyInt(b->data());
    if (const PyStr* s = dynamic_cast<PyStr*>(args[0]))
        return new PyInt(std::stoll(s->data()));
    throw std::runtime_error("Cannot convert to int()");
}


PyObj* pyir_builtinFloat(PyObj** args, const int64_t argc) {
    if (argc != 1)
        throw std::runtime_error("Too many arguments for float()");

    if (const PyInt* i = dynamic_cast<PyInt*>(args[0]))
        return new PyFloat(static_cast<double_t>(i->data()));
    if (PyFloat* f = dynamic_cast<PyFloat*>(args[0]))
        return f;
    if (const PyBool* b = dynamic_cast<PyBool*>(args[0]))
        return new PyFloat(b->data());
    if (const PyStr* s = dynamic_cast<PyStr*>(args[0]))
        return new PyFloat(std::stod(s->data()));
    throw std::runtime_error("cannot convert to float()");
}


PyObj* pyir_builtinStr(PyObj** args, const int64_t argc) {
    if (argc != 1)
        throw std::runtime_error("Too many arguments for str()");
    return new PyStr(valueToString(args[0]));
}


PyObj* pyir_builtinBool(PyObj** args, const int64_t argc) {
    if (argc != 1)
        throw std::runtime_error("Too many arguments for bool()");
    return new PyBool(args[0]->isTruthy());
}


PyObj* pyir_builtinList(PyObj** args, const int64_t argc) {
    if (argc > 1)
        throw std::runtime_error("Too many arguments for list()");
    if (argc == 0)
        return new PyList({});

    return new PyList(valueToList(args[0]));
}

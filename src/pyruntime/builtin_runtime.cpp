//
// Created by matthew on 3/24/26.
//

#include "pyruntime/builtin_runtime.h"

#include <cmath>
#include <ranges>
#include <stdexcept>

#include "pyruntime/objects/py_bool.h"
#include "pyruntime/objects/py_dict.h"
#include "pyruntime/objects/py_float.h"
#include "pyruntime/objects/py_int.h"
#include "pyruntime/objects/py_iter.h"
#include "pyruntime/objects/py_list.h"
#include "pyruntime/objects/py_none.h"
#include "pyruntime/objects/py_set.h"
#include "pyruntime/objects/py_str.h"
#include "pyruntime/objects/py_tuple.h"
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
        throw std::runtime_error("len() takes exactly one argument");
    return args[0]->len();
}


PyObj* pyir_builtinInt(PyObj** args, const int64_t argc) {
    if (argc != 1)
        throw std::runtime_error("int() takes exactly one argument");

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
        throw std::runtime_error("float() takes exactly one argument");

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
        throw std::runtime_error("str() takes exactly one argument");
    return new PyStr(valueToString(args[0]));
}


PyObj* pyir_builtinBool(PyObj** args, const int64_t argc) {
    if (argc != 1)
        throw std::runtime_error("bool() takes exactly one argument");
    return new PyBool(args[0]->isTruthy());
}


PyObj* pyir_builtinList(PyObj** args, const int64_t argc) {
    if (argc > 1)
        throw std::runtime_error("list() takes exactly one argument");
    if (argc == 0)
        return new PyList({});

    return new PyList(valueToList(args[0]));
}


PyObj* pyir_builtinSet(PyObj** args, const int64_t argc) {
    if (argc > 1)
        throw std::runtime_error("set() takes exactly one argument");
    if (argc == 0)
        return new PySet({});

    return new PySet(valueToSet(args[0]));
}


PyObj* pyir_builtinTuple(PyObj** args, const int64_t argc) {
    if (argc > 1)
        throw std::runtime_error("tuple() takes exactly one argument");
    if (argc == 0)
        return new PyTuple({});

    return new PyTuple(valueToList(args[0]));
}


PyObj* pyir_builtinDict(PyObj**, const int64_t argc) {
    if (argc > 0)
        throw std::runtime_error("set() takes no arguments");
    return new PyDict({});
}


PyObj* pyir_builtinIter(PyObj** args, const int64_t argc) {
    if (argc != 1)
        throw std::runtime_error("iter() takes exactly one argument");

    if (const PyStr* str = dynamic_cast<PyStr*>(args[0]))
        return new PyStrIter(str->data());
    if (const PyList* list = dynamic_cast<PyList*>(args[0]))
        return new PyListIter(list->data());
    if (const PySet* set = dynamic_cast<PySet*>(args[0]))
        return new PySetIter(set->data());
    if (const PyTuple* tuple = dynamic_cast<PyTuple*>(args[0]))
        return new PyTupleIter(tuple->data());
    if (const PyDict* dict = dynamic_cast<PyDict*>(args[0]))
        return new PyDictIter(dict->data());

    throw std::runtime_error("cannot convert to iter()");
}


PyObj* pyir_builtinNext(PyObj** args, const int64_t argc) {
    if (argc != 1)
        throw std::runtime_error("next() takes exactly one argument");

    if (PyObj* iter = dynamic_cast<PyStrIter*>(args[0]))
        return PyStrIter::next(iter, nullptr, 0);
    if (PyObj* iter = dynamic_cast<PyListIter*>(args[0]))
        return PyListIter::next(iter, nullptr, 0);
    if (PyObj* iter = dynamic_cast<PySetIter*>(args[0]))
        return PySetIter::next(iter, nullptr, 0);
    if (PyObj* iter = dynamic_cast<PyTupleIter*>(args[0]))
        return PyTupleIter::next(iter, nullptr, 0);
    if (PyObj* iter = dynamic_cast<PyDictIter*>(args[0]))
        return PyDictIter::next(iter, nullptr, 0);

    throw std::runtime_error("cannot convert to iter()");
}

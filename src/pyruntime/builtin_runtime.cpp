//
// Created by matthew on 3/24/26.
//

#include "pyruntime/builtin_runtime.h"

#include <cmath>
#include <format>
#include <iostream>
#include <ranges>

#include "pyruntime/objects/py_bool.h"
#include "pyruntime/objects/py_dict.h"
#include "pyruntime/objects/py_float.h"
#include "pyruntime/objects/py_function.h"
#include "pyruntime/objects/py_int.h"
#include "pyruntime/objects/py_list.h"
#include "pyruntime/objects/py_none.h"
#include "pyruntime/objects/py_set.h"
#include "pyruntime/objects/py_str.h"
#include "pyruntime/objects/py_tuple.h"
#include "pyruntime/runtime_errors.h"
#include "pyruntime/runtime_state.h"
#include "pyruntime/runtime_util.h"


static std::string moduleFile;
static std::string moduleName;


PyObj* pyir_builtinVarName() { return new PyStr(moduleName); }


PyObj* pyir_builtinVarFile() { return new PyStr(moduleFile); }


void pyir_initModule(const char* file, const char* name) {
    moduleFile = file;
    moduleName = name;

    builtinFuncs = {{"print", new PyFunction("print", pyir_builtinPrint)},
                    {"len", new PyFunction("len", pyir_builtinLen)},
                    {"int", new PyFunction("int", pyir_builtinInt)},
                    {"float", new PyFunction("float", pyir_builtinFloat)},
                    {"str", new PyFunction("str", pyir_builtinStr)},
                    {"bool", new PyFunction("bool", pyir_builtinBool)},
                    {"list", new PyFunction("list", pyir_builtinList)},
                    {"set", new PyFunction("set", pyir_builtinSet)},
                    {"tuple", new PyFunction("tuple", pyir_builtinTuple)},
                    {"dict", new PyFunction("dict", pyir_builtinDict)},
                    {"iter", new PyFunction("iter", pyir_builtinIter)},
                    {"next", new PyFunction("next", pyir_builtinNext)},
                    {"enumerate", new PyFunction("enumerate", pyir_builtinEnumerate)},
                    {"isinstance", new PyFunction("isinstance", pyir_builtinIsInstance)},
                    {"range", new PyFunction("range", pyir_builtinRange)},
                    {"type", new PyFunction("type", pyir_builtinType)},
                    {"zip", new PyFunction("zip", pyir_builtinZip)},
                    {"input", new PyFunction("input", pyir_builtinInput)}};
}


void pyir_destroyModule() {
    std::cout << "Cleaning up module" << std::endl;
    if (!scopeStack.empty())
        throw std::runtime_error("Exiting with dangling scope");

    // Clear module scope
    for (PyObj*& obj : moduleScope | std::views::values) {
        if (!obj)
            continue; // Skip already nulled objects
        if (dynamic_cast<PyNone*>(obj) || dynamic_cast<PyBool*>(obj))
            continue; // Skip immortal objects
        if (obj->decref())
            obj = nullptr;
    }
    moduleScope.clear();

    // Clear builtin functions
    for (PyFunction*& func : builtinFuncs | std::views::values) {
        if (func->decref())
            func = nullptr;
    }
    builtinFuncs.clear();
}


PyObj* pyir_builtinPrint(PyObj** args, const int64_t argc) {
    for (int64_t i = 0; i < argc; i++) {
        if (i > 0)
            printf(" ");
        printf("%s", valueToString(args[i]).c_str());
        if (args[i]->decref())
            args[i] = nullptr;
    }
    printf("\n");
    return PyNone::None;
}


PyObj* pyir_builtinLen(PyObj** args, const int64_t argc) {
    if (argc != 1)
        throw PyTypeError("len() takes exactly one argument");
    PyObj* len = args[0]->len();
    if (args[0]->decref())
        args[0] = nullptr;
    return len;
}


PyObj* pyir_builtinInt(PyObj** args, const int64_t argc) {
    if (argc != 1)
        throw PyTypeError("int() takes exactly one argument");

    PyInt* result = nullptr;
    if (PyInt* i = dynamic_cast<PyInt*>(args[0]))
        result = i;
    else if (const PyFloat* f = dynamic_cast<PyFloat*>(args[0]))
        result = new PyInt(static_cast<int64_t>(f->data()));
    else if (const PyBool* b = dynamic_cast<PyBool*>(args[0]))
        result = new PyInt(b->data());
    else if (const PyStr* s = dynamic_cast<PyStr*>(args[0]))
        result = new PyInt(std::stoll(s->data()));

    const std::string typeName = args[0]->typeName();
    if (args[0]->decref())
        args[0] = nullptr;

    if (!result)
        throw PyTypeError(formatBadConversion(typeName, "int", typeName));
    return result;
}


PyObj* pyir_builtinFloat(PyObj** args, const int64_t argc) {
    if (argc != 1)
        throw PyTypeError("float() takes exactly one argument");

    PyObj* result = nullptr;
    if (const PyInt* i = dynamic_cast<PyInt*>(args[0]))
        result = new PyFloat(static_cast<double_t>(i->data()));
    else if (PyFloat* f = dynamic_cast<PyFloat*>(args[0]))
        result = f;
    else if (const PyBool* b = dynamic_cast<PyBool*>(args[0]))
        result = new PyFloat(b->data());
    else {
        try {
            if (const PyStr* s = dynamic_cast<PyStr*>(args[0]))
                result = new PyFloat(std::stod(s->data()));
        } catch (...) {
        }
    }

    const std::string typeName = args[0]->typeName();
    const std::string strVal = args[0]->toString();
    if (args[0]->decref())
        args[0] = nullptr;

    if (!result)
        throw PyTypeError(formatBadConversion(typeName, "float", strVal));
    return result;
}


PyObj* pyir_builtinStr(PyObj** args, const int64_t argc) {
    if (argc != 1)
        throw PyTypeError("str() takes exactly one argument");
    PyObj* result = new PyStr(valueToString(args[0]));
    if (args[0]->decref())
        args[0] = nullptr;
    return result;
}


PyObj* pyir_builtinBool(PyObj** args, const int64_t argc) {
    if (argc != 1)
        throw PyTypeError("bool() takes exactly one argument");
    PyObj* result = args[0]->isTruthy() ? PyBool::True : PyBool::False;
    if (args[0]->decref())
        args[0] = nullptr;
    return result;
}


PyObj* pyir_builtinList(PyObj** args, const int64_t argc) {
    if (argc > 1)
        throw PyTypeError("list() takes exactly one argument");
    if (argc == 0)
        return new PyList({});
    PyObj* result = new PyList(valueToList(args[0]));
    if (args[0]->decref())
        args[0] = nullptr;
    return result;
}


PyObj* pyir_builtinSet(PyObj** args, const int64_t argc) {
    if (argc > 1)
        throw PyTypeError("set() takes exactly one argument");
    if (argc == 0)
        return new PySet({});
    PyObj* result = new PySet(valueToSet(args[0]));
    if (args[0]->decref())
        args[0] = nullptr;
    return result;
}


PyObj* pyir_builtinTuple(PyObj** args, const int64_t argc) {
    if (argc > 1)
        throw PyTypeError("tuple() takes exactly one argument");
    if (argc == 0)
        return new PyTuple({});
    PyObj* result = new PyTuple(valueToList(args[0]));
    if (args[0]->decref())
        args[0] = nullptr;
    return result;
}


PyObj* pyir_builtinDict(PyObj**, const int64_t argc) {
    if (argc > 0)
        throw PyTypeError("dict() takes no arguments");
    return new PyDict({});
}


PyObj* pyir_builtinIter(PyObj** args, const int64_t argc) {
    if (argc != 1)
        throw PyTypeError("iter() takes exactly one argument");

    PyObj* result = nullptr;
    if (PyStr* str = dynamic_cast<PyStr*>(args[0]))
        result = new PyStrIter(str);
    else if (PyList* list = dynamic_cast<PyList*>(args[0]))
        result = new PyListIter(list);
    else if (PySet* set = dynamic_cast<PySet*>(args[0]))
        result = new PySetIter(set);
    else if (PyTuple* tuple = dynamic_cast<PyTuple*>(args[0]))
        result = new PyTupleIter(tuple);
    else if (PyDict* dict = dynamic_cast<PyDict*>(args[0]))
        result = new PyDictIter(dict);

    const std::string typeName = args[0]->typeName();
    const std::string strVal = args[0]->toString();
    if (args[0]->decref())
        args[0] = nullptr;

    if (!result)
        throw PyTypeError(formatBadConversion(typeName, "iter", strVal));
    return result;
}


PyObj* pyir_builtinNext(PyObj** args, const int64_t argc) {
    if (argc != 1)
        throw PyTypeError("next() takes exactly one argument");

    // Note: do not decref the iterator, it must remain alive across calls
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

    throw PyTypeError(formatBadConversion(args[0]->typeName(), "iter", args[0]->toString()));
}


PyObj* pyir_builtinEnumerate(PyObj** args, const int64_t argc) {
    if (argc == 0)
        throw PyTypeError("enumerate() takes one argument");
    std::vector<PyObj*> result;
    // Iterate over keys for dict
    if (PyDict* dict = dynamic_cast<PyDict*>(args[0])) {
        size_t i = 0;
        for (const auto& key : dict->data() | std::views::keys) {
            key->incref();
            result.push_back(new PyTuple({new PyInt(static_cast<int64_t>(i)), key}));
            i++;
        }
    } else {
        PyInt* length = args[0]->len();
        for (int64_t i = 0; i < length->data(); i++) {
            PyInt* idx = new PyInt(i);
            PyObj* elem = args[0]->idx(idx);
            elem->incref();
            result.push_back(new PyTuple({idx, elem}));
        }
        (void) length->decref();
    }
    PyObj* ret = new PyList(result);
    if (args[0]->decref())
        args[0] = nullptr;
    return ret;
}


PyObj* pyir_builtinIsInstance(PyObj** args, const int64_t argc) {
    if (argc < 2)
        throw PyTypeError("isinstance() takes two arguments");
    const std::string instanceType = args[0]->typeName();
    PyObj* result = nullptr;
    if (const PyFunction* type = dynamic_cast<PyFunction*>(args[1]))
        result = instanceType == type->funcName() ? PyBool::True : PyBool::False;
    if (args[0]->decref())
        args[0] = nullptr;
    // Do not decref the type builtin
    if (!result)
        throw PyTypeError("isinstance() takes a type as the second argument");
    return result;
}


PyObj* pyir_builtinRange(PyObj** args, const int64_t argc) {
    if (argc == 0)
        throw PyTypeError("range() takes at least one argument");
    int64_t startIdx = 0;
    int64_t endIdx = 0;
    if (argc >= 1) {
        if (const PyInt* endInteger = dynamic_cast<PyInt*>(args[argc == 1 ? 0 : 1]))
            endIdx = endInteger->data();
        else
            throw PyTypeError("range() expects integer arguments");
    }
    if (argc == 2) {
        if (const PyInt* startInteger = dynamic_cast<PyInt*>(args[0]))
            startIdx = startInteger->data();
        else
            throw PyTypeError("range() expects integer arguments");
    } else if (argc > 2)
        throw PyTypeError("range() takes at most two arguments");

    std::vector<PyObj*> seq;
    for (int64_t i = startIdx; i < endIdx; i++)
        seq.push_back(new PyInt(i));

    for (int64_t i = 0; i < argc; i++)
        if (args[i]->decref())
            args[i] = nullptr;

    return new PyTuple(seq);
}


PyObj* pyir_builtinType(PyObj** args, const int64_t argc) {
    if (argc == 0)
        throw PyTypeError("type() takes at least one argument");
    PyObj* result = new PyStr(std::format("<class '{}'>", args[0]->typeName()));
    if (args[0]->decref())
        args[0] = nullptr;
    return result;
}


PyObj* getShortestContainer(PyObj** args, const int64_t argc) {
    if (argc < 1)
        throw PyTypeError("Expected at least one element to compare the lengths of");
    int64_t minLength = INT64_MAX;
    PyObj* shortest = nullptr;
    for (int64_t i = 0; i < argc; i++) {
        PyInt* length = args[i]->len();
        if (length->data() < minLength) {
            minLength = length->data();
            shortest = args[i];
        }
        (void) length->decref();
    }

    return shortest;
}


PyObj* pyir_builtinZip(PyObj** args, const int64_t argc) {
    if (argc < 1)
        throw PyTypeError("zip() takes at least one argument");
    const PyObj* shortest = getShortestContainer(args, argc);
    PyInt* shortestLen = shortest->len();
    std::vector<std::vector<PyObj*>> zipped(shortestLen->data());
    for (int64_t elemIdx = 0; elemIdx < shortestLen->data(); elemIdx++)
        zipped[elemIdx] = std::vector<PyObj*>(argc);

    for (int64_t containerIdx = 0; containerIdx < argc; containerIdx++) {
        // Iterate over keys for dict
        if (PyDict* dict = dynamic_cast<PyDict*>(args[containerIdx])) {
            int64_t elemIdx = 0;
            for (const auto& key : dict->data() | std::views::keys) {
                if (elemIdx >= shortestLen->data())
                    break;

                key->incref();
                zipped[elemIdx][containerIdx] = key;
                elemIdx++;
            }
        } else {
            for (int64_t elemIdx = 0; elemIdx < shortestLen->data(); elemIdx++) {
                const PyInt* idx = new PyInt(elemIdx);
                PyObj* elem = args[containerIdx]->idx(idx);
                elem->incref();
                zipped[elemIdx][containerIdx] = elem;
            }
        }
    }

    std::vector<PyObj*> result;
    result.reserve(shortestLen->data());
    for (int64_t elemIdx = 0; elemIdx < shortestLen->data(); elemIdx++)
        result.push_back(new PyTuple(zipped[elemIdx]));

    (void) shortestLen->decref();

    for (int64_t i = 0; i < argc; i++)
        if (args[i]->decref())
            args[i] = nullptr;

    return new PyList(result);
}


PyObj* pyir_builtinInput(PyObj** args, const int64_t argc) {
    if (argc > 1)
        throw PyTypeError("input() expects at most one argument");
    if (argc > 0) {
        printf("%s", valueToString(args[0]).c_str());
        if (args[0]->decref())
            args[0] = nullptr;
    }
    char buf[4096];
    if (!fgets(buf, sizeof(buf), stdin))
        return new PyStr("");

    std::string line(buf);
    // Strip trailing newline
    if (!line.empty() && line.back() == '\n')
        line.pop_back();
    return new PyStr(line);
}

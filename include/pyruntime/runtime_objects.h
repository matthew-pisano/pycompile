//
// Created by matthew on 3/29/26.
//

#ifndef PYCOMPILE_RUNTIME_OBJECTS_H
#define PYCOMPILE_RUNTIME_OBJECTS_H
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

struct PyValue; // Forward declaration

using PyFunction = PyValue* (*) (PyValue**, int64_t);
using PySet = std::unordered_set<PyValue*>;


struct PyNone {
    bool operator==(const PyNone&) const {
        return true; // None is always None
    }
};


struct PyBoundMethod {
    using SelfFunction = PyValue* (*) (PyValue*, PyValue**, int64_t);

    PyValue* self;
    SelfFunction fn;

    bool operator==(const PyBoundMethod&) const = default;
};


struct PyList {
    static const std::unordered_map<std::string, PyBoundMethod::SelfFunction> attrs;

    static PyValue* append(PyValue* self, PyValue** args, int64_t argc);

    static PyValue* extend(PyValue* self, PyValue** args, int64_t argc);

    static PyValue* getAttr(PyValue* self, const char* name);

    static PyValue* len(const PyValue* self);

    static PyValue* str(const PyValue* self);

    std::string toString() const;

    const std::vector<PyValue*>& data() const;
    std::vector<PyValue*>& data();

    bool operator==(const PyList& other) const;

private:
    std::vector<PyValue*> rawData;
};

#endif // PYCOMPILE_RUNTIME_OBJECTS_H

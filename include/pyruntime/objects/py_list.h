//
// Created by matthew on 3/29/26.
//

#ifndef PYCOMPILE_PY_LIST_H
#define PYCOMPILE_PY_LIST_H
#include <string>
#include <unordered_map>

#include "pyruntime/runtime_objects.h"

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

#endif // PYCOMPILE_PY_LIST_H

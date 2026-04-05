//
// Created by matthew on 3/29/26.
//

#ifndef PYCOMPILE_PY_METHOD_H
#define PYCOMPILE_PY_METHOD_H
#include <utility>


#include "py_object.h"

using PyMethodData = PyObj* (*) (PyObj*, PyObj**, int64_t);

struct PyMethod : PyObj {
    explicit PyMethod(std::string fnName, PyObj* self, const PyMethodData& func) :
        fnName(std::move(fnName)), self(self), fn(func) {}

    [[nodiscard]] size_t hash() const override;

    [[nodiscard]] std::string toString() const override;

    [[nodiscard]] std::string typeName() const override;

    [[nodiscard]] bool isTruthy() const override;

    [[nodiscard]] std::string funcName() const;

    [[nodiscard]] PyObj* selfObj() const;

    [[nodiscard]] PyMethodData data() const;

    bool operator==(const PyObj& other) const override;

private:
    std::string fnName;
    PyObj* self;
    PyMethodData fn;
};

#endif // PYCOMPILE_PY_METHOD_H

//
// Created by matthew on 3/29/26.
//

#ifndef PYCOMPILE_PY_METHOD_H
#define PYCOMPILE_PY_METHOD_H
#include <utility>


#include "py_object.h"

using PyMethodType = PyObj* (*) (PyObj*, PyObj**, int64_t);

struct PyMethod : PyObj {
    explicit PyMethod(std::string fnName, PyObj* self, const PyMethodType& func) :
        fnName(std::move(fnName)), self(self), fn(func) {}

    [[nodiscard]] std::string toString() const override;

    [[nodiscard]] std::string typeName() const override;

    [[nodiscard]] bool isTruthy() const override;

    [[nodiscard]] std::string funcName() const;

    [[nodiscard]] PyObj* selfObj() const;

    [[nodiscard]] PyMethodType data() const;

    bool operator==(const PyObj& other) const override;

private:
    std::string fnName;
    PyObj* self;
    PyMethodType fn;
};

#endif // PYCOMPILE_PY_METHOD_H

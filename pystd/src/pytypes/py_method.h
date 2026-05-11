//
// Created by matthew on 3/29/26.
//

#ifndef PYCOMPILE_PY_METHOD_H
#define PYCOMPILE_PY_METHOD_H
#include <utility>


#include "py_object.h"

using PyMethodData = PyObj* (*) (PyObj*, PyObj**, int64_t);

/**
 * PyMethod represents a method bound to a specific object. It is created when an attribute lookup on an object returns
 * a method, and it allows the method to be called with the correct self argument.
 */
struct PyMethod : PyObj {
    explicit PyMethod(std::string fnName, PyObj* self, const PyMethodData& func) :
        fnName(std::move(fnName)), self(self), fn(func) {}
    ~PyMethod() override;

    [[nodiscard]] size_t hash() const override;

    [[nodiscard]] std::string toString() const override;

    [[nodiscard]] std::string typeName() const override;

    [[nodiscard]] bool isTruthy() const override;

    /// Returns the name of this PyMethod.
    [[nodiscard]] std::string funcName() const;

    /// Returns the self object that this PyMethod is bound to.
    [[nodiscard]] PyObj* selfObj() const;

    /// Returns the raw function pointer of this PyMethod.
    [[nodiscard]] PyMethodData data() const;

    std::partial_ordering operator<=>(const PyObj& other) const noexcept override;

    bool operator==(const PyObj&) const noexcept override;

private:
    std::string fnName;
    PyObj* self;
    PyMethodData fn;
};

#endif // PYCOMPILE_PY_METHOD_H

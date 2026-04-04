//
// Created by matthew on 3/29/26.
//

#ifndef PYCOMPILE_PY_LIST_H
#define PYCOMPILE_PY_LIST_H
#include <vector>

#include "py_method.h"
#include "py_object.h"


struct PyNone;

struct PyList : PyObj {
    explicit PyList(const std::vector<PyObj*>& list) : raw(list) {}

    static PyObj* append(PyObj* self, PyObj** args, int64_t argc);

    static PyObj* extend(PyObj* self, PyObj** args, int64_t argc);

    PyObj* getAttr(const std::string& name) override;

    [[nodiscard]] PyInt* len() const override;

    [[nodiscard]] std::string toString() const override;

    [[nodiscard]] std::string typeName() const override;

    [[nodiscard]] bool isTruthy() const override;

    [[nodiscard]] std::vector<PyObj*> data() const;

    std::vector<PyObj*>& data();

    bool operator==(const PyObj& other) const override;

private:
    std::vector<PyObj*> raw;
};

#endif // PYCOMPILE_PY_LIST_H

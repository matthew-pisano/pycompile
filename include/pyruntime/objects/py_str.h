//
// Created by matthew on 3/29/26.
//

#ifndef PYCOMPILE_PY_STR_H
#define PYCOMPILE_PY_STR_H
#include <utility>

#include "py_method.h"
#include "py_object.h"

struct PyStr : PyObj {
    explicit PyStr(std::string str) : raw(std::move(str)) {}
    explicit PyStr(const char c) : raw(1, c) {}

    [[nodiscard]] PyInt* len() const override;

    [[nodiscard]] PyBool* contains(const PyObj* obj) const override;

    PyObj* idx(const PyObj* idx) const override;

    [[nodiscard]] size_t hash() const override;

    [[nodiscard]] std::string toString() const override;

    [[nodiscard]] std::string typeName() const override;

    [[nodiscard]] bool isTruthy() const override;

    [[nodiscard]] std::string data() const;

    bool operator==(const PyObj& other) const override;

private:
    std::string raw;
};

#endif // PYCOMPILE_PY_STR_H

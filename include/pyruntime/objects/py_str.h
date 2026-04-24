//
// Created by matthew on 3/29/26.
//

#ifndef PYCOMPILE_PY_STR_H
#define PYCOMPILE_PY_STR_H
#include <utility>

#include "py_iter.h"
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

    std::partial_ordering operator<=>(const PyObj& other) const noexcept override;

    bool operator==(const PyObj&) const noexcept override;

private:
    std::string raw;

    friend struct PyStrIter;
};


struct PyStrIter : PyIter {
    explicit PyStrIter(PyStr* str) : str(str) {
        this->str->incref();
        it = this->str->raw.begin();
        end = this->str->raw.end();
    }
    ~PyStrIter() override;

    static PyObj* next(PyObj* self, PyObj**, int64_t argc);

    [[nodiscard]] std::string toString() const override { return "<str_iterator>"; }

    [[nodiscard]] std::string typeName() const override { return "str_iterator"; }

    std::partial_ordering operator<=>(const PyObj& other) const noexcept override;

private:
    std::string::iterator it;
    std::string::iterator end;
    PyStr* str;
};

#endif // PYCOMPILE_PY_STR_H

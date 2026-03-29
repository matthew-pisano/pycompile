//
// Created by matthew on 3/8/26.
//

#ifndef PYCOMPILE_RUNTIME_VALUE_H
#define PYCOMPILE_RUNTIME_VALUE_H
#include <atomic>
#include <string>
#include <unordered_set>
#include <variant>
#include <vector>

#include "runtime_objects.h"


struct PyValue {
    using Data = std::variant<PyNone, bool, int64_t, double, std::string, PyList, PySet, PyFunction, PyBoundMethod>;

    Data data;
    std::atomic<int32_t> refcount{1};

    // Disable copy/move: Values live on the heap and are managed by refcount
    PyValue(const PyValue&) = delete;

    // Disable constructor from pointer
    explicit PyValue(PyValue*) = delete;

    PyValue& operator=(const PyValue&) = delete;

    explicit PyValue(PyNone) : data(PyNone{}) {}

    explicit PyValue(bool b) : data(b) {}

    explicit PyValue(int64_t i) : data(i) {}

    explicit PyValue(double f) : data(f) {}

    explicit PyValue(std::string s) : data(std::move(s)) {}

    explicit PyValue(const char c) : data(std::string(1, c)) {}

    explicit PyValue(PyList list) : data(std::move(list)) {}

    explicit PyValue(PySet set) : data(std::move(set)) {}

    explicit PyValue(PyFunction f) : data(f) {}

    explicit PyValue(PyBoundMethod bm) : data(bm) {}

    void incref() { refcount.fetch_add(1, std::memory_order_relaxed); }

    void decref() {
        if (refcount.fetch_sub(1, std::memory_order_acq_rel) == 1)
            delete this;
    }

    [[nodiscard]] bool isNone() const { return std::holds_alternative<PyNone>(data); }
    [[nodiscard]] bool isBool() const { return std::holds_alternative<bool>(data); }
    [[nodiscard]] bool isInt() const { return std::holds_alternative<int64_t>(data); }
    [[nodiscard]] bool isFloat() const { return std::holds_alternative<double>(data); }
    [[nodiscard]] bool isStr() const { return std::holds_alternative<std::string>(data); }
    [[nodiscard]] bool isList() const { return std::holds_alternative<PyList>(data); }
    [[nodiscard]] bool isSet() const { return std::holds_alternative<PySet>(data); }
    [[nodiscard]] bool isFn() const { return std::holds_alternative<PyFunction>(data); }
    [[nodiscard]] bool isBoundMethod() const { return std::holds_alternative<PyBoundMethod>(data); }
};


struct ValueRef {
    PyValue* ptr;

    explicit ValueRef(PyValue* p) : ptr(p) {}

    // Takes ownership, no incref
    ~ValueRef() {
        if (ptr)
            ptr->decref();
    }

    [[nodiscard]] PyValue* get() const { return ptr; }

    PyValue* release() {
        PyValue* p = ptr;
        ptr = nullptr;
        return p;
    }
};


#endif // PYCOMPILE_RUNTIME_VALUE_H

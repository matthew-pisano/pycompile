//
// Created by matthew on 3/29/26.
//

#ifndef PYCOMPILE_PY_OBJECT_H
#define PYCOMPILE_PY_OBJECT_H

#include <atomic>
#include <string>
#include <unordered_map>

struct PyMethod;
struct PyStr;
struct PyInt;

struct PyObj {
    PyObj() = default;

    // Disable copy/move: Objects live on the heap and are managed by refcount
    PyObj(const PyObj&) = delete;

    // Disable constructor from pointer
    explicit PyObj(PyObj*) = delete;

    // Disable assignment
    PyObj& operator=(const PyObj&) = delete;

    virtual ~PyObj() = default;

    virtual PyObj* getAttr(const std::string& name);

    [[nodiscard]] virtual PyStr* name() const;

    [[nodiscard]] virtual PyInt* len() const;

    [[nodiscard]] virtual PyStr* str() const;

    [[nodiscard]] virtual std::string toString() const = 0;

    [[nodiscard]] virtual std::string typeName() const = 0;

    [[nodiscard]] virtual bool isTruthy() const = 0;

    void incref();

    void decref();

    virtual bool operator==(const PyObj& other) const = 0;

private:
    std::atomic<int32_t> refcount{1};
};


struct PyObjRef {
    PyObj* ptr;

    explicit PyObjRef(PyObj* p) : ptr(p) {}

    // Takes ownership, no incref
    ~PyObjRef() {
        if (ptr)
            ptr->decref();
    }

    [[nodiscard]] PyObj* get() const { return ptr; }

    PyObj* release() {
        PyObj* p = ptr;
        ptr = nullptr;
        return p;
    }
};

#endif // PYCOMPILE_PY_OBJECT_H

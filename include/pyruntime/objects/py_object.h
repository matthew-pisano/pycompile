//
// Created by matthew on 3/29/26.
//

#ifndef PYCOMPILE_PY_OBJECT_H
#define PYCOMPILE_PY_OBJECT_H

#include <atomic>
#include <string>
#include <unordered_map>


struct PyBool;
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

    [[nodiscard]] virtual PyBool* contains(const PyObj* obj) const;

    [[nodiscard]] virtual PyObj* idx(const PyObj* idx) const;

    [[nodiscard]] virtual size_t hash() const = 0;

    [[nodiscard]] virtual std::string toString() const = 0;

    [[nodiscard]] virtual std::string typeName() const = 0;

    [[nodiscard]] virtual bool isTruthy() const = 0;

    void incref();

    void decref();

    virtual std::partial_ordering operator<=>(const PyObj& other) const noexcept = 0;
    virtual bool operator==(const PyObj& other) const noexcept = 0;

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


struct PyObjPtrHash {
    std::size_t operator()(const PyObj* s) const {
        if (!s)
            return 0; // Handle null pointers
        return s->hash();
    }
};


struct PyObjPtrEqual {
    bool operator()(const PyObj* lhs, const PyObj* rhs) const {
        if (lhs == rhs)
            return true;
        if (!lhs || !rhs)
            return false;
        return *lhs == *rhs;
    }
};


struct PyObjPtrCompare {
    bool operator()(const PyObj* lhs, const PyObj* rhs) const {
        if (!lhs || !rhs)
            return false;

        const std::partial_ordering result = *lhs <=> *rhs;
        if (result == std::partial_ordering::unordered)
            return lhs->typeName() < rhs->typeName();
        return result == std::partial_ordering::less;
    }
};


#endif // PYCOMPILE_PY_OBJECT_H

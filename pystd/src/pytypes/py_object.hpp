//
// Created by matthew on 3/29/26.
//

#ifndef PYCOMPILE_PY_OBJECT_H
#define PYCOMPILE_PY_OBJECT_H

#include <atomic>
#include <string>


struct PyBool;
struct PyMethod;
struct PyStr;
struct PyInt;


/**
 * Base class for all Python objects. Implements reference counting and common operations.
 */
struct PyObj {
    PyObj() = default;

    // Disable copy/move: Objects live on the heap and are managed by refcount
    PyObj(const PyObj&) = delete;

    // Disable constructor from pointer
    explicit PyObj(PyObj*) = delete;

    // Disable assignment
    PyObj& operator=(const PyObj&) = delete;

    virtual ~PyObj() = default;

    /// Get an attribute by name. Throws a PyAttributeError if the attribute does not exist.
    virtual PyObj* getAttr(const std::string& name);

    /// Get the name of the type of this object as a PyStr.
    [[nodiscard]] virtual PyStr* name() const;

    /// Get the length of this object as a PyInt. Throws a PyTypeError if the object does not have a length.
    [[nodiscard]] virtual PyInt* len() const;

    /// Get the string representation of this object as a PyStr.
    [[nodiscard]] virtual PyStr* str() const;

    /// Check if this object contains another object. Throws a PyTypeError if the object is not iterable.
    [[nodiscard]] virtual PyBool* contains(const PyObj* obj) const;

    /// Get the item at the given index. Throws a PyTypeError if the object is not subscriptable, or a PyIndexError if
    /// the index is out of range.
    [[nodiscard]] virtual PyObj* idx(const PyObj* idx) const;

    /// Set the item at the given index to the given value. Throws a PyTypeError if the object does not support item
    /// assignment, or a PyIndexError if the index is out of range.
    virtual void setIdx(PyObj* idx, PyObj* value);

    /// Get the hash of this object. Throws a PyTypeError if the object is unhashable.
    [[nodiscard]] virtual size_t hash() const = 0;

    /// Get the C++ string representation of this object.
    [[nodiscard]] virtual std::string toString() const = 0;

    /// Get the name of the type of this object as a C++ string.
    [[nodiscard]] virtual std::string typeName() const = 0;

    /// Check if this object is truthy or falsy.
    [[nodiscard]] virtual bool isTruthy() const = 0;

    /// Increment the reference count of this object.
    virtual void incref();

    /// Decrement the reference count of this object. If the reference count reaches zero, the object is deleted and
    /// true is returned. Otherwise, false is returned.
    [[nodiscard]] virtual bool decref();

    virtual std::partial_ordering operator<=>(const PyObj& other) const noexcept = 0;
    virtual bool operator==(const PyObj& other) const noexcept = 0;

private:
    std::atomic<int32_t> refcount{1};
};


/**
 * A helper class for managing PyObj references. Automatically decrefs the object when it goes out of scope. Does not
 * support copying or moving, as it is meant to be used as a unique reference to a PyObj.
 */
struct PyObjRef {
    PyObj* ptr;

    explicit PyObjRef(PyObj* p) : ptr(p) {}

    // Takes ownership, no incref
    ~PyObjRef() {
        if (ptr)
            if (ptr->decref())
                ptr = nullptr;
    }

    /// Get the raw pointer to the PyObj.
    [[nodiscard]] PyObj* get() const { return ptr; }

    /// Release ownership of the PyObj and return the raw pointer. The caller is responsible for managing the reference
    /// count of the returned pointer.
    PyObj* release() {
        PyObj* p = ptr;
        ptr = nullptr;
        return p;
    }
};


/**
 * Hash function for PyObj pointers. Uses the hash of the pointed-to object, or 0 for null pointers.
 */
struct PyObjPtrHash {
    std::size_t operator()(const PyObj* s) const {
        if (!s)
            return 0; // Handle null pointers
        return s->hash();
    }
};


/**
 * Equality function for PyObj pointers. Compares the pointed-to objects for equality.
 */
struct PyObjPtrEqual {
    bool operator()(const PyObj* lhs, const PyObj* rhs) const {
        if (lhs == rhs)
            return true;
        if (!lhs || !rhs)
            return false;
        return *lhs == *rhs;
    }
};


/**
 * Comparison function for PyObj pointers. Compares the pointed-to objects using their operator<=>, and if they are
 * unordered, compares their type names to provide a consistent ordering.
 */
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

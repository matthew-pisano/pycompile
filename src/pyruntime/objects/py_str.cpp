//
// Created by matthew on 3/29/26.
//

#include "pyruntime/objects/py_str.h"

#include <stdexcept>

#include "pyruntime/objects/py_bool.h"
#include "pyruntime/objects/py_int.h"

size_t PyStr::hash() const {
    constexpr std::hash<std::string> hasher;
    return hasher(raw);
}

PyInt* PyStr::len() const { return new PyInt(static_cast<int64_t>(raw.size())); }

PyBool* PyStr::contains(const PyObj* obj) const {
    if (const PyStr* str = dynamic_cast<const PyStr*>(obj))
        return new PyBool(raw.contains(str->data()));
    throw std::runtime_error("str objects can only contain other str objects");
}

PyObj* PyStr::idx(const PyObj* idx) const {
    if (const PyInt* idxVal = dynamic_cast<const PyInt*>(idx)) {
        int64_t index = idxVal->data();
        if (index < 0)
            index += static_cast<int64_t>(raw.size());
        if (index < 0 || index >= static_cast<int64_t>(raw.size()))
            throw std::runtime_error("str index out of range");
        return new PyStr(raw[index]);
    }
    throw std::runtime_error("str indices must be integers");
}

std::string PyStr::toString() const { return raw; }

std::string PyStr::typeName() const { return "str"; }

bool PyStr::isTruthy() const { return !raw.empty(); }

std::string PyStr::data() const { return raw; }

std::partial_ordering PyStr::operator<=>(const PyObj& other) const noexcept {
    if (const PyStr* s = dynamic_cast<const PyStr*>(&other))
        return raw <=> s->data();
    return std::partial_ordering::unordered;
}

bool PyStr::operator==(const PyObj& other) const noexcept {
    return *this <=> other == std::partial_ordering::equivalent;
}

PyObj* PyStrIter::next(PyObj* self, PyObj**, const int64_t argc) {
    if (argc != 0)
        throw std::runtime_error("next() takes no arguments");
    PyStrIter* selfIter = dynamic_cast<PyStrIter*>(self);
    if (!selfIter)
        throw std::runtime_error("Can only get the next value of iterator types");
    if (selfIter->it == selfIter->str.end())
        throw std::runtime_error("StopIteration()");

    const char c = *selfIter->it;
    ++selfIter->it;
    return new PyStr(c);
}

std::partial_ordering PyStrIter::operator<=>(const PyObj& other) const noexcept {
    if (const PyStrIter* iter = dynamic_cast<const PyStrIter*>(&other))
        return it <=> iter->it;
    return std::partial_ordering::unordered;
}

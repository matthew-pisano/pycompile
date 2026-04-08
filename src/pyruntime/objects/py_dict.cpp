//
// Created by matthew on 4/4/26.
//

#include "pyruntime/objects/py_dict.h"

#include <ranges>
#include <stdexcept>

#include "pyruntime/objects/py_bool.h"
#include "pyruntime/objects/py_int.h"
#include "pyruntime/objects/py_list.h"
#include "pyruntime/objects/py_none.h"
#include "pyruntime/objects/py_tuple.h"
#include "pyruntime/runtime_util.h"


PyObj* PyDict::get(PyObj* self, PyObj** args, const int64_t argc) {
    if (argc != 1)
        throw std::runtime_error("get() takes exactly one argument");
    const PyDict* selfDict = dynamic_cast<PyDict*>(self);
    if (!selfDict)
        throw std::runtime_error("Can only get from dict types");

    if (!selfDict->raw.contains(args[0]))
        return new PyNone();

    PyObj* value = selfDict->raw.at(args[0]);
    value->incref();
    return value;
}


PyObj* PyDict::keys(PyObj* self, PyObj**, const int64_t argc) {
    if (argc != 0)
        throw std::runtime_error("keys() takes no arguments");
    PyDict* selfDict = dynamic_cast<PyDict*>(self);
    if (!selfDict)
        throw std::runtime_error("Can only get keys for dict types");

    const std::vector<PyObj*> keys = selfDict->raw | std::views::keys | std::ranges::to<std::vector>();
    return new PyTuple(keys);
}


PyObj* PyDict::values(PyObj* self, PyObj**, const int64_t argc) {
    if (argc != 0)
        throw std::runtime_error("values() takes no arguments");
    PyDict* selfDict = dynamic_cast<PyDict*>(self);
    if (!selfDict)
        throw std::runtime_error("Can only get values for dict types");

    const std::vector<PyObj*> values = selfDict->raw | std::views::values | std::ranges::to<std::vector>();
    return new PyTuple(values);
}


PyObj* PyDict::items(PyObj* self, PyObj**, const int64_t argc) {
    if (argc != 0)
        throw std::runtime_error("items() takes no arguments");
    PyDict* selfDict = dynamic_cast<PyDict*>(self);
    if (!selfDict)
        throw std::runtime_error("Can only get items for dict types");

    const std::vector<PyObj*> keys = selfDict->raw | std::views::keys | std::ranges::to<std::vector>();
    const std::vector<PyObj*> values = selfDict->raw | std::views::values | std::ranges::to<std::vector>();
    std::vector<PyObj*> items;
    items.reserve(keys.size());

    for (size_t i = 0; i < keys.size(); i++)
        items.push_back(new PyTuple({keys[i], values[i]}));

    return new PyList(items);
}


PyObj* PyDict::update(PyObj* self, PyObj** args, const int64_t argc) {
    if (argc != 1)
        throw std::runtime_error("update() takes exactly one argument");
    PyDict* selfDict = dynamic_cast<PyDict*>(self);
    if (!selfDict)
        throw std::runtime_error("Can only update dict types");

    if (const PyDict* srcDict = dynamic_cast<const PyDict*>(args[0]))
        for (auto& v : srcDict->raw) {
            v.first->incref();
            v.second->incref();
            selfDict->raw.insert(v);
        }
    else
        throw std::runtime_error("Can only update with dict types, got" + args[0]->typeName());

    return new PyNone();
}

PyObj* PyDict::getAttr(const std::string& name) {
    if (name == "update")
        return new PyMethod("update", this, update);
    if (name == "get")
        return new PyMethod("get", this, get);
    if (name == "keys")
        return new PyMethod("keys", this, keys);
    if (name == "values")
        return new PyMethod("values", this, values);
    if (name == "items")
        return new PyMethod("items", this, items);
    throw std::runtime_error(this->typeName() + " has no attribute '" + name + "'");
}

PyInt* PyDict::len() const { return new PyInt(static_cast<int64_t>(raw.size())); }

PyBool* PyDict::contains(const PyObj* obj) const { return new PyBool(raw.contains(const_cast<PyObj*>(obj))); }

PyObj* PyDict::idx(const PyObj* idx) const {
    if (raw.contains(const_cast<PyObj*>(idx))) {
        PyObj* value = raw.at(const_cast<PyObj*>(idx));
        value->incref();
        return value;
    }
    throw std::runtime_error("Key " + idx->toString() + " not found");
}

size_t PyDict::hash() const { throw std::runtime_error("Unhashable type " + typeName()); }

std::string PyDict::toString() const {
    if (raw.empty())
        return "{}";

    std::string result = "{";
    for (const auto& [key, value] : raw)
        result += valueToString(key, true) + ": " + valueToString(value, true) + ", ";
    // Remove ', ' from end
    result.pop_back();
    result.pop_back();
    result += "}";
    return result;
}

std::string PyDict::typeName() const { return "dict"; }

bool PyDict::isTruthy() const { return !raw.empty(); }

PyDictData PyDict::data() const { return raw; }

PyDictData& PyDict::data() { return raw; }

std::partial_ordering PyDict::operator<=>(const PyObj& other) const noexcept {
    if (const PyDict* d = dynamic_cast<const PyDict*>(&other)) {
        if (raw.size() != d->raw.size())
            return raw.size() <=> d->raw.size();

        for (auto& [key, value] : raw) {
            if (!d->raw.contains(key))
                return std::partial_ordering::unordered;
            if (*d->raw.at(key) != *value)
                return std::partial_ordering::unordered;
        }
        return std::partial_ordering::equivalent;
    }
    return std::partial_ordering::unordered;
}

bool PyDict::operator==(const PyObj& other) const noexcept {
    return *this <=> other == std::partial_ordering::equivalent;
}

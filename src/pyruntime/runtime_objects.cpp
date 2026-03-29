//
// Created by matthew on 3/29/26.
//

#include "pyruntime/runtime_objects.h"

#include <stdexcept>

#include "pyruntime/builder_runtime.h"


const std::unordered_map<std::string, PyBoundMethod::SelfFunction> PyList::attrs = {
        {"append", append},
        {"extend", extend},
};


PyValue* PyList::append(PyValue* self, PyValue** args, const int64_t argc) {
    validateSelf(self);
    if (argc != 1)
        throw std::runtime_error("append() takes exactly one argument");
    pyir_listAppend(self, args[0]);
    return new PyValue(PyNone{});
}


PyValue* PyList::extend(PyValue* self, PyValue** args, const int64_t argc) {
    validateSelf(self);
    if (argc != 1)
        throw std::runtime_error("extend() takes exactly one argument");
    pyir_listExtend(self, args[0]);
    return new PyValue(PyNone{});
}


PyValue* PyList::getAttr(PyValue* self, const char* name) {
    validateSelf(self);
    const auto it = attrs.find(name);
    if (it == attrs.end())
        throw std::runtime_error(std::string("list has no attribute '") + name + "'");
    return new PyValue(PyBoundMethod{self, it->second});
}


const std::vector<PyValue*>& PyList::data() const { return rawData; }
std::vector<PyValue*>& PyList::data() { return rawData; }


bool PyList::operator==(const PyList& other) const { return rawData == other.rawData; }


void PyList::validateSelf(const PyValue* self) {
    if (!self || !self->isList())
        throw std::runtime_error("Expected a list type for self");
}

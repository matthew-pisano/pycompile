//
// Created by matthew on 3/29/26.
//

#include "pyruntime/runtime_objects.h"

#include <stdexcept>

#include "pyruntime/builder_runtime.h"
#include "pyruntime/runtime_util.h"


const std::unordered_map<std::string, PyBoundMethod::SelfFunction> PyList::attrs = {
        {"append", append},
        {"extend", extend},
};


PyValue* PyList::append(PyValue* self, PyValue** args, const int64_t argc) {
    if (argc != 1)
        throw std::runtime_error("append() takes exactly one argument");
    pyir_listAppend(self, args[0]);
    return new PyValue(PyNone{});
}


PyValue* PyList::extend(PyValue* self, PyValue** args, const int64_t argc) {
    if (argc != 1)
        throw std::runtime_error("extend() takes exactly one argument");
    pyir_listExtend(self, args[0]);
    return new PyValue(PyNone{});
}


PyValue* PyList::getAttr(PyValue* self, const char* name) {
    const auto it = attrs.find(name);
    if (it == attrs.end())
        throw std::runtime_error(std::string("list has no attribute '") + name + "'");
    return new PyValue(PyBoundMethod{self, it->second});
}


PyValue* PyList::len(const PyValue* self) {
    return new PyValue(static_cast<int64_t>(std::get<PyList>(self->data).data().size()));
}


PyValue* PyList::str(const PyValue* self) { return new PyValue(std::get<PyList>(self->data).toString()); }


std::string PyList::toString() const {
    if (rawData.empty())
        return "[]";

    std::string result = "[";
    for (size_t i = 0; i < rawData.size() - 1; i++)
        result += valueToString(rawData[i], true) + ", ";
    result += valueToString(rawData[rawData.size() - 1], true) + "]";
    return result;
}


const std::vector<PyValue*>& PyList::data() const { return rawData; }
std::vector<PyValue*>& PyList::data() { return rawData; }


bool PyList::operator==(const PyList& other) const { return rawData == other.rawData; }

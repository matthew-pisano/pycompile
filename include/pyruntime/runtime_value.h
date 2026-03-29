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


struct PyValue {
    using Function = PyValue* (*) (PyValue**, int64_t);
    using List = std::vector<PyValue*>;
    using Set = std::unordered_set<PyValue*>;

    struct NoneType {
        bool operator==(const NoneType&) const {
            return true; // None is always None
        }
    };

    struct BoundMethod {
        using SelfFunction = PyValue* (*) (PyValue*, PyValue**, int64_t);

        PyValue* self;
        SelfFunction fn;

        bool operator==(const BoundMethod&) const = default;
    };

    using Data = std::variant<NoneType, bool, int64_t, double, std::string, List, Set, Function, BoundMethod>;

    Data data;
    std::atomic<int32_t> refcount{1};

    // Disable copy/move: Values live on the heap and are managed by refcount
    PyValue(const PyValue&) = delete;

    // Disable constructor from pointer
    explicit PyValue(PyValue*) = delete;

    PyValue& operator=(const PyValue&) = delete;

    explicit PyValue(NoneType) : data(NoneType{}) {}

    explicit PyValue(bool b) : data(b) {}

    explicit PyValue(int64_t i) : data(i) {}

    explicit PyValue(double f) : data(f) {}

    explicit PyValue(std::string s) : data(std::move(s)) {}

    explicit PyValue(const char c) : data(std::string(1, c)) {}

    explicit PyValue(List list) : data(std::move(list)) {}

    explicit PyValue(Set set) : data(std::move(set)) {}

    explicit PyValue(Function f) : data(f) {}

    explicit PyValue(BoundMethod bm) : data(bm) {}

    void incref() { refcount.fetch_add(1, std::memory_order_relaxed); }

    void decref() {
        if (refcount.fetch_sub(1, std::memory_order_acq_rel) == 1)
            delete this;
    }

    [[nodiscard]] bool isNone() const { return std::holds_alternative<NoneType>(data); }
    [[nodiscard]] bool isBool() const { return std::holds_alternative<bool>(data); }
    [[nodiscard]] bool isInt() const { return std::holds_alternative<int64_t>(data); }
    [[nodiscard]] bool isFloat() const { return std::holds_alternative<double>(data); }
    [[nodiscard]] bool isStr() const { return std::holds_alternative<std::string>(data); }
    [[nodiscard]] bool isList() const { return std::holds_alternative<List>(data); }
    [[nodiscard]] bool isSet() const { return std::holds_alternative<Set>(data); }
    [[nodiscard]] bool isFn() const { return std::holds_alternative<Function>(data); }
    [[nodiscard]] bool isBoundMethod() const { return std::holds_alternative<BoundMethod>(data); }
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

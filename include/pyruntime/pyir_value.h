//
// Created by matthew on 3/8/26.
//

#ifndef PYCOMPILE_PYIR_VALUE_H
#define PYCOMPILE_PYIR_VALUE_H
#include <atomic>
#include <string>
#include <variant>
#include <vector>


struct NoneType {};


inline bool operator==(NoneType, NoneType) {
    // Unused args
    return true; // None is always None
}


struct Value {
    using Function = Value* (*) (Value**, int64_t);
    using List = std::vector<Value*>;


    struct BoundMethod {
        using SelfFunction = Value* (*) (Value*, Value**, int64_t);

        Value* self;
        SelfFunction fn;

        bool operator==(const BoundMethod&) const = default;
    };

    using Data = std::variant<NoneType, bool, int64_t, double, std::string, List, Function, BoundMethod>;

    Data data;
    std::atomic<int32_t> refcount{1};

    // Disable copy/move: Values live on the heap and are managed by refcount
    Value(const Value&) = delete;

    // Disable constructor from pointer
    explicit Value(Value*) = delete;

    Value& operator=(const Value&) = delete;

    explicit Value(NoneType) : data(NoneType{}) {}

    explicit Value(bool b) : data(b) {}

    explicit Value(int64_t i) : data(i) {}

    explicit Value(double f) : data(f) {}

    explicit Value(std::string s) : data(std::move(s)) {}

    explicit Value(const char c) : data(std::string(1, c)) {}

    explicit Value(List list) : data(std::move(list)) {}

    explicit Value(Function f) : data(f) {}

    explicit Value(BoundMethod bm) : data(bm) {}

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
    [[nodiscard]] bool isFn() const { return std::holds_alternative<Function>(data); }
    [[nodiscard]] bool isBoundMethod() const { return std::holds_alternative<BoundMethod>(data); }
};


struct ValueRef {
    Value* ptr;

    explicit ValueRef(Value* p) : ptr(p) {}

    // takes ownership, no incref
    ~ValueRef() {
        if (ptr)
            ptr->decref();
    }

    [[nodiscard]] Value* get() const { return ptr; }

    Value* release() {
        Value* p = ptr;
        ptr = nullptr;
        return p;
    }
};


#endif // PYCOMPILE_PYIR_VALUE_H

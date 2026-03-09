//
// Created by matthew on 3/8/26.
//

#ifndef PYCOMPILE_PYIR_VALUE_H
#define PYCOMPILE_PYIR_VALUE_H
#include <string>
#include <variant>

namespace pyir::runtime {
    struct NoneType {
    };


    inline bool operator==(NoneType lhs, NoneType rhs) {
        return true; // None is always None
    }


    struct Value {
        using Fn = Value(*)(Value*, int64_t);
        using Data = std::variant<NoneType, bool, int64_t, double, std::string, Fn>;
        Data data;

        explicit Value(NoneType) :
            data(NoneType{}) {
        }

        explicit Value(bool b) :
            data(b) {
        }

        explicit Value(int64_t i) :
            data(i) {
        }

        explicit Value(double f) :
            data(f) {
        }

        explicit Value(std::string s) :
            data(std::move(s)) {
        }

        explicit Value(Fn f) :
            data(f) {
        }

        bool isNone() const { return std::holds_alternative<NoneType>(data); }
        bool isBool() const { return std::holds_alternative<bool>(data); }
        bool isInt() const { return std::holds_alternative<int64_t>(data); }
        bool isFloat() const { return std::holds_alternative<double>(data); }
        bool isStr() const { return std::holds_alternative<std::string>(data); }
        bool isFn() const { return std::holds_alternative<Fn>(data); }
    };

} //namespace pyir::runtime

#endif //PYCOMPILE_PYIR_VALUE_H

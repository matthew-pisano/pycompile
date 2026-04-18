//
// Created by matthew on 4/18/26.
//

#ifndef PYCOMPILE_RUNTIME_ERRORS_H
#define PYCOMPILE_RUNTIME_ERRORS_H
#include <exception>
#include <string>

class PyException : public std::exception {
public:
    explicit PyException(const std::string& msg) { this->msg = "Exception: " + msg; }

    [[nodiscard]] const char* what() const noexcept override { return msg.c_str(); }

private:
    std::string msg;
};


class PyValueError : public PyException {
public:
    explicit PyValueError(const std::string& msg) : PyException(msg) { this->msg = "ValueError: " + msg; }

    [[nodiscard]] const char* what() const noexcept override { return msg.c_str(); }

private:
    std::string msg;
};


class PyIndexError : public PyException {
public:
    explicit PyIndexError(const std::string& msg) : PyException(msg) { this->msg = "IndexError: " + msg; }

    [[nodiscard]] const char* what() const noexcept override { return msg.c_str(); }

private:
    std::string msg;
};


class PyKeyError : public PyException {
public:
    explicit PyKeyError(const std::string& msg) : PyException(msg) { this->msg = "KeyError: " + msg; }

    [[nodiscard]] const char* what() const noexcept override { return msg.c_str(); }

private:
    std::string msg;
};


class PyTypeError : public PyException {
public:
    explicit PyTypeError(const std::string& msg) : PyException(msg) { this->msg = "TypeError: " + msg; }

    [[nodiscard]] const char* what() const noexcept override { return msg.c_str(); }

private:
    std::string msg;
};


class PyAttributeError : public PyException {
public:
    explicit PyAttributeError(const std::string& msg) : PyException(msg) { this->msg = "AttributeError: " + msg; }

    [[nodiscard]] const char* what() const noexcept override { return msg.c_str(); }

private:
    std::string msg;
};


class PyNameError : public PyException {
public:
    explicit PyNameError(const std::string& msg) : PyException(msg) { this->msg = "NameError: " + msg; }

    [[nodiscard]] const char* what() const noexcept override { return msg.c_str(); }

private:
    std::string msg;
};


class PyStopIteration : public PyException {
public:
    PyStopIteration() : PyException("StopIteration()") { this->msg = "StopIteration()"; }

    [[nodiscard]] const char* what() const noexcept override { return msg.c_str(); }

private:
    std::string msg;
};

#endif // PYCOMPILE_RUNTIME_ERRORS_H

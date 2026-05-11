//
// Created by matthew on 4/18/26.
//

#ifndef PYCOMPILE_RUNTIME_ERRORS_H
#define PYCOMPILE_RUNTIME_ERRORS_H
#include <exception>
#include <string>

/**
 * Base class for all Python exceptions in the runtime.
 */
class PyException : public std::exception {
public:
    explicit PyException(const std::string& msg) { this->msg = "Exception: " + msg; }

    [[nodiscard]] const char* what() const noexcept override { return msg.c_str(); }

private:
    std::string msg;
};


/**
 * Exception class for Python's ValueError.
 */
class PyValueError : public PyException {
public:
    explicit PyValueError(const std::string& msg) : PyException(msg) { this->msg = "ValueError: " + msg; }

    [[nodiscard]] const char* what() const noexcept override { return msg.c_str(); }

private:
    std::string msg;
};


/**
 * Exception class for Python's IndexError.
 */
class PyIndexError : public PyException {
public:
    explicit PyIndexError(const std::string& msg) : PyException(msg) { this->msg = "IndexError: " + msg; }

    [[nodiscard]] const char* what() const noexcept override { return msg.c_str(); }

private:
    std::string msg;
};


/**
 * Exception class for Python's KeyError.
 */
class PyKeyError : public PyException {
public:
    explicit PyKeyError(const std::string& msg) : PyException(msg) { this->msg = "KeyError: " + msg; }

    [[nodiscard]] const char* what() const noexcept override { return msg.c_str(); }

private:
    std::string msg;
};


/**
 * Exception class for Python's TypeError.
 */
class PyTypeError : public PyException {
public:
    explicit PyTypeError(const std::string& msg) : PyException(msg) { this->msg = "TypeError: " + msg; }

    [[nodiscard]] const char* what() const noexcept override { return msg.c_str(); }

private:
    std::string msg;
};


/**
 * Exception class for Python's AttributeError.
 */
class PyAttributeError : public PyException {
public:
    explicit PyAttributeError(const std::string& msg) : PyException(msg) { this->msg = "AttributeError: " + msg; }

    [[nodiscard]] const char* what() const noexcept override { return msg.c_str(); }

private:
    std::string msg;
};


/**
 * Exception class for Python's NameError.
 */
class PyNameError : public PyException {
public:
    explicit PyNameError(const std::string& msg) : PyException(msg) { this->msg = "NameError: " + msg; }

    [[nodiscard]] const char* what() const noexcept override { return msg.c_str(); }

private:
    std::string msg;
};


/**
 * Exception class for Python's StopIteration, used to signal the end of an iterator.
 */
class PyStopIteration : public PyException {
public:
    PyStopIteration() : PyException("StopIteration") { this->msg = "StopIteration"; }

    [[nodiscard]] const char* what() const noexcept override { return msg.c_str(); }

private:
    std::string msg;
};

#endif // PYCOMPILE_RUNTIME_ERRORS_H

//
// Created by matthew on 3/1/26.
//

#ifndef PYCOMPILE_UTILS_H
#define PYCOMPILE_UTILS_H
#include <string>

/**
 * Helper function to extract the full traceback of a Python exception as a string.
 * @return A string containing the traceback of the most recently raised Python exception.
 * @throws std::runtime_error if there is no active Python exception.
 */
std::string getPythonErrorTraceback();

#endif //PYCOMPILE_UTILS_H

//
// Created by matthew on 3/24/26.
//

#ifndef PYCOMPILE_RUNTIME_UTIL_H
#define PYCOMPILE_RUNTIME_UTIL_H
#include <cmath>
#include <string>

#include "pytypes/iterables/py_list.h"
#include "pytypes/iterables/py_set.h"
#include "pytypes/py_object.h"

/**
 * Utility function to decref an array of PyObj* arguments, used by built-in functions to clean up their arguments.
 * @param args The array of PyObj* arguments to decref
 * @param argc The number of arguments in the args array
 */
void decrefArgs(PyObj** args, int64_t argc);


/**
 * Utility function to format an error message for a failed type conversion, used by built-ins to report type errors.
 * @param valType The type of the value that failed to convert
 * @param type The type that the value was being converted to
 * @param valRepr A string representation of the value that failed to convert
 * @return A formatted error message describing the failed conversion
 */
std::string formatBadConversion(const std::string& valType, const std::string& type, const std::string& valRepr);


/**
 * Utility function to convert a compatible PyObj* to a double_t, used by built-ins that require numeric arguments.
 * @param val The PyObj* value to convert to a double_t
 * @return The double_t representation of the value
 * @throws PyTypeError if the value cannot be converted to a double_t
 */
double_t valueToFloat(const PyObj* val);


/**
 * Utility function to convert a PyObj* to a string, used by built-ins that need to represent values as strings.
 * @param val The PyObj* value to convert to a string
 * @param quoteStrings Whether to quote string values in the output (e.g. 'hello' instead of hello)
 * @return A string representation of the value
 */
std::string valueToString(const PyObj* val, bool quoteStrings = false);


/**
 * Utility function to convert a compatible PyObj* to a PyListData, used by built-ins that list conversions.
 * @param val The PyObj* value to convert to a PyListData
 * @return The PyListData representation of the value
 * @throws PyTypeError if the value cannot be converted to a PyListData
 */
PyListData valueToList(const PyObj* val);


/**
 * Utility function to convert a compatible PyObj* to a PySetData, used by built-ins that set conversions.
 * @param val The PyObj* value to convert to a PySetData
 * @return The PySetData representation of the value
 * @throws PyTypeError if the value cannot be converted to a PySetData
 */
PySetData valueToSet(const PyObj* val);

#endif // PYCOMPILE_RUNTIME_UTIL_H

//
// Created by matthew on 3/24/26.
//

#ifndef PYCOMPILE_BUILDER_RUNTIME_H
#define PYCOMPILE_BUILDER_RUNTIME_H
#include <cstdint>


extern "C" {

struct PyObj;

/**
 * Creates a new PyStr* from the given string parts.
 * @param parts An array of string parts for concetanation
 * @param count The size of the array
 * @return A new concatenated PyStr*
 */
PyObj* pyir_buildString(PyObj** parts, int64_t count);

/**
 * Creates a new PyList* from the given object parts.
 * @param parts An array of object parts for adding to the list
 * @param count The size of the array
 * @return A new PyList*
 */
PyObj* pyir_buildList(PyObj** parts, int64_t count);

/**
 * Extends the given list with the given item container.
 * @param list The list to extend
 * @param items A container containing elements to extend the list with
 */
void pyir_listExtend(PyObj* list, PyObj* items);

/**
 * Appends to the given list with the given item.
 * @param list The list to append to
 * @param item A container containing element to append to the list
 */
void pyir_listAppend(PyObj* list, PyObj* item);

/**
 * Creates a new PySet* from the given object parts.
 * @param parts An array of object parts for adding to the set
 * @param count The size of the array
 * @return A new PySet*
 */
PyObj* pyir_buildSet(PyObj** parts, int64_t count);

/**
 * Updates the given set with the given item container.
 * @param set The set to update
 * @param items A container containing elements to update the set with
 */
void pyir_setUpdate(PyObj* set, PyObj* items);

/**
 * Adds to the given set with the given item.
 * @param set The set to add to
 * @param item A container containing element to add to the set
 */
void pyir_setAdd(PyObj* set, PyObj* item);

/**
 * Creates a new PyDict* from the given object parts.
 * @param parts An array of object parts for adding to the dict
 * @param count The size of the array
 * @return A new PyDict*
 */
PyObj* pyir_buildMap(PyObj** parts, int64_t count);

/**
 * Adds to the given dict with the given key-value pair.
 * @param dict The dict to add to
 * @param key The key to add
 * @param value The value to add
 */
void pyir_mapAdd(PyObj* dict, PyObj* key, PyObj* value);
}

#endif // PYCOMPILE_BUILDER_RUNTIME_H

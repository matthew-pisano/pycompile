//
// Created by matthew on 4/12/26.
//

#ifndef PYCOMPILE_CONTROL_FLOW_RUNTIME_H
#define PYCOMPILE_CONTROL_FLOW_RUNTIME_H

extern "C" {

struct PyObj;

/**
 * Get a PyIter* iterator for a container object.
 * @param container The container to get an iterator for.
 * @return A PyIter* iterator for the container of the corresponding type.
 */
PyObj* pyir_getIter(PyObj* container);

/**
 * Get the next item from an iterator.
 * @param iterator The iterator to get the next item from.
 * @return The next item from the iterator, or nullptr if the iterator is exhausted.
 */
PyObj* pyir_forIter(PyObj* iterator);

/**
 * Pop the top iterator from the stack.
 * @return PyNone::None
 */
PyObj* pyir_popIter();
}

#endif // PYCOMPILE_CONTROL_FLOW_RUNTIME_H

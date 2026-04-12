//
// Created by matthew on 4/12/26.
//

#ifndef PYCOMPILE_CONTROL_FLOW_RUNTIME_H
#define PYCOMPILE_CONTROL_FLOW_RUNTIME_H

extern "C" {

struct PyObj;

PyObj* pyir_getIter(PyObj* container);

PyObj* pyir_forIter(PyObj* iterator);
}

#endif // PYCOMPILE_CONTROL_FLOW_RUNTIME_H

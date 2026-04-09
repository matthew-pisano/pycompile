//
// Created by matthew on 4/8/26.
//

#ifndef PYCOMPILE_PYIR_RUN_MODULE_H
#define PYCOMPILE_PYIR_RUN_MODULE_H

extern "C" {
int pyir_runModule(void (*moduleFn)());
const char* pyir_getLastError();
}

#endif // PYCOMPILE_PYIR_RUN_MODULE_H

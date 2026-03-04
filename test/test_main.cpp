//
// Created by matthew on 3/3/26.
//

#include <catch2/catch_all.hpp>
#include <Python.h>


/**
 * A Catch2 event listener that initializes the Python interpreter before any tests are run, and finalizes it after all
 * tests have completed.
 *
 * This ensures that the Python interpreter is available for any tests that need to use it, and
 * that it is properly cleaned up after the tests are done.
 */
struct PythonListener : Catch::EventListenerBase {
    using EventListenerBase::EventListenerBase;

    void testRunStarting(Catch::TestRunInfo const&) override {
        Py_Initialize();
    }

    void testRunEnded(Catch::TestRunStats const&) override {
        if (Py_IsInitialized())
            Py_Finalize();
    }
};


CATCH_REGISTER_LISTENER(PythonListener)

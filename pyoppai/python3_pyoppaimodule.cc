#include <Python.h>

#define PYTHON_MODINIT_FUNC_NAME(modname) PyInit_##modname
#define PYTHON_MODINIT_FUNC_RETURN(varname) return varname;

PyObject* python_init_module(
    char const* name,
    char const* desc,
    PyMethodDef* methods)
{
    static PyModuleDef moduledef =
    {
        PyModuleDef_HEAD_INIT,
        name, desc,
        -1,
        methods,
        0, 0, 0, 0
    };

    return PyModule_Create(&moduledef);
}

#include "pyoppaimodule.cc"

/*
// example embedded main:
int main(int argc, char* argv[])
{
    wchar_t* progname = python_decode_locale(argv[0], 0);
    if (!progname) {
        fprintf(stderr, "Fatal error: cannot decode argv[0]\n");
        exit(1);
    }

    PyImport_AppendInittab(PYOPPAI_NAME, PYTHON_MODINIT_FUNC_NAME(pyoppai));
    Py_SetProgramName(progname);
    Py_Initialize();

    // do shit

    PyMem_RawFree(progname); // probably not needed since we are exiting

    return 0;
}
*/

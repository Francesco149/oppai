#define python_init_module(name, desc, methods) \
    Py_InitModule3(name, methods, desc)

#define PYTHON_MODINIT_FUNC_NAME(modname) init##modname
#define PYTHON_MODINIT_FUNC_RETURN(varname)

#include "pyoppaimodule.cc"

/*
// example embedded main:
int main(int argc, char* argv[])
{
    Py_SetProgramName(argv[0]);
    Py_Initialize();
    PYTHON_MODINIT_FUNC_NAME(pyoppai)();

    // do shit

    return 0;
}
*/

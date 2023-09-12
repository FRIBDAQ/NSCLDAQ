/**
 * @file qtscope.cpp
 * @brief QtScope main.
 */

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <stdexcept>

#include <Python.h>

#include <config.h>
#include "CPyHelper.h"

/**
 * @brief QtScope main.
 *
 * Uses Python C++ API to call main.py to configure and run QtScope.
 *
 * @param argc  Number of command line options.
 * @param argv  The command line options.
 *
 * @return int
 * @retval 0  Success.
 */
int main(int argc, char *argv[])
{  
    CPyHelper pInstance;

    wchar_t* version = Py_DecodeLocale(
	std::to_string(XIAAPI_VERSION).c_str(), NULL
	);
    int pyargc = 1;
    wchar_t* pyargv[1];
    pyargv[0] = version;
  
    try {
	PySys_SetArgv(pyargc, pyargv);
	std::string filename(PREFIX"/ddas/qtscope/main.py");
	PyObject *obj = Py_BuildValue("s", filename.c_str());    
	FILE *file = _Py_fopen_obj(obj, "r");
	if(file != NULL) {
	    PyRun_SimpleFile(file, filename.c_str());
	} else {
	    std::string errmsg = "Cannot open QtScope main from: " + filename;
	    throw std::invalid_argument(errmsg);
	}
    }
    catch (std::exception& e) {
	std::cout << "QtScope main caught an exception: "
		  << e.what() << std::endl;
	exit(EXIT_FAILURE);
    }

    return 0;
}

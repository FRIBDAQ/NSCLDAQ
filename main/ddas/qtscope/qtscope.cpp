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

/**
 * @brief QtScope main.
 *
 * @param argc Number of command line options.
 * @param argv The command line options.
 *
 * @return int
 * @retval 0 Success.
 *
 * @details
 * Uses Python C++ API to call main.py to configure and run QtScope.
 */
int main(int argc, char *argv[]) {
  Py_Initialize();

  try {
    std::string filename(PREFIX "/ddas/qtscope/main.py");
    PyObject *obj = Py_BuildValue("s", filename.c_str());
    FILE *file = _Py_fopen_obj(obj, "r");
    if (file != NULL) {
      PyRun_SimpleFile(file, filename.c_str());
    } else {
      std::string errmsg = "Cannot open QtScope main from: " + filename;
      throw std::invalid_argument(errmsg);
    }
  } catch (std::exception &e) {
    std::cout << "QtScope main caught an exception: " << e.what() << std::endl;
    exit(EXIT_FAILURE);
  }

  Py_Finalize();

  return 0;
}

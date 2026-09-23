#
#  logbook - the electronic logbook: C++ API library, its Python and Tcl
#  bindings, the lg_* programs and the logbook Tcl packages.
#

#---------------------------------------------------------------------------
#  Libraries.

nscldaq_add_library(Logbook
  SOURCES LogBook.cpp LogBookPerson.cpp LogBookShift.cpp LogBookRun.cpp
    LogBookNote.cpp)
target_include_directories(Logbook PRIVATE ${PROJECT_SOURCE_DIR}/base/sqliteplus)
target_link_libraries(Logbook PUBLIC Sqlite3pp NSCLDAQ::SQLite3)

nscldaq_add_library(PyLogbook
  SOURCES PyLogbook.cpp PyLogbookPerson.cpp PyLogBookShift.cpp
    PyLogBookRun.cpp PyNote.cpp)
target_include_directories(PyLogbook PRIVATE ${PROJECT_SOURCE_DIR}/base/sqliteplus)
target_link_libraries(PyLogbook
  PUBLIC Logbook Sqlite3pp NSCLDAQ::SQLite3 NSCLDAQ::Python)

nscldaq_install_headers(LogBook.h LogBookPerson.h LogBookShift.h LogBookRun.h
  LogBookNote.h)

nscldaq_add_library(TclLogbook
  SOURCES LogBookPackage.cpp TclLogbook.cpp TclLogBookInstance.cpp
    TclPersonInstance.cpp TclShiftInstance.cpp TclRunInstance.cpp
    TclNoteInstance.cpp)
target_include_directories(TclLogbook PRIVATE ${PROJECT_SOURCE_DIR}/base/sqliteplus)
target_link_libraries(TclLogbook
  PUBLIC Logbook Sqlite3pp NSCLDAQ::SQLite3 NSCLDAQ::tclPlus NSCLDAQ::Tcl)

#---------------------------------------------------------------------------
#  install-exec-hook

nscldaq_install_mkdir(pythonLibs/nscldaq/LogBook share/html
  share/examples/logbook TclLibs/logbook)
nscldaq_install_copy(lib/libPyLogbook.so pythonLibs/nscldaq/LogBook/LogBook.so)
nscldaq_install_copy(lib/libTclLogbook.so TclLibs/logbook/libTclLogbook.so)
install(PROGRAMS logbookadmin.tcl lg_utilities.tcl ShiftEditor.tcl
  logbookBundle.tcl lg_noteutilities.tcl
  DESTINATION TclLibs/logbook)
foreach(_prog lg_create lg_current lg_ls lg_addperson lg_lspeople lg_mkshift
    lg_mgshift lg_wrnote lg_selshift lg_browse lg_kvstore lg_print)
  install(PROGRAMS ${_prog}.py DESTINATION bin RENAME ${_prog})
endforeach()
install(FILES shifteditor.jpg shiftselector.jpg noteeditor.jpg
  imageselector.jpg lg_browser.jpg lg_peopletab.jpg lg_shifttab.jpg
  lg_logbooktab.jpg
  DESTINATION share/html)
install(FILES kvexample.cpp kvexamplelow.tcl kvexamplehi.tcl kvexample.py
  DESTINATION share/examples/logbook)

# The pkgIndex.tcl is written by hand (the duplicated lg_noteutilities line
# is as the Makefile.am wrote it).

set(_logbook_pkgindex [=[
package ifneeded logbook 1.0 [list load [file join $dir libTclLogbook.so]]
package ifneeded logbookadmin 1.0 [list source [file join $dir logbookadmin.tcl]]
package ifneeded ShiftEditor 1.0 [list source [file join $dir ShiftEditor.tcl]]
package ifneeded lg_utilities 1.0 [list source [file join $dir lg_utilities.tcl]]
package ifneeded lg_noteutilities 1.0 [list source [file join $dir lg_noteutilities.tcl]]
package ifneeded lg_noteutilities 1.0 [list source [file join $dir lg_noteutilities.tcl]]
package ifneeded logbookbundle 1.0 [list source [file join $dir logbookBundle.tcl]]
]=])
set(_logbook_pkgindex_file ${CMAKE_CURRENT_BINARY_DIR}/logbook_pkgIndex.tcl)
file(WRITE ${_logbook_pkgindex_file} "${_logbook_pkgindex}")
install(FILES ${_logbook_pkgindex_file} DESTINATION TclLibs/logbook
  RENAME pkgIndex.tcl)

install(PROGRAMS logbookadmin.py LogBookUIUtilities.py
  DESTINATION pythonLibs/nscldaq/LogBook)

#---------------------------------------------------------------------------
#  Tests.

nscldaq_add_executable(logbook_unittests TEST OUTPUT_NAME unittests
  SOURCES TestRunner.cpp excepttests.cpp bookstatictests.cpp
    logbooktests.cpp persontests.cpp personapitest.cpp shifttests.cpp
    shiftapitest.cpp runtests.cpp runapitests.cpp notetests.cpp
    noteapitests.cpp kvtests.cpp)
target_include_directories(logbook_unittests PRIVATE
  ${PROJECT_SOURCE_DIR}/base/testutils ${PROJECT_SOURCE_DIR}/base/sqliteplus)
target_link_libraries(logbook_unittests PRIVATE
  NSCLDAQ::CppUnit Logbook Sqlite3pp NSCLDAQ::SQLite3)

# check-TESTS

foreach(_t logbookstatictests pykvtests pypersontests pyshifttests
    pyruntests pynotetests)
  nscldaq_add_test(NAME ${_t}
    COMMAND ${PYTHON} ${CMAKE_CURRENT_SOURCE_DIR}/${_t}.py
    ENVIRONMENT PYTHONPATH=${prefix}/pythonLibs/nscldaq)
endforeach()
foreach(_t logbook logbookinstance personinstance shiftinstance
    runinstance noteinstance admin noteutil)
  nscldaq_add_tcl_test(NAME ${_t}.test SCRIPT ${_t}.test)
endforeach()
nscldaq_add_test(NAME logbookadmin
  COMMAND ${PYTHON} ${CMAKE_CURRENT_SOURCE_DIR}/logbookadmin.py
  ENVIRONMENT PYTHONPATH=${prefix}/pythonLibs)
nscldaq_add_test(NAME unittests COMMAND logbook_unittests)

# The Makefile.am's per-target CXXFLAGS did not include $(AM_CXXFLAGS), so
# these were compiled without -fno-strict-aliasing.
nscldaq_no_am_cxxflags(Logbook PyLogbook TclLogbook)

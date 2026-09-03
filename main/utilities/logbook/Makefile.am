
# I Saw a dependency? issue make parallel make fail here so for now:

.NOTPARALLEL:

lib_LTLIBRARIES=libLogbook.la libPyLogbook.la libTclLogbook.la

libLogbook_la_SOURCES=LogBook.cpp LogBookPerson.cpp LogBookShift.cpp \
	LogBookRun.cpp LogBookNote.cpp
libLogbook_la_CXXFLAGS=@SQLITE3_CFLAGS@ -I@top_srcdir@/base/sqliteplus
libLogbook_la_LDFLAGS=@top_builddir@/base/sqliteplus/libSqlite3pp.la \
	@SQLITE3_LIBS@


libPyLogbook_la_DEPENDENCIES=libLogbook.la
libPyLogbook_la_CXXFLAGS=$(libLogbook_la_CXXFLAGS) @PYTHON3_CFLAGS@
libPyLogbook_la_LDFLAGS= libLogbook.la $(libLogbook_la_LDFLAGS) @PYTHON3_LIBS@
libPyLogbook_la_SOURCES=PyLogbook.cpp PyLogbookPerson.cpp PyLogbook.h \
	PyLogBookPerson.h PyLogBookShift.h PyLogBookShift.cpp	\
	PyLogBookRun.h PyLogBookRun.cpp PyNote.h PyNote.cpp

PY_PACKAGES=logbookadmin.py LogBookUIUtilities.py

include_HEADERS=LogBook.h LogBookPerson.h LogBookShift.h LogBookRun.h \
	LogBookNote.h

libTclLogbook_la_SOURCES=LogBookPackage.cpp TclLogbook.h TclLogbook.cpp \
	TclLogBookInstance.h TclLogBookInstance.cpp \
	TclPersonInstance.h TclPersonInstance.cpp \
	TclShiftInstance.h TclShiftInstance.cpp	\
	TclRunInstance.h TclRunInstance.cpp  \
	TclNoteInstance.h TclNoteInstance.cpp

libTclLogbook_la_CXXFLAGS=$(libLogbook_la_CXXFLAGS) @LIBTCLPLUS_CFLAGS@ @TCL_CPPFLAGS@
libTclLogbook_la_LDFLAGS=libLogbook.la $(libLogbook_la_LDFLAGS) @LIBTCLPLUS_LDFLAGS@ \
	@TCL_LDFLAGS@


FIGURES=shifteditor.jpg shiftselector.jpg noteeditor.jpg imageselector.jpg \
	lg_browser.jpg lg_peopletab.jpg lg_shifttab.jpg lg_logbooktab.jpg

EXAMPLES=kvexample.cpp kvexamplelow.tcl kvexamplehi.tcl kvexample.py

install-exec-hook:
	$(mkinstalldirs) @prefix@/pythonLibs @prefix@/pythonLibs/nscldaq
	$(mkinstalldirs) @prefix@/pythonLibs/nscldaq/LogBook
	$(mkinstalldirs) @prefix/share/html
	$(mkinstalldirs) @prefix@/share/examples/logbook
	$(INSTALL_PROGRAM) @libdir@/libPyLogbook.so @prefix@/pythonLibs/nscldaq/LogBook/LogBook.so
	$(mkinstalldirs) @prefix@/TclLibs @prefix@/TclLibs/logbook
	$(INSTALL_PROGRAM) @libdir@/libTclLogbook.so @prefix@/TclLibs/logbook
	$(INSTALL_PROGRAM) @srcdir@/logbookadmin.tcl @prefix@/TclLibs/logbook
	$(INSTALL_PROGRAM) @srcdir@/lg_utilities.tcl @prefix@/TclLibs/logbook
	$(INSTALL_PROGRAM) @srcdir@/ShiftEditor.tcl @prefix@/TclLibs/logbook
	$(INSTALL_PROGRAM) @srcdir@/logbookBundle.tcl @prefix@/TclLibs/logbook
	$(INSTALL_PROGRAM) @srcdir@/lg_noteutilities.tcl \
		@prefix@/TclLibs/logbook
	$(INSTALL_PROGRAM) @srcdir@/lg_create.py @bindir@/lg_create
	$(INSTALL_PROGRAM) @srcdir@/lg_current.py @bindir@/lg_current
	$(INSTALL_PROGRAM) @srcdir@/lg_ls.py @bindir@/lg_ls
	$(INSTALL_PROGRAM) @srcdir@/lg_addperson.py @bindir@/lg_addperson
	$(INSTALL_PROGRAM) @srcdir@/lg_lspeople.py @bindir@/lg_lspeople
	$(INSTALL_PROGRAM) @srcdir@/lg_mkshift.py @bindir@/lg_mkshift
	$(INSTALL_PROGRAM) @srcdir@/lg_mgshift.py @bindir@/lg_mgshift
	$(INSTALL_PROGRAM) @srcdir@/lg_wrnote.py @bindir@/lg_wrnote
	$(INSTALL_PROGRAM) @srcdir@/lg_selshift.py @bindir@/lg_selshift
	$(INSTALL_PROGRAM) @srcdir@/lg_browse.py @bindir@/lg_browse
	$(INSTALL_PROGRAM) @srcdir@/lg_kvstore.py @bindir@/lg_kvstore
	$(INSTALL_PROGRAM) @srcdir@/lg_print.py @bindir@/lg_print
	for f in $(FIGURES); do $(INSTALL_DATA) @srcdir@/$$f \
		@prefix@/share/html; done
	for f in $(EXAMPLES); do $(INSTALL_DATA) @srcdir@/$$f \
		@prefix@/share/examples/logbook; done
	echo package ifneeded logbook 1.0 [list load [file join \$$dir libTclLogbook.so]] \
		> @prefix@/TclLibs/logbook/pkgIndex.tcl
	echo package ifneeded logbookadmin 1.0 [list source [file join \$$dir logbookadmin.tcl]] \
		>> @prefix@/TclLibs/logbook/pkgIndex.tcl
	echo package ifneeded ShiftEditor 1.0 [list source [file join \$$dir ShiftEditor.tcl]] \
		>> @prefix@/TclLibs/logbook/pkgIndex.tcl
	echo package ifneeded lg_utilities  1.0 [list source [file join \$$dir lg_utilities.tcl]] \
		>> @prefix@/TclLibs/logbook/pkgIndex.tcl
	echo package ifneeded lg_noteutilities  1.0 [list source [file join \$$dir lg_noteutilities.tcl]] \
		>> @prefix@/TclLibs/logbook/pkgIndex.tcl
	echo package ifneeded lg_noteutilities  1.0 [list source [file join \$$dir lg_noteutilities.tcl]] \
		>> @prefix@/TclLibs/logbook/pkgIndex.tcl
	echo package ifneeded logbookbundle  1.0 [list source [file join \$$dir logbookBundle.tcl]] \
		>> @prefix@/TclLibs/logbook/pkgIndex.tcl
	(cd @srcdir@; $(INSTALL_SCRIPT) $(PY_PACKAGES) @prefix@/pythonLibs/nscldaq/LogBook)





#----------------------- tests ------------------------------------

check_PROGRAMS=unittests

unittests_SOURCES=TestRunner.cpp logtestutils.hpp excepttests.cpp \
	bookstatictests.cpp logbooktests.cpp persontests.cpp \
	personapitest.cpp shifttests.cpp shiftapitest.cpp runtests.cpp \
	runapitests.cpp notetests.cpp noteapitests.cpp kvtests.cpp

unittests_CXXFLAGS=@CPPUNIT_CFLAGS@ -I@top_srcdir@/base/testutils \
	$(libLogbook_la_CXXFLAGS)
unittests_LDFLAGS=libLogbook.la $(libLogbook_la_LDFLAGS) @CPPUNIT_LDFLAGS@
unittests_DEPENDENCIES=libLogbook.la




check-TESTS:
	PYTHONPATH=@prefix@/pythonLibs/nscldaq @PYTHON@ @srcdir@/logbookstatictests.py
	PYTHONPATH=@prefix@/pythonLibs/nscldaq @PYTHON@ @srcdir@/pykvtests.py
	PYTHONPATH=@prefix@/pythonLibs/nscldaq @PYTHON@ @srcdir@/pypersontests.py
	PYTHONPATH=@prefix@/pythonLibs/nscldaq @PYTHON@ @srcdir@/pyshifttests.py
	PYTHONPATH=@prefix@/pythonLibs/nscldaq @PYTHON@ @srcdir@/pyruntests.py
	PYTHONPATH=@prefix@/pythonLibs/nscldaq @PYTHON@ @srcdir@/pynotetests.py
	TCLLIBPATH=@prefix@/TclLibs @TCLSH_CMD@ @srcdir@/logbook.test
	TCLLIBPATH=@prefix@/TclLibs @TCLSH_CMD@ @srcdir@/logbookinstance.test
	TCLLIBPATH=@prefix@/TclLibs @TCLSH_CMD@ @srcdir@/personinstance.test
	TCLLIBPATH=@prefix@/TclLibs @TCLSH_CMD@ @srcdir@/shiftinstance.test
	TCLLIBPATH=@prefix@/TclLibs @TCLSH_CMD@ @srcdir@/runinstance.test
	TCLLIBPATH=@prefix@/TclLibs @TCLSH_CMD@ @srcdir@/noteinstance.test
	TCLLIBPATH=@prefix@/TclLibs @TCLSH_CMD@ @srcdir@/admin.test
	TCLLIBPATH=@prefix@/TclLibs @TCLSH_CMD@ @srcdir@/noteutil.test
	PYTHONPATH=@prefix@/pythonLibs @PYTHON@ @srcdir@/logbookadmin.py
	@builddir@/unittests



check: check-TESTS


EXTRA_DIST=logbookstatictests.py pypersontests.py pyshifttests.py \
	pyruntests.py pynotetests.py logbook.test logbookinstance.test \
	personinstance.test shiftinstance.test runinstance.test \
	noteinstance.test logbookadmin.tcl admin.test lg_create.py \
	lg_current.py lg_ls.py lg_addperson.py lg_lspeople.py \
	lg_mkshift.py ShiftEditor.tcl lg_mgshift.py lg_utilities.tcl \
	lg_noteutilities.tcl noteutil.test lg_wrnote.py logbookBundle.tcl \
	lg_selshift.py pykvtests.py lg_browse.py lg_kvstore.py lg_print.py \
	logbookdocs.xml \
	$(FIGURES) $(EXAMPLES)

set(_sqliteplus_sources CSqlite.cpp CSqliteException.cpp CSqliteStatement.cpp
  CSqliteTransaction.cpp CSqliteWhere.cpp)

nscldaq_add_library(Sqlite3pp SOURCES ${_sqliteplus_sources})
target_link_libraries(Sqlite3pp PUBLIC NSCLDAQ::SQLite3)

nscldaq_install_headers(CSqlite.h CSqliteException.h CSqliteStatement.h
  CSqliteTransaction.h CSqliteWhere.h)

#  noinst_PROGRAMS (built, never installed or run by 'make check').

nscldaq_add_executable(sqliteplus_unittests NOINST OUTPUT_NAME unittests
  SOURCES TestRunner.cpp exceptionTest.cpp connectionTest.cpp
    statementTest.cpp transactionTest.cpp ${_sqliteplus_sources})
target_include_directories(sqliteplus_unittests PRIVATE
  ${PROJECT_SOURCE_DIR}/base/testutils)
target_link_libraries(sqliteplus_unittests PRIVATE
  NSCLDAQ::SQLite3 NSCLDAQ::CppUnit)

# The Makefile.am's per-target CXXFLAGS did not include $(AM_CXXFLAGS), so
# these were compiled without -fno-strict-aliasing.
nscldaq_no_am_cxxflags(Sqlite3pp sqliteplus_unittests)

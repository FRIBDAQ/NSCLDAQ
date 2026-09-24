#
#  NSCLDAQHelpers.cmake
#
#  Functions shared by the per-directory CMakeLists.txt files.  They capture
#  the automake conventions the Makefile.am files relied on:
#
#    nscldaq_add_library        lib_LTLIBRARIES / noinst_LTLIBRARIES
#    nscldaq_add_executable     bin_PROGRAMS / noinst_PROGRAMS / check_PROGRAMS
#    nscldaq_no_am_cxxflags     target whose foo_CXXFLAGS omit $(AM_CXXFLAGS)
#    nscldaq_add_test           TESTS
#    nscldaq_add_tcl_test       TESTS that run tclTests.tcl with TCLLIBPATH=...
#    nscldaq_gengetopt          foosw.ggo -> foosw.c/foosw.h
#    nscldaq_use_gengetopt      add those generated files to a target
#    nscldaq_install_headers    include_HEADERS (+ staging for -I@prefix@/include)
#    nscldaq_install_mkdir      $(mkinstalldirs) ...
#    nscldaq_install_tcl_index  echo "pkg_mkIndex ..." | $(TCLSH_CMD)
#    nscldaq_install_copy       copy an installed file elsewhere in the prefix
#    nscldaq_install_code       arbitrary shell-ish install step
#    nscldaq_configure_file     AC_CONFIG_FILES for a non-Makefile .in file
#
#  External dependencies are NSCLDAQ::<Name> targets (see
#  NSCLDAQDependencies.cmake and NSCLDAQIncorp.cmake); NSCLDAQ::<Name>Headers
#  are their compile-only halves.
#
#  Conventions:
#  * Library target names are the library name without 'lib' (libDataFlow.la
#    -> DataFlow).  Installed program target names are the program name.
#    Programs that are not installed (tests, noinst) are named
#    <something unique>_<program> with OUTPUT_NAME <program> since names
#    like 'unittests' repeat in many directories.
#  * Install destinations are relative to CMAKE_INSTALL_PREFIX so DESTDIR
#    and 'cmake --install --prefix' work.
#

include_guard(GLOBAL)

#---------------------------------------------------------------------------
#  Autoconf-style directory variables for configured files.

set(prefix "${CMAKE_INSTALL_PREFIX}")
set(exec_prefix "${prefix}")
set(bindir "${prefix}/bin")
set(libdir "${prefix}/lib")
set(includedir "${prefix}/include")
set(datarootdir "${prefix}/share")
set(datadir "${datarootdir}")
set(sysconfdir "${prefix}/etc")
set(SOVERSION "11:0:0")

#---------------------------------------------------------------------------
#  Header-only variants of the external dependency targets.
#
#  Makefile.am files often used only the compile half of a flag pair, e.g.
#  @PIXIE_CPPFLAGS@ without @PIXIE_LDFLAGS@ or @TCL_FLAGS@ without
#  @TCL_LDFLAGS@.  NSCLDAQ::<Name>Headers carries only the include
#  directories, definitions, options and compile features of NSCLDAQ::<Name>
#  (and whatever it links) so such a target does not gain the libraries.

function(_nscldaq_collect_compile tgt)
  get_property(_seen GLOBAL PROPERTY _NSCLDAQ_COLLECT_SEEN)
  if(tgt IN_LIST _seen)
    return()
  endif()
  set_property(GLOBAL APPEND PROPERTY _NSCLDAQ_COLLECT_SEEN ${tgt})
  foreach(_p INTERFACE_INCLUDE_DIRECTORIES INTERFACE_COMPILE_DEFINITIONS
      INTERFACE_COMPILE_OPTIONS INTERFACE_COMPILE_FEATURES)
    get_target_property(_v ${tgt} ${_p})
    if(_v)
      set_property(GLOBAL APPEND PROPERTY _NSCLDAQ_COLLECT_${_p} ${_v})
    endif()
  endforeach()
  get_target_property(_deps ${tgt} MANUALLY_ADDED_DEPENDENCIES)
  if(_deps)
    set_property(GLOBAL APPEND PROPERTY _NSCLDAQ_COLLECT_DEPS ${_deps})
  endif()
  get_target_property(_links ${tgt} INTERFACE_LINK_LIBRARIES)
  if(_links)
    foreach(_l IN LISTS _links)
      if(TARGET ${_l})
        _nscldaq_collect_compile(${_l})
      endif()
    endforeach()
  endif()
endfunction()

function(_nscldaq_headers_target name)
  foreach(_p SEEN INTERFACE_INCLUDE_DIRECTORIES INTERFACE_COMPILE_DEFINITIONS
      INTERFACE_COMPILE_OPTIONS INTERFACE_COMPILE_FEATURES DEPS)
    set_property(GLOBAL PROPERTY _NSCLDAQ_COLLECT_${_p} "")
  endforeach()
  _nscldaq_collect_compile(NSCLDAQ::${name})
  add_library(NSCLDAQ::${name}Headers INTERFACE IMPORTED GLOBAL)
  foreach(_p INTERFACE_INCLUDE_DIRECTORIES INTERFACE_COMPILE_DEFINITIONS
      INTERFACE_COMPILE_OPTIONS INTERFACE_COMPILE_FEATURES)
    get_property(_v GLOBAL PROPERTY _NSCLDAQ_COLLECT_${_p})
    if(_v)
      list(REMOVE_DUPLICATES _v)
      set_property(TARGET NSCLDAQ::${name}Headers PROPERTY ${_p} ${_v})
    endif()
  endforeach()
  get_property(_deps GLOBAL PROPERTY _NSCLDAQ_COLLECT_DEPS)
  if(_deps)
    list(REMOVE_DUPLICATES _deps)
    add_dependencies(NSCLDAQ::${name}Headers ${_deps})
  endif()
endfunction()

foreach(_dep Tcl TclOnly tclPlus Exception Pixie ROOT UFMT DDASFormat Python
    CppUnit BoostLog EPICS USB libusb_1_0 libusb CAEN CAENNG MVLC JsonCpp
    YamlCpp SQLite3 OpenSSL MPI)
  _nscldaq_headers_target(${_dep})
endforeach()

#---------------------------------------------------------------------------
#  Testing.  Test programs are built with 'all' when BUILD_TESTING is on.
#  Like 'make check' after 'make install', most tests expect NSCLDAQ to be
#  installed (Tcl tests load packages from ${prefix}/TclLibs).  The 'check'
#  target runs ctest.

option(BUILD_TESTING "Build the unit tests" ON)
if(BUILD_TESTING)
  enable_testing()
  add_custom_target(check
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
    USES_TERMINAL)
endif()

# Directory of the current CMakeLists.txt relative to main/ - used to make
# test names unique.

function(_nscldaq_reldir out)
  file(RELATIVE_PATH _rel ${PROJECT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
  if(_rel STREQUAL "")
    set(_rel ".")
  endif()
  set(${out} "${_rel}" PARENT_SCOPE)
endfunction()

#---------------------------------------------------------------------------
#  AM_CXXFLAGS.
#
#  configure.ac set AM_CXXFLAGS=-fno-strict-aliasing.  Automake applies
#  AM_CXXFLAGS to the C++ (not C) compilations of every target that has no
#  foo_CXXFLAGS of its own; a target with foo_CXXFLAGS gets it only if those
#  include $(AM_CXXFLAGS).  So:
#
#  * every target made by nscldaq_add_library/nscldaq_add_executable gets
#    ${NSCLDAQ_AM_CXXFLAGS} on its C++ sources.  A directory can change the
#    value for itself and its subdirectories (usb/vmusb passed
#    AM_CXXFLAGS=$(WORKAROUNDS) down with AM_MAKEFLAGS).
#  * nscldaq_no_am_cxxflags(<targets>) removes it for targets whose
#    foo_CXXFLAGS did not include $(AM_CXXFLAGS).

set(NSCLDAQ_AM_CXXFLAGS -fno-strict-aliasing)

function(_nscldaq_apply_am_cxxflags tgt)
  set_property(TARGET ${tgt} PROPERTY NSCLDAQ_AM_CXXFLAGS ${NSCLDAQ_AM_CXXFLAGS})
  target_compile_options(${tgt} PRIVATE
    "$<$<COMPILE_LANGUAGE:CXX>:$<TARGET_PROPERTY:NSCLDAQ_AM_CXXFLAGS>>")
endfunction()

function(nscldaq_no_am_cxxflags)
  foreach(_t IN LISTS ARGN)
    set_property(TARGET ${_t} PROPERTY NSCLDAQ_AM_CXXFLAGS "")
  endforeach()
endfunction()

#---------------------------------------------------------------------------
#  nscldaq_version_info(<current:revision:age> <version-var> <soversion-var>)
#
#  libtool -version-info c:r:a  ->  libX.so.(c-a).a.r with SONAME libX.so.(c-a)

function(nscldaq_version_info info version_var soversion_var)
  string(REPLACE ":" ";" _parts "${info}")
  list(LENGTH _parts _n)
  if(NOT _n EQUAL 3)
    message(FATAL_ERROR "bad libtool version-info '${info}'")
  endif()
  list(GET _parts 0 _c)
  list(GET _parts 1 _r)
  list(GET _parts 2 _a)
  math(EXPR _major "${_c} - ${_a}")
  set(${version_var} "${_major}.${_a}.${_r}" PARENT_SCOPE)
  set(${soversion_var} "${_major}" PARENT_SCOPE)
endfunction()

#---------------------------------------------------------------------------
#  nscldaq_add_library(<target>
#       [OUTPUT_NAME <name>]         default <target>
#       [VERSION_INFO <c:r:a>]       libtool -version-info; default 0:0:0
#       [NOINST]                     noinst_LTLIBRARIES (static convenience lib)
#       [DESTINATION <dir>]          default lib
#       SOURCES <files...>)

function(nscldaq_add_library tgt)
  cmake_parse_arguments(A "NOINST" "OUTPUT_NAME;VERSION_INFO;DESTINATION" "SOURCES" ${ARGN})
  if(A_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR "nscldaq_add_library(${tgt}): unknown arguments ${A_UNPARSED_ARGUMENTS}")
  endif()
  if(A_NOINST)
    add_library(${tgt} STATIC ${A_SOURCES})
  else()
    add_library(${tgt} SHARED ${A_SOURCES})
    if(NOT A_VERSION_INFO)
      set(A_VERSION_INFO "0:0:0")
    endif()
    nscldaq_version_info(${A_VERSION_INFO} _ver _sover)
    set_target_properties(${tgt} PROPERTIES VERSION ${_ver} SOVERSION ${_sover})
    if(NOT A_DESTINATION)
      set(A_DESTINATION lib)
    endif()
    install(TARGETS ${tgt} LIBRARY DESTINATION ${A_DESTINATION})
    # libtool also built and installed a static libFoo.a next to libFoo.so;
    # nscldaq_add_static_libraries() creates those at the end.
    set_property(GLOBAL APPEND PROPERTY _NSCLDAQ_INSTALLED_LIBRARIES ${tgt})
    set_property(TARGET ${tgt} PROPERTY NSCLDAQ_LIBRARY_DESTINATIONS ${A_DESTINATION})
  endif()
  if(A_OUTPUT_NAME)
    set_target_properties(${tgt} PROPERTIES OUTPUT_NAME ${A_OUTPUT_NAME})
  endif()
  _nscldaq_apply_am_cxxflags(${tgt})
endfunction()

#  nscldaq_install_library_copy(<target> <destination>)
#
#  Install an nscldaq_add_library() library a second time, e.g. into TclLibs
#  ($(INSTALL_SCRIPT) .libs/libFoo.* @prefix@/TclLibs), with its static
#  archive, as copying libtool's .libs/libFoo.* did.

function(nscldaq_install_library_copy tgt dest)
  install(TARGETS ${tgt} LIBRARY DESTINATION ${dest})
  set_property(TARGET ${tgt} APPEND PROPERTY NSCLDAQ_LIBRARY_DESTINATIONS ${dest})
endfunction()

#---------------------------------------------------------------------------
#  Static libraries.
#
#  libtool built every installed library twice: shared (PIC) and static
#  (non-PIC), and installed libFoo.a beside libFoo.so.  To do the same, each
#  nscldaq_add_library() library gets a <target>_static archive, created once
#  every directory has been processed (so all target_* calls made on the
#  shared library are known) by nscldaq_add_static_libraries(), called at the
#  end of the top level CMakeLists.txt.  The archive is compiled from the same
#  sources with the same compile settings, and installed wherever the shared
#  library is.  -DBUILD_STATIC_LIBS=OFF is the equivalent of
#  configure --disable-static.

option(BUILD_STATIC_LIBS "Also build and install static (.a) libraries, as libtool did" ON)

function(_nscldaq_add_static_library tgt)
  get_target_property(_srcdir ${tgt} SOURCE_DIR)
  get_target_property(_bindir ${tgt} BINARY_DIR)
  get_target_property(_srcs ${tgt} SOURCES)
  set(_abs "")
  foreach(_s IN LISTS _srcs)
    if(_s MATCHES "^\\$<")
      list(APPEND _abs "${_s}")
      continue()
    endif()
    if(IS_ABSOLUTE "${_s}")
      set(_f "${_s}")
    elseif(EXISTS "${_srcdir}/${_s}")
      set(_f "${_srcdir}/${_s}")
    else()
      set(_f "${_bindir}/${_s}")
    endif()
    # Sources generated in the library's own directory (gengetopt, SWIG,
    # rootcling, ...) must be marked generated here too; the library target
    # (a dependency below) runs their rules.
    get_source_file_property(_gen "${_f}" DIRECTORY "${_srcdir}" GENERATED)
    if(_gen OR NOT EXISTS "${_f}")
      set_source_files_properties("${_f}" PROPERTIES GENERATED TRUE)
    endif()
    list(APPEND _abs "${_f}")
  endforeach()

  set(_st ${tgt}_static)
  add_library(${_st} STATIC ${_abs})
  add_dependencies(${_st} ${tgt})

  get_target_property(_name ${tgt} OUTPUT_NAME)
  if(NOT _name)
    set(_name ${tgt})
  endif()
  set_target_properties(${_st} PROPERTIES
    OUTPUT_NAME ${_name}
    ARCHIVE_OUTPUT_DIRECTORY ${_bindir}/static
    POSITION_INDEPENDENT_CODE OFF)

  # Same compile settings as the shared library: its own properties (which
  # include its directory's include_directories()/definitions), the
  # automake -I. -I$(srcdir) for its directory, and the usage requirements
  # of everything it links.
  # The include path in the shared library's exact order: its directory's
  # -I. -I$(srcdir) first (CMAKE_INCLUDE_CURRENT_DIR), then its own list
  # (which starts with the top build directory).  Order matters: some
  # directories have their own config.h.
  get_target_property(_incs ${tgt} INCLUDE_DIRECTORIES)
  if(NOT _incs)
    set(_incs "")
  endif()
  set_property(TARGET ${_st} PROPERTY INCLUDE_DIRECTORIES ${_bindir} ${_srcdir} ${_incs})
  # Other list properties are appended; single-valued ones are copied
  # (newer CMake refuses to append to e.g. C_STANDARD).
  foreach(_p COMPILE_DEFINITIONS COMPILE_OPTIONS COMPILE_FEATURES)
    get_target_property(_v ${tgt} ${_p})
    if(NOT _v STREQUAL "_v-NOTFOUND")
      set_property(TARGET ${_st} APPEND PROPERTY ${_p} ${_v})
    endif()
  endforeach()
  foreach(_p C_STANDARD C_STANDARD_REQUIRED C_EXTENSIONS CXX_STANDARD
      CXX_STANDARD_REQUIRED CXX_EXTENSIONS NSCLDAQ_AM_CXXFLAGS)
    get_target_property(_v ${tgt} ${_p})
    if(NOT _v STREQUAL "_v-NOTFOUND")
      set_property(TARGET ${_st} PROPERTY ${_p} ${_v})
    endif()
  endforeach()
  get_target_property(_links ${tgt} LINK_LIBRARIES)
  if(_links)
    target_link_libraries(${_st} PRIVATE ${_links})
  endif()

  get_target_property(_dests ${tgt} NSCLDAQ_LIBRARY_DESTINATIONS)
  foreach(_d IN LISTS _dests)
    install(TARGETS ${_st} ARCHIVE DESTINATION ${_d})
  endforeach()
endfunction()

function(nscldaq_add_static_libraries)
  if(NOT BUILD_STATIC_LIBS)
    return()
  endif()
  get_property(_libs GLOBAL PROPERTY _NSCLDAQ_INSTALLED_LIBRARIES)
  foreach(_t IN LISTS _libs)
    _nscldaq_add_static_library(${_t})
  endforeach()
endfunction()

#---------------------------------------------------------------------------
#  nscldaq_add_executable(<target>
#       [OUTPUT_NAME <name>]         default <target>
#       [NOINST]                     noinst_PROGRAMS: built, not installed
#       [TEST]                       check_PROGRAMS: built if BUILD_TESTING
#       [DESTINATION <dir>]          default bin
#       SOURCES <files...>)
#
#  With BUILD_TESTING off a TEST executable still exists (so the usual
#  target_* calls work) but is excluded from 'all'.

function(nscldaq_add_executable tgt)
  cmake_parse_arguments(A "NOINST;TEST" "OUTPUT_NAME;DESTINATION" "SOURCES" ${ARGN})
  if(A_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR "nscldaq_add_executable(${tgt}): unknown arguments ${A_UNPARSED_ARGUMENTS}")
  endif()
  if(A_TEST AND NOT BUILD_TESTING)
    add_executable(${tgt} EXCLUDE_FROM_ALL ${A_SOURCES})
  else()
    add_executable(${tgt} ${A_SOURCES})
  endif()
  if(A_OUTPUT_NAME)
    set_target_properties(${tgt} PROPERTIES OUTPUT_NAME ${A_OUTPUT_NAME})
  endif()
  _nscldaq_apply_am_cxxflags(${tgt})
  if(NOT A_NOINST AND NOT A_TEST)
    if(NOT A_DESTINATION)
      set(A_DESTINATION bin)
    endif()
    install(TARGETS ${tgt} RUNTIME DESTINATION ${A_DESTINATION})
  endif()
endfunction()

#---------------------------------------------------------------------------
#  nscldaq_add_test(NAME <name> COMMAND <cmd...>
#       [ENVIRONMENT <VAR=value>...]
#       [WORKING_DIRECTORY <dir>])    default CMAKE_CURRENT_BINARY_DIR
#
#  The test is registered as <dir-relative-to-main>/<name>.  COMMAND may name
#  an executable target.
#
#  Tests run with ${TCLSH_CMD} are judged by their tcltest summary line
#  ("Total N Passed P Skipped S Failed F"), not the exit status: the tcltest
#  drivers exit 0 even when tests fail (as they did under 'make check').  The
#  test passes only if a summary is printed and fails if any count Failed.

function(nscldaq_add_test)
  if(NOT BUILD_TESTING)
    return()
  endif()
  cmake_parse_arguments(A "" "NAME;WORKING_DIRECTORY" "COMMAND;ENVIRONMENT" ${ARGN})
  if(NOT A_WORKING_DIRECTORY)
    set(A_WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
  endif()
  _nscldaq_reldir(_rel)
  set(_name "${_rel}/${A_NAME}")
  add_test(NAME ${_name} COMMAND ${A_COMMAND}
    WORKING_DIRECTORY ${A_WORKING_DIRECTORY})
  if(A_ENVIRONMENT)
    set_tests_properties(${_name} PROPERTIES ENVIRONMENT "${A_ENVIRONMENT}")
  endif()
  list(GET A_COMMAND 0 _cmd0)
  if(_cmd0 STREQUAL TCLSH_CMD OR _cmd0 STREQUAL "${TCLSH_CMD}")
    set(_summary "Total[ \t]+[0-9]+[ \t]+Passed[ \t]+[0-9]+[ \t]+Skipped[ \t]+[0-9]+[ \t]+Failed")
    set_tests_properties(${_name} PROPERTIES
      PASS_REGULAR_EXPRESSION "${_summary}"
      FAIL_REGULAR_EXPRESSION "${_summary}[ \t]+[1-9]")
  endif()
endfunction()

#---------------------------------------------------------------------------
#  nscldaq_add_tcl_test(NAME <name>
#       [SCRIPT <script>]              default tclTests.tcl (in the source dir)
#       [TCLLIBPATH <dirs...>]         default ${prefix}/TclLibs
#       [ENVIRONMENT <VAR=value>...])
#
#  The usual Makefile.am idiom:
#     TCLLIBPATH=@prefix@/TclLibs HERE=@srcdir@ tcl=@TCLSH_CMD@ \
#        @TCLSH_CMD@ @srcdir@/tclTests.tcl

function(nscldaq_add_tcl_test)
  cmake_parse_arguments(A "" "NAME;SCRIPT" "TCLLIBPATH;ENVIRONMENT" ${ARGN})
  if(NOT A_SCRIPT)
    set(A_SCRIPT tclTests.tcl)
  endif()
  if(NOT IS_ABSOLUTE ${A_SCRIPT})
    set(A_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/${A_SCRIPT})
  endif()
  if(NOT A_TCLLIBPATH)
    set(A_TCLLIBPATH ${prefix}/TclLibs)
  endif()
  string(REPLACE ";" " " _tcllibpath "${A_TCLLIBPATH}")
  nscldaq_add_test(NAME ${A_NAME}
    COMMAND ${TCLSH_CMD} ${A_SCRIPT}
    ENVIRONMENT "TCLLIBPATH=${_tcllibpath}" "HERE=${CMAKE_CURRENT_SOURCE_DIR}"
      "tcl=${TCLSH_CMD}" ${A_ENVIRONMENT})
endfunction()

#---------------------------------------------------------------------------
#  nscldaq_gengetopt(<basename> <ggo-file>
#       [ARGS <extra gengetopt args...>]   e.g. --unamed-opts
#       [C_EXTENSION <ext>])               default c
#
#  Runs gengetopt on <ggo-file> (relative to the source dir) producing
#  <basename>.<ext> and <basename>.h in the current binary directory.  Sets
#  <basename>_GGO_SOURCES in the caller's scope.  Several targets may share
#  the output; attach it with nscldaq_use_gengetopt() so only one of them
#  runs the generator.

function(nscldaq_gengetopt base ggo)
  cmake_parse_arguments(A "" "C_EXTENSION" "ARGS" ${ARGN})
  if(NOT A_C_EXTENSION)
    set(A_C_EXTENSION c)
  endif()
  if(NOT IS_ABSOLUTE ${ggo})
    set(ggo ${CMAKE_CURRENT_SOURCE_DIR}/${ggo})
  endif()
  set(_c ${CMAKE_CURRENT_BINARY_DIR}/${base}.${A_C_EXTENSION})
  set(_h ${CMAKE_CURRENT_BINARY_DIR}/${base}.h)
  set(_cext "")
  if(NOT A_C_EXTENSION STREQUAL "c")
    set(_cext --c-extension=${A_C_EXTENSION})
  endif()
  add_custom_command(
    OUTPUT ${_c} ${_h}
    COMMAND ${GENGETOPT} --input=${ggo}
      --output-dir=${CMAKE_CURRENT_BINARY_DIR} --file-name=${base} ${_cext} ${A_ARGS}
    DEPENDS ${ggo}
    COMMENT "gengetopt ${base}"
    VERBATIM)
  _nscldaq_reldir(_rel)
  string(MAKE_C_IDENTIFIER "ggo_${_rel}_${base}" _gentgt)
  add_custom_target(${_gentgt} DEPENDS ${_c} ${_h})
  set_property(DIRECTORY PROPERTY _NSCLDAQ_GGO_TARGET_${base} ${_gentgt})
  set_property(DIRECTORY PROPERTY _NSCLDAQ_GGO_OUTPUTS_${base} ${_c} ${_h})
  set(${base}_GGO_SOURCES ${_c} ${_h} PARENT_SCOPE)
endfunction()

#  nscldaq_use_gengetopt(<target> <basename>...)
#  Adds the generated parser(s) to <target>'s sources and orders the target
#  after the generator (so targets sharing a parser do not race to make it).

function(nscldaq_use_gengetopt tgt)
  foreach(base IN LISTS ARGN)
    get_property(_gentgt DIRECTORY PROPERTY _NSCLDAQ_GGO_TARGET_${base})
    if(NOT _gentgt)
      message(FATAL_ERROR "nscldaq_use_gengetopt: no nscldaq_gengetopt(${base} ...) in this directory")
    endif()
    get_property(_outs DIRECTORY PROPERTY _NSCLDAQ_GGO_OUTPUTS_${base})
    target_sources(${tgt} PRIVATE ${_outs})
    add_dependencies(${tgt} ${_gentgt})
  endforeach()
endfunction()

#---------------------------------------------------------------------------
#  nscldaq_install_headers(<files...> [DESTINATION <dir>] [EXECUTABLE])
#
#  DESTINATION defaults to include.  EXECUTABLE installs them 0755, for
#  Makefiles that used $(INSTALL_SCRIPT) rather than include_HEADERS.
#
#  Installs headers and, when they go under include/, also links them into
#  ${NSCLDAQ_STAGE_INCLUDE_DIR} (NSCLDAQ::PrefixInclude) so that code which
#  used -I@prefix@/include finds them without a prior install.

function(nscldaq_install_headers)
  cmake_parse_arguments(A "EXECUTABLE" "DESTINATION" "" ${ARGN})
  if(NOT A_DESTINATION)
    set(A_DESTINATION include)
  endif()
  set(_files "")
  foreach(_f IN LISTS A_UNPARSED_ARGUMENTS)
    if(NOT IS_ABSOLUTE ${_f})
      if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${_f})
        set(_f ${CMAKE_CURRENT_SOURCE_DIR}/${_f})
      else()
        set(_f ${CMAKE_CURRENT_BINARY_DIR}/${_f})
      endif()
    endif()
    list(APPEND _files ${_f})
  endforeach()
  if(A_EXECUTABLE)
    install(PROGRAMS ${_files} DESTINATION ${A_DESTINATION})
  else()
    install(FILES ${_files} DESTINATION ${A_DESTINATION})
  endif()
  if(A_DESTINATION MATCHES "^include(/(.*))?$")
    set(_stage ${NSCLDAQ_STAGE_INCLUDE_DIR})
    if(CMAKE_MATCH_2)
      set(_stage ${_stage}/${CMAKE_MATCH_2})
    endif()
    file(MAKE_DIRECTORY ${_stage})
    foreach(_f IN LISTS _files)
      get_filename_component(_name ${_f} NAME)
      # Two directories installing different headers with the same name
      # would silently overwrite each other at install time - say so.
      get_property(_owner GLOBAL PROPERTY "_NSCLDAQ_STAGED_${_stage}/${_name}")
      set(_differs FALSE)
      if(_owner AND NOT _owner STREQUAL _f AND EXISTS "${_owner}" AND EXISTS "${_f}")
        file(SHA256 ${_owner} _h1)
        file(SHA256 ${_f} _h2)
        if(NOT _h1 STREQUAL _h2)
          set(_differs TRUE)
        endif()
      endif()
      if(_differs)
        message(WARNING "${_f} and ${_owner} are both installed as "
          "${A_DESTINATION}/${_name}")
      endif()
      set_property(GLOBAL PROPERTY "_NSCLDAQ_STAGED_${_stage}/${_name}" ${_f})
      file(CREATE_LINK ${_f} ${_stage}/${_name} SYMBOLIC)
    endforeach()
  endif()
endfunction()

#---------------------------------------------------------------------------
#  Install-time helpers.  Paths are relative to the install prefix.

set(_NSCLDAQ_DEST "\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}")

#  nscldaq_install_mkdir(<dirs...>)

function(nscldaq_install_mkdir)
  foreach(_d IN LISTS ARGN)
    install(CODE "file(MAKE_DIRECTORY \"${_NSCLDAQ_DEST}/${_d}\")")
  endforeach()
endfunction()

#  nscldaq_install_tcl_index(<dir> [<pkg_mkIndex options>...] <patterns...>)
#
#  echo "pkg_mkIndex <options> @prefix@/<dir> <patterns>" | $(TCLSH_CMD)
#  Arguments starting with '-' are options (e.g. -verbose), the rest are
#  patterns.  Like the make rule, a failure is reported but does not stop
#  the install.

function(nscldaq_install_tcl_index dir)
  set(_opts "")
  set(_pats "")
  foreach(_a IN LISTS ARGN)
    if(_a MATCHES "^-")
      string(APPEND _opts " ${_a}")
    else()
      string(APPEND _pats " ${_a}")
    endif()
  endforeach()
  set(_cmd "pkg_mkIndex${_opts} ${_NSCLDAQ_DEST}/${dir}${_pats}")
  install(CODE "
    message(STATUS \"${_cmd}\")
    file(MAKE_DIRECTORY \"${_NSCLDAQ_DEST}/${dir}\")
    execute_process(
      COMMAND \"${CMAKE_COMMAND}\" -E echo \"${_cmd}\"
      COMMAND \"${TCLSH_CMD}\"
      RESULT_VARIABLE _status)
    if(NOT _status EQUAL 0)
      message(WARNING \"pkg_mkIndex in ${dir} failed: \${_status}\")
    endif()
  ")
endfunction()

#  nscldaq_install_copy(<from> <to>)
#  Copy a file that is already installed (e.g. lib/libFoo.so) to another place
#  in the prefix (e.g. pythonLibs/nscldaq/Foo.so).  Symlinks are followed.

function(nscldaq_install_copy from to)
  install(CODE "
    get_filename_component(_dir \"${_NSCLDAQ_DEST}/${to}\" DIRECTORY)
    file(MAKE_DIRECTORY \"\${_dir}\")
    message(STATUS \"Installing: ${_NSCLDAQ_DEST}/${to}\")
    execute_process(COMMAND \"${CMAKE_COMMAND}\" -E copy
      \"${_NSCLDAQ_DEST}/${from}\" \"${_NSCLDAQ_DEST}/${to}\"
      RESULT_VARIABLE _status)
    if(NOT _status EQUAL 0)
      message(FATAL_ERROR \"Could not copy ${from} to ${to}\")
    endif()
  ")
endfunction()

#  nscldaq_install_code(<shell command string> [NOFAIL])
#  Run a /bin/sh command at install time; the string may use
#  $DESTDIR_PREFIX for "$DESTDIR$prefix" and $SRCDIR/$BUILDDIR for the
#  current source/binary directories.  NOFAIL mirrors a '-' prefixed make
#  recipe line.  Prefer the specific helpers or install() when they fit.

function(nscldaq_install_code cmd)
  cmake_parse_arguments(A "NOFAIL" "" "" ${ARGN})
  set(_fail "message(FATAL_ERROR \"install command failed: \${_status}\")")
  if(A_NOFAIL)
    set(_fail "message(STATUS \"(ignored) install command failed: \${_status}\")")
  endif()
  string(REPLACE "\\" "\\\\" cmd "${cmd}")
  string(REPLACE "\"" "\\\"" cmd "${cmd}")
  string(REPLACE "\$" "\\\$" cmd "${cmd}")
  install(CODE "
    set(ENV{DESTDIR_PREFIX} \"${_NSCLDAQ_DEST}\")
    set(ENV{SRCDIR} \"${CMAKE_CURRENT_SOURCE_DIR}\")
    set(ENV{BUILDDIR} \"${CMAKE_CURRENT_BINARY_DIR}\")
    execute_process(COMMAND /bin/sh -c \"${cmd}\"
      WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}\"
      RESULT_VARIABLE _status)
    if(NOT _status EQUAL 0)
      ${_fail}
    endif()
  ")
endfunction()

#---------------------------------------------------------------------------
#  @CC@/@CXX@ as autoconf recorded them: the name AC_PROG_CC/AC_PROG_CXX
#  would have found on PATH (gcc, g++) when that is the compiler in use,
#  otherwise the full compiler path (as for an explicit CXX=/path/mpicxx).
#  ($CC/$CXX can't be consulted here: CMake itself sets them in its own
#  environment while detecting compilers on the first configure.)

function(_nscldaq_ac_compiler out compiler)
  set(_names ${ARGN})
  get_filename_component(_real "${compiler}" REALPATH)
  foreach(_n IN LISTS _names)
    find_program(_nscldaq_ac_${_n} ${_n})
    if(_nscldaq_ac_${_n})
      get_filename_component(_nreal "${_nscldaq_ac_${_n}}" REALPATH)
      if(_nreal STREQUAL _real)
        set(${out} "${_n}" PARENT_SCOPE)
        return()
      endif()
    endif()
  endforeach()
  set(${out} "${compiler}" PARENT_SCOPE)
endfunction()
_nscldaq_ac_compiler(NSCLDAQ_AC_CC "${CMAKE_C_COMPILER}" gcc cc)
_nscldaq_ac_compiler(NSCLDAQ_AC_CXX "${CMAKE_CXX_COMPILER}" g++ c++)

#---------------------------------------------------------------------------
#  nscldaq_configure_file(<input> <output> [EXECUTABLE])
#
#  AC_CONFIG_FILES equivalent: substitutes @VAR@ (prefix, TCLSH_CMD, ...,
#  and srcdir/builddir/top_srcdir/top_builddir for this directory).  Output
#  is relative to the current binary dir.  EXECUTABLE makes it +x (useful
#  for scripts run from the build tree).

function(nscldaq_configure_file in out)
  cmake_parse_arguments(A "EXECUTABLE" "" "" ${ARGN})
  set(srcdir "${CMAKE_CURRENT_SOURCE_DIR}")
  set(abs_srcdir "${CMAKE_CURRENT_SOURCE_DIR}")
  set(builddir "${CMAKE_CURRENT_BINARY_DIR}")
  set(abs_builddir "${CMAKE_CURRENT_BINARY_DIR}")
  set(top_srcdir "${PROJECT_SOURCE_DIR}")
  set(abs_top_srcdir "${PROJECT_SOURCE_DIR}")
  set(top_builddir "${PROJECT_BINARY_DIR}")
  set(abs_top_builddir "${PROJECT_BINARY_DIR}")
  set(CC "${NSCLDAQ_AC_CC}")
  set(CXX "${NSCLDAQ_AC_CXX}")
  if(NOT IS_ABSOLUTE ${out})
    set(out ${CMAKE_CURRENT_BINARY_DIR}/${out})
  endif()
  if(A_EXECUTABLE)
    get_filename_component(_name ${out} NAME)
    get_filename_component(_dir ${out} DIRECTORY)
    set(_tmp ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/configured/${_name})
    configure_file(${in} ${_tmp} @ONLY)
    file(COPY ${_tmp} DESTINATION ${_dir}
      FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
        GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
  else()
    configure_file(${in} ${out} @ONLY)
  endif()
endfunction()

#---------------------------------------------------------------------------
#  Summary printed at the end of configuration.

function(nscldaq_print_summary)
  message(STATUS "")
  message(STATUS "NSCLDAQ ${NSCLDAQ_VERSION_STRING} configuration:")
  message(STATUS "  Install prefix ............ ${CMAKE_INSTALL_PREFIX}")
  message(STATUS "  C++ compiler .............. ${CMAKE_CXX_COMPILER}")
  message(STATUS "  Tcl ....................... ${TCL_VERSION} (${TCLSH_CMD})")
  message(STATUS "  ROOT ...................... ${ROOTSYS}")
  message(STATUS "  MPI ....................... ${NSCLDAQ_HAVE_MPI}")
  message(STATUS "  Boost::log ................ ${HAVE_BOOST_LOG}")
  foreach(_o ENABLE_USB ENABLE_DDAS ENABLE_DDAS_DOCS ENABLE_DOCS ENABLE_EPICS_TOOLS
      ENABLE_CAEN_DIGITIZER_SUPPORT ENABLE_CAEN_NEXTGEN ENABLE_MVLC_STACK_GENERATOR
      ENABLE_SBS ENABLE_RUST_TOOLS BUILD_TESTING)
    message(STATUS "  ${_o} = ${${_o}}")
  endforeach()
  message(STATUS "")
endfunction()

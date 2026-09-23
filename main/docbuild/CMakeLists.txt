#
#  DocBook documentation: the NSCLDAQ reference (daq.xml -> html + man
#  pages) and the tutorial (pdf + html).
#
#  In the Makefile.am these were prerequisites of install-exec-local, so they
#  were built by 'make install'.  Here they are built with 'all' in the binary
#  directory and installed by 'cmake --install'.
#
#  images/ (and images/callouts) are only DIST_SUBDIRS in the Makefile.am -
#  their gifs are installed by this directory, so they are not added.
#

set(_mansects 1compatibility.xml 1daq.xml 1epics.xml 1evb.xml 1tcl.xml 1utilities.xml
  3ccusb.xml 3daq.xml 3evb.xml 3provider.xml 3python.xml 3rdogui.xml 3sbsReadout.xml
  3tcl.xml 3vmusb.xml 3mvlcgen.xml
  5daq.xml 5tcl.xml 5vmusb.xml)

# Directories whose *.xml fragments docbuilder.tcl assembles into daq.xml
# (BOOKDIRS).  @top_builddir@/libtcl is where configure incorporated libtcl++;
# here it is the libtcl++ source directory used by libtcl_incorp.

set(_bookdirs
  ${PROJECT_SOURCE_DIR}
  ${PROJECT_SOURCE_DIR}/base/dataflow
  ${PROJECT_SOURCE_DIR}/daq/format
  ${PROJECT_SOURCE_DIR}/utilities/manager
  ${PROJECT_SOURCE_DIR}/utilities/readoutREST
  ${PROJECT_SOURCE_DIR}/utilities/daqstart
  ${PROJECT_SOURCE_DIR}/usb
  ${PROJECT_SOURCE_DIR}/usb/usb1
  ${PROJECT_SOURCE_DIR}/usb/vmusb
  ${PROJECT_SOURCE_DIR}/usb/ccusb
  ${PROJECT_SOURCE_DIR}/usb/vmusb/SIS3316Readout
  ${PROJECT_SOURCE_DIR}/daq/readoutgui
  ${PROJECT_SOURCE_DIR}/utilities/logbook
  ${PROJECT_SOURCE_DIR}/daq/eventbuilder
  ${PROJECT_SOURCE_DIR}/daq/evbtools/glom
  ${PROJECT_SOURCE_DIR}/daq/evbtools/evblite
  ${PROJECT_SOURCE_DIR}/daq/evbtools/ringsource
  ${PROJECT_SOURCE_DIR}/daq/evbtools/unglom
  ${PROJECT_SOURCE_DIR}/utilities/newscaler
  ${PROJECT_SOURCE_DIR}/utilities/pyscaler
  ${PROJECT_SOURCE_DIR}/simplesetups/vmusb
  ${PROJECT_SOURCE_DIR}/simplesetups/v775
  ${PROJECT_SOURCE_DIR}/simplesetups/v785
  ${PROJECT_SOURCE_DIR}/utilities/bufdump
  ${PROJECT_SOURCE_DIR}/utilities/pybufdump
  ${PROJECT_SOURCE_DIR}/utilities/eventlog
  ${PROJECT_SOURCE_DIR}/utilities/scalersum
  ${PROJECT_SOURCE_DIR}/servers/portmanager
  ${PROJECT_SOURCE_DIR}/servers/tclserver
  ${PROJECT_SOURCE_DIR}/usb/mesytec/MCFD16
  ${PROJECT_SOURCE_DIR}/usb/mesytec/MSCF16
  ${PROJECT_SOURCE_DIR}/usb/vmusb/controlpanels
  ${PROJECT_SOURCE_DIR}/usb/ccusb/controlpanels
  ${PROJECT_SOURCE_DIR}/usb/ccusb/tclpackage
  ${PROJECT_SOURCE_DIR}/usb/common/tcldrivers
  ${PROJECT_SOURCE_DIR}/sbs
  ${PROJECT_SOURCE_DIR}/sbs/readout
  ${PROJECT_SOURCE_DIR}/sbs/vmemodules
  ${PROJECT_SOURCE_DIR}/sbs/slowcontrols
  ${PROJECT_SOURCE_DIR}/sbs/slowcontrols/caenv812
  ${PROJECT_SOURCE_DIR}/sbs/slowcontrols/bcnaf
  ${PROJECT_SOURCE_DIR}/sbs/slowcontrols/n568b
  ${PROJECT_SOURCE_DIR}/sbs/slowcontrols/vhs
  ${PROJECT_SOURCE_DIR}/sbs/slowcontrols/vhq
  ${PROJECT_SOURCE_DIR}/sbs/puretcl
  ${PROJECT_SOURCE_DIR}/sbs/tclpackage
  ${PROJECT_SOURCE_DIR}/usb/loaders
  ${PROJECT_SOURCE_DIR}/usb/caenfwloader
  ${PROJECT_SOURCE_DIR}/utilities/filter
  ${PROJECT_SOURCE_DIR}/daq/actions
  ${PROJECT_SOURCE_DIR}/utilities/sequencer
  ${PROJECT_SOURCE_DIR}/utilities/ringselector
  ${PROJECT_SOURCE_DIR}/utilities/compatibility
  ${PROJECT_SOURCE_DIR}/utilities/tkbufdump
  ${PROJECT_SOURCE_DIR}/utilities/reglom
  ${PROJECT_SOURCE_DIR}/epics/controlpush
  ${PROJECT_SOURCE_DIR}/epics/epicsdisplay
  ${PROJECT_SOURCE_DIR}/epics/chanlog
  ${PROJECT_SOURCE_DIR}/base/cvt
  ${PROJECT_SOURCE_DIR}/base/thread
  ${PROJECT_SOURCE_DIR}/base/security
  ${PROJECT_SOURCE_DIR}/base/plotcontainer
  ${PROJECT_SOURCE_DIR}/base/uri
  ${PROJECT_SOURCE_DIR}/base/CopyrightTools
  ${PROJECT_SOURCE_DIR}/base/os
  ${PROJECT_SOURCE_DIR}/base/plotchart
  ${PROJECT_SOURCE_DIR}/base/tclutils
  ${PROJECT_SOURCE_DIR}/base/tclwidgets
  ${PROJECT_SOURCE_DIR}/base/tcpip
  ${NSCLDAQ_LIBTCL_SOURCE_DIR}/tclplus
  ${NSCLDAQ_LIBTCL_SOURCE_DIR}/exception
  ${PROJECT_SOURCE_DIR}/examples
  ${PROJECT_SOURCE_DIR}/ddas/readout
  ${PROJECT_SOURCE_DIR}/ddas/pixieplugin
  ${PROJECT_SOURCE_DIR}/caen-nextgen
  ${PROJECT_SOURCE_DIR}/utilities/ringdiagnostics
  ${PROJECT_SOURCE_DIR}/usb/vmusb/sis3316examples
  ${PROJECT_SOURCE_DIR}/mvlc
  ${PROJECT_SOURCE_DIR}/rust_tools/ring_monitor
  ${PROJECT_SOURCE_DIR}/utilities/statsview
  ${PROJECT_SOURCE_DIR}/utilities/swtrigger)

# The Makefile rule for daq.xml had no prerequisites (built once); depend
# on the fragments so edits are picked up.

set(_bookxml "")
foreach(_d IN LISTS _bookdirs)
  string(FIND "${_d}" "${NSCLDAQ_LIBTCL_SOURCE_DIR}/" _inlibtcl)
  if(NOT _inlibtcl EQUAL 0)      # libtcl++ may only exist at build time.
    file(GLOB _x CONFIGURE_DEPENDS ${_d}/*.xml)
    list(APPEND _bookxml ${_x})
  endif()
endforeach()
file(GLOB _metaxml CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/docconfig/*.xml)

set(_bin ${CMAKE_CURRENT_BINARY_DIR})
set(_sectxml "")
foreach(_s IN LISTS _mansects ITEMS 1usbReadout.xml 1sbsReadout.xml)
  list(APPEND _sectxml ${_bin}/${_s})
endforeach()

# daq.xml: docbuilder.tcl writes the book to stdout and the reference
# sections (the MANSECTS files) into the current directory.

add_custom_command(
  OUTPUT ${_bin}/daq.xml
  BYPRODUCTS ${_sectxml}
  COMMAND sh -c "\"$0\" \"$@\" > daq.xml.tmp && mv daq.xml.tmp daq.xml"
    ${TCLSH_CMD} ${CMAKE_CURRENT_SOURCE_DIR}/docbuilder.tcl
    ${PROJECT_SOURCE_DIR}/docconfig "NSCL DAQ Software Documentation"
    ${_bookdirs}
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/docbuilder.tcl
    ${PROJECT_SOURCE_DIR}/docconfig/config ${_metaxml} ${_bookxml}
    libtcl_incorp
  WORKING_DIRECTORY ${_bin}
  COMMENT "docbuilder: daq.xml"
  VERBATIM)
add_custom_target(docbuild_daq_xml DEPENDS ${_bin}/daq.xml)

# daq.html: the reference as html in html/.

add_custom_command(
  OUTPUT ${_bin}/daq.html
  COMMAND ${CMAKE_COMMAND} -E remove_directory html
  COMMAND ${CMAKE_COMMAND} -E make_directory html
  COMMAND ${DOCBOOK2HTML} -o ${_bin}/html daq.xml
  COMMAND ${CMAKE_COMMAND} -E touch daq.html
  DEPENDS ${_bin}/daq.xml
  WORKING_DIRECTORY ${_bin}
  COMMENT "docbook2html: daq.xml"
  VERBATIM)

# daq.mans: man pages in man/ from the reference sections.  Errors were
# ignored ('-').

list(JOIN _mansects " " _mansects_str)
string(REPLACE ";" " " _toman_str "${DOCBOOK2MAN}")
add_custom_command(
  OUTPUT ${_bin}/daq.mans
  COMMAND sh -c "for f in ${_mansects_str}; do ${_toman_str} -o man --skip-validation $f; done; exit 0"
  COMMAND ${CMAKE_COMMAND} -E touch daq.mans
  DEPENDS ${_bin}/daq.xml
  WORKING_DIRECTORY ${_bin}
  COMMENT "xmlto man: reference sections"
  VERBATIM)

# tutorial.pdf and tutorial.html (in tuthtml/).

add_custom_command(
  OUTPUT ${_bin}/tutorial.pdf
  COMMAND ${DOCBOOK2PDF} ${CMAKE_CURRENT_SOURCE_DIR}/tutorial.xml
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/tutorial.xml
  WORKING_DIRECTORY ${_bin}
  COMMENT "docbook2pdf: tutorial.xml"
  VERBATIM)

add_custom_command(
  OUTPUT ${_bin}/tutorial.html
  COMMAND ${CMAKE_COMMAND} -E remove_directory tuthtml
  COMMAND ${CMAKE_COMMAND} -E make_directory ${_bin}/tuthtml
  COMMAND ${DOCBOOK2HTML} -o ${_bin}/tuthtml ${CMAKE_CURRENT_SOURCE_DIR}/tutorial.xml
  COMMAND ${CMAKE_COMMAND} -E touch tutorial.html
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/tutorial.xml
  WORKING_DIRECTORY ${_bin}
  COMMENT "docbook2html: tutorial.xml"
  VERBATIM)

add_custom_target(docbuild_docs ALL
  DEPENDS ${_bin}/daq.html ${_bin}/daq.mans ${_bin}/tutorial.pdf ${_bin}/tutorial.html)
add_dependencies(docbuild_docs docbuild_daq_xml)

# daq.pdf was a make target that nothing depended on; kept as an optional
# target.

add_custom_command(
  OUTPUT ${_bin}/daq.pdf
  COMMAND ${DOCBOOK2PDF} daq.xml
  DEPENDS ${_bin}/daq.xml
  WORKING_DIRECTORY ${_bin}
  COMMENT "docbook2pdf: daq.xml"
  VERBATIM)
add_custom_target(docbuild_daq_pdf DEPENDS ${_bin}/daq.pdf)
add_dependencies(docbuild_daq_pdf docbuild_daq_xml)

#---------------------------------------------------------------------------
#  Installation (was install-exec-local).

nscldaq_install_mkdir(share share/man share/html share/tuthtml share/pdfs)
foreach(_i 1 2 3 4 5 6 7 8)
  nscldaq_install_mkdir(share/man/man${_i})
endforeach()

# '-' for i in 1..8; for f in man/*.$i*; install $f share/man/man$i
# Done in the shell: some page names contain ';' (e.g. "wait.3tc;") which a
# CMake list cannot hold.

nscldaq_install_code("for i in 1 2 3 4 5 6 7 8; do for f in man/*.$i*; do install -c -m 644 \"$f\" \"$DESTDIR_PREFIX/share/man/man$i\"; done; done" NOFAIL)

install(CODE "
  message(STATUS \"${MANDB} \$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/share/man\")
  execute_process(COMMAND \"${MANDB}\" \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/share/man\"
    RESULT_VARIABLE _status)
  if(NOT _status EQUAL 0)
    message(FATAL_ERROR \"mandb failed: \${_status}\")
  endif()
")

# '-' install html/* share/html  (html/ has no subdirectories)
install(DIRECTORY ${_bin}/html/ DESTINATION share/html OPTIONAL)

nscldaq_install_mkdir(share/images share/images/callouts)
file(GLOB _gifs CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/images/*.gif)
install(FILES ${_gifs} DESTINATION share/images)
file(GLOB _callout_gifs CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/images/callouts/*.gif)
install(FILES ${_callout_gifs} DESTINATION share/images/callouts)

install(FILES ${_bin}/tutorial.pdf DESTINATION share/pdfs OPTIONAL)
install(DIRECTORY ${_bin}/tuthtml/ DESTINATION share/tuthtml OPTIONAL)

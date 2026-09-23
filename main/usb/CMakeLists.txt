#
#  usb: VM-USB/CC-USB support (only added when ENABLE_USB).
#  usb1 and usb0.1 are DIST_SUBDIRS only and are not built.
#

add_subdirectory(common)
add_subdirectory(vmusb)
add_subdirectory(ccusb)
add_subdirectory(loaders)
add_subdirectory(caenfwloader)
add_subdirectory(mesytec)

#  install-exec-local: index the swig wrappers for the XX_USB packages by
#  hand (pkg_mkIndex fails if no controllers are online).  This rewrites
#  lib/pkgIndex.tcl from scratch after the subdirectories appended to it.

install(CODE [=[
  set(_idx "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/pkgIndex.tcl")
  message(STATUS "Writing: ${_idx}")
  file(REMOVE "${_idx}")
  file(APPEND "${_idx}" "package ifneeded cccusb 1.0 [list load [file join $dir libCCCUSB.so]]\n")
  file(APPEND "${_idx}" "package ifneeded cccusbreadoutlist 1.0 [list load [file join $dir libCCCUSBReadoutList.so]]\n")
  file(APPEND "${_idx}" "package ifneeded cvmusb 1.0 [list load [file join $dir libCVMUSB.so]]\n")
  file(APPEND "${_idx}" "package ifneeded cvmusbreadoutlist 1.0 [list load [file join $dir libCVMUSBReadoutList.so]]\n")
]=])

#
#  usb/common.  configurableobject is also in SUBDIRS of the Makefile.am
#  but is added from the top level CMakeLists.txt (it is needed even
#  without USB) so it is not added again here.
#

add_subdirectory(tcldrivers)
add_subdirectory(devices)
add_subdirectory(slowcontrols)

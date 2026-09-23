nscldaq_add_library(caenfwloadCommon
  SOURCES CAENComm_Common.cpp CFASegment.c cvupgrade.c flash.c)

#  install-exec-local: firmware loader parameter files -> etc/caenfw.
#  (@top_srcdir@/caenfw-common/*.txt - not this CMakeLists.txt, of course.)

file(GLOB _caenfw_txt CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.txt)
list(FILTER _caenfw_txt EXCLUDE REGEX "/CMakeLists\\.txt$")
install(FILES ${_caenfw_txt} DESTINATION etc/caenfw)

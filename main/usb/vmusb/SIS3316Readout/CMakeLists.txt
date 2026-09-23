#
#  usb/vmusb/SIS3316Readout: VMUSBnostackReadout, an SBS-readout-framework
#  based VM-USB readout for the SIS3316.
#

nscldaq_add_executable(VMUSBnostackReadout
  SOURCES CConfigurableCompoundEventSegment.cpp CSIS3316EventSegment.cpp
    CSIS3316Trigger.cpp CSIS3820ScalerBank.cpp CSIS3820Scaler.cpp
    CSIS3820TimestampEventSegment.cpp CV977EventSegment.cpp CVMUSBBusy.cpp
    CVMUSBEventSegment.cpp sis3316_class.cpp Skeleton.cpp)
target_include_directories(VMUSBnostackReadout PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/../vmusb
  ${CMAKE_CURRENT_SOURCE_DIR}/..
  ${PROJECT_SOURCE_DIR}/usb/common/configurableobject
  ${PROJECT_SOURCE_DIR}/base/thread
  ${PROJECT_SOURCE_DIR}/base/os
  ${PROJECT_SOURCE_DIR}/sbs/readout
  ${PROJECT_BINARY_DIR}/sbs/readout)
# CReadoutMain.h includes the gengetopt generated options.h of sbs/readout
# (the autotools build found it in that source directory).
add_dependencies(VMUSBnostackReadout ggo_sbs_readout_options)
target_link_libraries(VMUSBnostackReadout PRIVATE
  NSCLDAQ::TclOnlyHeaders SBSProductionReadout ConfigurableObject VMUSB
  NSCLDAQ::tclPlus NSCLDAQ::Tcl)

nscldaq_install_headers(CConfigurableCompoundEventSegment.h
  DESTINATION include/vmusbnostack)

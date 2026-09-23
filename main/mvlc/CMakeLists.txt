#
#  MVLC configuration generator (only added when ENABLE_MVLC_STACK_GENERATOR).
#
#  Device support modules are imported from usb/vmusb/daqconfig: they are
#  copied into the build directory (as the Makefile did) so that their
#  quoted #includes resolve against this directory's headers first, and are
#  compiled with MVLC_GENERATOR defined.
#

set(_mvlc_import_dir ${PROJECT_SOURCE_DIR}/usb/vmusb/daqconfig)
set(_mvlc_imported_names
  CMQDC32RdoHdwr.cpp CMQDC32StackBuilder.cpp C785.cpp C830.cpp C3804.cpp
  C3820.cpp CCAENChain.cpp CDelay.cpp CMesytecBase.cpp CMADC32.cpp
  CMADCChain.cpp CMADCScaler.cpp CMarker.cpp CMDPP16QDC.cpp CMDPP32QDC.cpp
  CMDPP32SCP.cpp CMTDC32.cpp CNADC2530.cpp CV1x90.cpp CV977.cpp CV1495sc.cpp
  CV1729.cpp CXLM.cpp CXLMTimestamp.cpp CXLMFERA.cpp s800TriggerRegisters.cpp
  CS800TriggerNew.cpp CS800TriggerScalers.cpp)
set(_mvlc_imported_modules "")
foreach(_f IN LISTS _mvlc_imported_names)
  configure_file(${_mvlc_import_dir}/${_f} ${CMAKE_CURRENT_BINARY_DIR}/${_f} COPYONLY)
  list(APPEND _mvlc_imported_modules ${CMAKE_CURRENT_BINARY_DIR}/${_f})
endforeach()

#------------------ libraries

nscldaq_add_library(mvlcSlowControlsClient SOURCES CVMEClient.cpp TclVMEClient.cpp)
target_link_libraries(mvlcSlowControlsClient PUBLIC NSCLDAQ::tclPlus NSCLDAQ::Tcl)

set(_mvlc_generator_sources CReadoutHardware.cpp CReadoutModule.cpp
  DeviceCommand.cpp TCLConfigParser.cpp CVMUSBReadoutList.cpp CVMUSB.cpp
  CStack.cpp)

nscldaq_add_library(mvlcGenerator SOURCES ${_mvlc_generator_sources})
target_include_directories(mvlcGenerator PRIVATE
  ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/usb/common/configurableobject)
target_link_libraries(mvlcGenerator PRIVATE
  NSCLDAQ::TclOnlyHeaders NSCLDAQ::tclPlusHeaders)

# includedir=@prefix@/include/mvlc

nscldaq_install_headers(TCLConfigParser.h CVMUSBReadoutList.h CVMUSB.h
  MVLCConfigParser.h DeviceCommand.h CReadoutModule.h CReadoutHardware.h
  CVMEClient.h DESTINATION include/mvlc)

# Example driver (noinst; SampleDriver.cpp includes <mvlc/...>).

nscldaq_add_library(SampleDriver NOINST SOURCES SampleDriver.cpp)
target_include_directories(SampleDriver PRIVATE
  ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/usb/common/configurableobject)
target_link_libraries(SampleDriver
  PRIVATE NSCLDAQ::TclOnlyHeaders NSCLDAQ::tclPlusHeaders
  PUBLIC mvlcGenerator)

#------------------ mvlcgenerate

nscldaq_gengetopt(options options.ggo)
nscldaq_gengetopt(xlm_options xlmload_options.ggo)

set(_mvlc_local_modules VMUSBCommand.cpp VMUSBListCommand.cpp TclWrapper.cpp)
set(_mvlc_main_modules utilities.cpp MVLCConfigParser.cpp
  ${_mvlc_local_modules} MVLCGenerator.cpp sis_vmusb_interface.cpp)

set(_mvlcgenerate_includes
  ${PROJECT_SOURCE_DIR}/usb/common/configurableobject
  ${PROJECT_SOURCE_DIR}/base/headers
  ${PROJECT_SOURCE_DIR}/base/os
  ${PROJECT_SOURCE_DIR}/usb/vmusb/daqconfig)
set(_mvlcgenerate_defs
  MVLC_TEMPLATE="${datarootdir}/mvlc/triggeronly.yaml"
  MVLC_TCLLIBDIR="${datarootdir}/mvlc/TclLibs"
  MVLC_GENERATOR
  DEFAULT_S800REGFILE="${datarootdir}/s800triggers/RegisterFile.json")

nscldaq_add_executable(mvlcgenerate
  SOURCES generate.cpp ${_mvlc_main_modules} ${_mvlc_imported_modules})
nscldaq_use_gengetopt(mvlcgenerate options)
target_include_directories(mvlcgenerate PRIVATE ${_mvlcgenerate_includes})
target_compile_definitions(mvlcgenerate PRIVATE ${_mvlcgenerate_defs})
target_link_libraries(mvlcgenerate PRIVATE
  ConfigurableObject NSCLDAQ::tclPlus NSCLDAQ::Tcl NSCLDAQ::YamlCpp
  mvlcGenerator NSCLDAQ::JsonCpp)

#------------------ mvlc_xlmload (compiles the generator sources itself)

nscldaq_add_executable(mvlc_xlmload
  SOURCES xlmload.cpp CMVLCDirect.cpp ${CMAKE_CURRENT_BINARY_DIR}/CXLM.cpp
    ${_mvlc_generator_sources})
nscldaq_use_gengetopt(mvlc_xlmload xlm_options)
add_dependencies(mvlc_xlmload mvlcGenerator)
target_include_directories(mvlc_xlmload PRIVATE
  ${PROJECT_SOURCE_DIR}/usb/common/configurableobject
  ${PROJECT_SOURCE_DIR}/base/headers
  ${PROJECT_SOURCE_DIR}/base/os
  ${_mvlc_import_dir})
target_compile_definitions(mvlc_xlmload PRIVATE MVLC_GENERATOR)
target_link_libraries(mvlc_xlmload PRIVATE
  NSCLDAQ::MVLC ConfigurableObject NSCLDAQ::tclPlus NSCLDAQ::Tcl)

#------------------ mvlc_sets800run

nscldaq_add_executable(mvlc_sets800run
  SOURCES sets800run.cpp CVMUSBReadoutList.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/s800TriggerRegisters.cpp)
target_include_directories(mvlc_sets800run PRIVATE
  ${PROJECT_SOURCE_DIR}/usb/vmusb/daqconfig)
target_compile_definitions(mvlc_sets800run PRIVATE
  MVLC_GENERATOR
  DEFAULT_S800REGFILE="${datarootdir}/s800triggers/RegisterFile.json")
target_link_libraries(mvlc_sets800run PRIVATE
  NSCLDAQ::MVLCHeaders NSCLDAQ::JsonCpp mvlcSlowControlsClient)

#------------------ tests

nscldaq_add_executable(mvlc_unittests TEST OUTPUT_NAME unittests
  SOURCES TestRunner.cpp
    utiltests.cpp parsertests.cpp stacktests.cpp devcommandtests.cpp
    rdomoduletests.cpp stackmoduletests.cpp markertests.cpp v785tests.cpp
    3820tests.cpp delaytests.cpp madc32tests.cpp caenchaintests.cpp
    madcchaintests.cpp madcscalertests.cpp mtdctests.cpp mqdctests.cpp
    mdpptests.cpp sis3804tests.cpp v830tests.cpp hytectests.cpp v977tests.cpp
    v1495sctests.cpp v1729tests.cpp v1x90tests.cpp sisvmusbTests.cpp
    ${_mvlc_imported_modules} ${_mvlc_local_modules}
    ${_mvlc_generator_sources} sis_vmusb_interface.cpp MVLCConfigParser.cpp
    utilities.cpp)
nscldaq_use_gengetopt(mvlc_unittests options)
target_include_directories(mvlc_unittests PRIVATE ${_mvlcgenerate_includes})
target_compile_definitions(mvlc_unittests PRIVATE ${_mvlcgenerate_defs})
target_link_libraries(mvlc_unittests PRIVATE
  ConfigurableObject NSCLDAQ::tclPlus NSCLDAQ::Tcl NSCLDAQ::YamlCpp
  NSCLDAQ::CppUnit NSCLDAQ::JsonCpp)
nscldaq_add_test(NAME unittests COMMAND mvlc_unittests)

#------------------ install-exec-local

install(FILES triggeronly.yaml DESTINATION share/mvlc)
install(FILES SampleDriver.cpp DESTINATION share/mvlc-examples)
install(PROGRAMS SampleDriver.tcl DESTINATION share/mvlc-examples)
install(PROGRAMS VMUSBDriverSupport.tcl Utils.tcl
  sis3300defs.tcl sis330xdriver.tcl sis3300.tcl
  ACAENV1290.tcl ACAENV785.tcl AXLM72.tcl ACrdcXLM72.tcl
  AGD16XLM72.tcl ALevel3XLM72Readout.tcl AMesytecMADC32.tcl
  APpacXLM72.tcl ATimeStampXLM72.tcl AXLM72Generic.tcl
  AXLM72Scaler.tcl
  DESTINATION share/mvlc/TclLibs)
nscldaq_install_tcl_index(share/mvlc/TclLibs -verbose "*.tcl")

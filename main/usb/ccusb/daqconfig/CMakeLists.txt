nscldaq_add_library(CCUSBDaqConfig
  SOURCES CConfiguration.cpp CStackCommand.cpp CPH7xxCommand.cpp
    CAD811Command.cpp CLRS2551Command.cpp CLRS2249Command.cpp
    CLRS2228Command.cpp CCAEN257Command.cpp C1205Command.cpp
    CCCUSBCommand.cpp CPH7132Command.cpp CUserCommand.cpp CAddTclDriver.cpp
    CMarkerCommand.cpp CReadoutModule.cpp CReadoutHardware.cpp CPH7xx.cpp
    CAD811.cpp CStack.cpp CLRS2551.cpp CLRS2249.cpp CLRS2228.cpp
    CCAEN257.cpp CC1205.cpp CCCUSBModule.cpp CPH7132.cpp CTclModule.cpp
    CLeCroy4434.cpp CCamacCrate.cpp CLeCroy4300B.cpp CULMTrigger.cpp
    CMarker.cpp)
target_include_directories(CCUSBDaqConfig PRIVATE
  ${PROJECT_SOURCE_DIR}/usb/ccusb
  ${PROJECT_SOURCE_DIR}/usb/ccusb/ccusb
  ${PROJECT_SOURCE_DIR}/usb/ccusb/daqconfig
  ${PROJECT_SOURCE_DIR}/usb/common/devices
  ${PROJECT_SOURCE_DIR}/usb/common/configurableobject
  ${PROJECT_SOURCE_DIR}/base/headers)
target_link_libraries(CCUSBDaqConfig
  PRIVATE NSCLDAQ::PixieHeaders
  PUBLIC CCUSB ConfigurableObject NSCLDAQ::tclPlus NSCLDAQ::Tcl NSCLDAQ::Threads)

# install-data-local

file(GLOB _ccusb_daqconfig_headers CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
install(FILES ${_ccusb_daqconfig_headers} DESTINATION ccusbdriver/includes)

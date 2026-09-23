nscldaq_add_library(SBSDeviceSupport VERSION_INFO 1:0:0
  SOURCES CADC2530.cpp CAENcard_767.cpp CAENcard.cpp CaenIO.cpp camacmap.cpp
    CCAENChain.cpp CCAENV1x90.cpp CCAENV560.cpp CCAENV830.cpp CCAENV977.cpp
    CModule32.cpp CVMEScaler.cpp CSIS3600.cpp CSIS3820.cpp
    CVMEScalerLRS1151.cpp Nimout.cpp sis3300.cpp v890.cpp
    CMemoryMappedCAMACBranch.cpp CESCBD8210.cpp CVC32CC32.cpp
    CCAMACModule.cpp)
target_include_directories(SBSDeviceSupport PRIVATE
  ${PROJECT_SOURCE_DIR}/base/headers
  ${PROJECT_SOURCE_DIR}/sbs/nsclapi
  ${PROJECT_SOURCE_DIR}/sbs/driver/include
  ${PROJECT_SOURCE_DIR}/base/os)
target_compile_definitions(SBSDeviceSupport PRIVATE
  HAVE_VME_MAPPING HAVE_SBSVME_INTERFACE)
target_link_libraries(SBSDeviceSupport
  PRIVATE NSCLDAQ::PixieHeaders
  PUBLIC NSCLDAQ::tclPlus daqshm SBSVmeAPI btp)

# includedir=@prefix@/include/sbsmodules (avoids collisions with the VMUSB
# daqconfig headers of the same name).

nscldaq_install_headers(CADC2530.h CAENcard_767.h CAENcard.h CaenIO.h camac.h
  camacmap.h CCAENChain.h CCAENV1x90Data.h CCAENV1x90.h CCAENV1x90Opcodes.h
  CCAENV1x90Registers.h CCAENV560.h CCAENV560Registers.h CCAENV820Registers.h
  CCAENV830.h CCAENV977.h cescamac.h cesmacros.h CModule32.h CVMEScaler.h
  CSIS3600.h CSIS3820.h CVMEScalerLRS1151.h macros.h Nimout.h sis3300.h
  v890.h wienercamac.h wienermacros.h CCAMACBranch.h
  CMemoryMappedCAMACBranch.h CESCBD8210.h CVC32CC32.h CCAMACModule.h
  DESTINATION include/sbsmodules)

#  COMPILATION_FLAGS ($(VMEBUILDEADERS) is not defined anywhere).
#
#  -I@top_srcdir@/sbs/readout also found the gengetopt generated options.h
#  (included by CReadoutMain.h) which now lives in the sbs/readout binary
#  directory.

set(_scripted_includes
  ${PROJECT_SOURCE_DIR}/base/headers
  ${PROJECT_SOURCE_DIR}/sbs/nsclapi
  ${PROJECT_SOURCE_DIR}/sbs/vmemodules
  ${PROJECT_SOURCE_DIR}/sbs/readout
  ${PROJECT_BINARY_DIR}/sbs/readout
  ${PROJECT_SOURCE_DIR}/base/os
  ${PROJECT_SOURCE_DIR}/base/factories)
set(_scripted_defs VME INSTDIR="${prefix}")

function(_scripted_flags tgt)
  target_include_directories(${tgt} PRIVATE ${_scripted_includes})
  target_compile_definitions(${tgt} PRIVATE ${_scripted_defs})
  target_compile_options(${tgt} PRIVATE -pthread)
  target_link_libraries(${tgt} PRIVATE
    NSCLDAQ::tclPlusHeaders NSCLDAQ::TclHeaders NSCLDAQ::PixieHeaders)
  add_dependencies(${tgt} ggo_sbs_readout_options)
endfunction()

nscldaq_add_library(ScriptedRdo VERSION_INFO ${SOVERSION}
  SOURCES CScriptedSegment.cpp CBoolConfigParam.cpp CCAENModule.cpp
    CCAENV775.cpp CCAENV775Creator.cpp CCAENV785.cpp CCAENV785Creator.cpp
    CCAENV792.cpp CCAENV792Creator.cpp CCAENV830Creator.cpp
    CCAENV830Module.cpp CSIS3300Creator.cpp CSIS3300Module.cpp
    CConfigurationParameter.cpp CDigitizerDictionary.cpp
    CDigitizerModule.cpp CIntArrayParam.cpp CIntConfigParam.cpp
    CModuleCommand.cpp CModuleCreator.cpp CScalerBank.cpp CScalerModule.cpp
    CStringArrayparam.cpp CStringConfigParam.cpp CConfigurableObject.cpp
    CReadableObject.cpp CReadException.cpp CPacketCreator.cpp
    CEnumParameter.cpp CV1x90Creator.cpp CV1x90Module.cpp CReadOrder.cpp
    CScriptedScalers.cpp ScriptedBundle.cpp)
_scripted_flags(ScriptedRdo)

nscldaq_add_executable(ScriptedReadout SOURCES Skeleton.cpp)
_scripted_flags(ScriptedReadout)
target_link_libraries(ScriptedReadout PRIVATE
  ScriptedRdo SBSProductionReadout SBSDeviceSupport SBSVmeAPI daqthreads
  daqshm NSCLDAQ::tclPlus NSCLDAQ::Tcl pthread)

#  install-exec-local: the skeleton for user extensions of the scripted
#  readout.  Makefile.user is INSTDIR=@prefix@ followed by Makefile.skel.

file(READ ${CMAKE_CURRENT_SOURCE_DIR}/Makefile.skel _scripted_skel)
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/Makefile.user
  "INSTDIR=${prefix}\n${_scripted_skel}")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS Makefile.skel)

nscldaq_install_mkdir(skeletons skeletons/scriptedReadout
  skeletons/scriptedReadout/include skeletons/scriptedReadout/skel
  skeletons/scriptedReadout/src etc)
file(GLOB _scripted_cpp CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB _scripted_h CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
install(FILES ${_scripted_cpp} DESTINATION skeletons/scriptedReadout/src)
install(FILES ${_scripted_h} DESTINATION skeletons/scriptedReadout/include)
install(FILES Skeleton.cpp Skeleton.h DESTINATION skeletons/scriptedReadout/skel)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Makefile.user
  DESTINATION skeletons/scriptedReadout/skel RENAME Makefile)
install(FILES scriptedreadout_Makefile.include DESTINATION etc)

#  Tests

nscldaq_add_executable(sbs_scripted_unittests TEST OUTPUT_NAME unittests
  SOURCES TestRunner.cpp factorytests.cpp CModuleCommand.cpp
    CDigitizerDictionary.cpp CModuleCreator.cpp CCAENModule.cpp CCAENV775.cpp
    CCAENV775Creator.cpp CCAENV785.cpp CCAENV785Creator.cpp CCAENV792.cpp
    CCAENV792Creator.cpp CCAENV830Module.cpp CCAENV830Creator.cpp
    CV1x90Module.cpp CV1x90Creator.cpp CSIS3300Module.cpp CSIS3300Creator.cpp
    CDigitizerModule.cpp CReadableObject.cpp CReadOrder.cpp
    CReadException.cpp CConfigurableObject.cpp CConfigurationParameter.cpp
    CEnumParameter.cpp CIntArrayParam.cpp CIntConfigParam.cpp
    CStringArrayparam.cpp CStringConfigParam.cpp CBoolConfigParam.cpp)
_scripted_flags(sbs_scripted_unittests)
target_link_libraries(sbs_scripted_unittests PRIVATE
  NSCLDAQ::CppUnit SBSProductionReadout SBSDeviceSupport SBSVmeAPI
  daqthreads daqshm NSCLDAQ::tclPlus NSCLDAQ::TclOnly)
nscldaq_add_test(NAME unittests COMMAND sbs_scripted_unittests)

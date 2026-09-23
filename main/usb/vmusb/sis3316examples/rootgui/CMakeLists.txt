#
#  usb/vmusb/sis3316examples/rootgui: root_gui (installed in ${prefix}/sis3316)
#  plus its images and example configuration files.
#

nscldaq_add_executable(sis3316_root_gui OUTPUT_NAME root_gui DESTINATION sis3316
  SOURCES get_configuration_parameters.cpp sis3316_cern_root_class.cpp
    sis3316_class.cpp sis3316_ethernet_access_class.cpp
    sis3316_root_gui_flash.cpp sis3316_root_gui_main.cpp
    sis3316_root_gui_register_access.cpp sis3316_root_gui_test1.cpp
    sis3316_root_gui_udp_properties.cpp)
target_include_directories(sis3316_root_gui PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/../../vmusb
  ${PROJECT_SOURCE_DIR}/base/thread)
target_compile_definitions(sis3316_root_gui PRIVATE
  IMAGES_DIR="${datarootdir}/sis_images")
# The Makefile.am forced CFLAGS/CXXFLAGS=-g -O0: this program does not run
# properly with the optimizer.
target_compile_options(sis3316_root_gui PRIVATE -O0)
target_link_libraries(sis3316_root_gui PRIVATE
  NSCLDAQ::ROOT VMUSB NSCLDAQ::Tcl fftw3 m)

#  install-exec-local

nscldaq_install_mkdir(share/sis_images share/sis3316_configs)
install(FILES sis1_sis3316.png sislogo.bmp DESTINATION share/sis_images)
install(FILES
  sis3316_MultiEvent_10000_ExternalTrigger_with_Ch1_InternalTrigger_Feedback_record_ch1_3_5_6.ini
  sis3316_MultiEvent_ExternalTrigger_NIM_TI_AllChannels.ini
  sis3316_MultiEvent_ExternalTrigger_NIM_TI_AllChannels_StopAfter60sec.ini
  sis3316_MultiEvent_InternalTrigger_EnergyP150G50.ini
  sis3316_MultiEvent_InternalTrigger_EnergyP298G200_DisplayAllHistograms.ini
  sis3316_parameter_test_external_trigger_software.ini
  sis3316_parameter_test_internal_trigger.ini
  sis3316_SingleEvent_ExternalTrigger_NIM_TI_AllChannels.ini
  sis3316_SingleEvent_ExternalTrigger_NIM_TI_Ch1-8.ini
  sis3316_SingleEvent_ExternalTrigger_with_Ch1_InternalTrigger_Feedback.ini
  sis3316_SingleEvent_ExternalTrigger_with_Ch1_InternalTrigger_TO_TI_connection.ini
  sis3316_SingleEvent_InternalPileupTriggerOnly_AllChannels.ini
  sis3316_SingleEvent_InternlTrigger_DisplayEnergyMaw_DecayPulse.ini
  sis3316_SingleEvent_InternlTrigger_DisplayTriggerMaw.ini
  DESTINATION share/sis3316_configs)

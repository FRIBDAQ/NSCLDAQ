#
#  s800scaler_gen - generates a scaler definition for the VMUSBReadout
#  from a register definition file for the S800 trigger module.
#

nscldaq_gengetopt(options options.ggo ARGS --unnamed-opts C_EXTENSION cpp)

nscldaq_add_executable(s800scaler_gen SOURCES main.cpp)
nscldaq_use_gengetopt(s800scaler_gen options)
target_compile_definitions(s800scaler_gen PRIVATE
  DEFAULT_REGISTER_FILE="${datarootdir}/s800trigger/RegisterFile.json")
target_link_libraries(s800scaler_gen PRIVATE NSCLDAQ::JsonCpp NSCLDAQ::Tcl)

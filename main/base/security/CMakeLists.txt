nscldaq_add_library(Security VERSION_INFO ${SOVERSION}
  SOURCES AccessListCheck.cpp FdInteractor.cpp HostListCheck.cpp
    IOInteractor.cpp Interactor.cpp PasswordCheck.cpp StringInteractor.cpp
    TclAccessListCheck.cpp UnixUserCheck.cpp)
target_include_directories(Security PRIVATE
  ${PROJECT_SOURCE_DIR}/base/headers ${PROJECT_SOURCE_DIR}/base/os)
target_link_libraries(Security
  PRIVATE NSCLDAQ::TclHeaders NSCLDAQ::PixieHeaders
  PUBLIC daqshm NSCLDAQ::tclPlus NSCLDAQ::Threads)

nscldaq_install_headers(AccessListCheck.h Authenticator.h FdInteractor.h
  HostListCheck.h IOInteractor.h Interactor.h PasswordCheck.h
  StringInteractor.h TclAccessListCheck.h UnixUserCheck.h)

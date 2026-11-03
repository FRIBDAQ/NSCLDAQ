/**
 * @file CXIAException.cpp
 * @brief Implement the XIA exception-handling class for DDAS.
 */

#include "CXIAException.h"

#include <sstream>

#include <config.h>
#include <config_pixie16api.h>

/**
 * @details
 * Get the XIA API error message from the passed return value and construct
 * the full error message.
 * @note (ASC 11/2/26): Unless XIA can guarantee that the buffer is always
 * null-terminated, we need to make sure that it is. We will use a buffer of
 * size kXIABUFSIZE and pass kXIABUFSIZE-1 to the PixieGetReturnCodeText()
 * function to ensure that the last character is always a null terminator. This
 * implementation assumes the actual error message doesn't exceed kXIABUFSIZE-1
 * characters.
 */
CXIAException::CXIAException(std::string msg, std::string fcn, int rv)
    : CException(msg), m_reasonCode(rv) {
  char buf[kXIABUFSIZE] = {}; // XIA reason text.
  PixieGetReturnCodeText(m_reasonCode, buf, kXIABUFSIZE - 1);
  std::ostringstream errmsg;
  m_reason = formatReason(WasDoing(), fcn, m_reasonCode, buf);
}

std::string CXIAException::formatReason(const char *doing,
                                        const std::string &fcn, int rv,
                                        const char *buf) {
  std::ostringstream errmsg;
  errmsg << doing << " XIA API Error: " << fcn << " returned " << rv
         << " with reason text '" << buf << "'";
  return errmsg.str();
}
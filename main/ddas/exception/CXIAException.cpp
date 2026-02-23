/**
 * @file CXIAException.cpp
 * @brief Implement the XIA exception-handling class for DDAS.
 */

#include "CXIAException.h"

#include <cstdio>

#include <config.h>
#include <config_pixie16api.h>

/**
 * @details
 * Get the XIA API error message from the passed return value and construct
 * the full error message.
 */
CXIAException::CXIAException(std::string msg, std::string fcn, int rv)
    : CException(msg), m_reasonCode(rv) {
  char errmsg[kREASONSIZE]; // Full context message.
  char buf[kXIABUFSIZE];    // XIA reason text.
  PixieGetReturnCodeText(m_reasonCode, buf, kXIABUFSIZE);
  sprintf(errmsg, "%s XIA API Error: %s returned %d with reason text '%s'",
          WasDoing(), fcn.c_str(), m_reasonCode, buf);
  m_reason = errmsg;
}

int CXIAException::ReasonCode() const { return m_reasonCode; }

const char *CXIAException::ReasonText() const { return m_reason.c_str(); }

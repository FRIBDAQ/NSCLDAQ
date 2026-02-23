/**
 * @file CXIAException.h
 * @brief Class to wrap XIA return codes and messages in a CException.
 */

#ifndef CXIAEXCEPTION_H
#define CXIAEXCEPTION_H

#include <Exception.h>

#include <string>

/**
 * @class CXIAException
 * @brief CException wrapper for XIA return codes containing the return code
 * and context message of the error.
 * @details
 * XIA API major version 4 and later implement a return code and context message
 * for non-zero return values from API functions. This class provides a wrapper
 * for that business using the CException class:
 * * m_reasonCode holds the XIA API return value,
 * * The XIA API return value is used to generate an XIA API context message
 * for that error,
 * * m_szAction holds some additional user-provided context message (passed
 * to the CException constructor as-is).
 * The full error message incorporating the XIA API return code and its
 * associated context message as well as the user-provided context can be
 * accessed using the class' `ReasonText()` method. The value of the return
 * code can be accessed using the `ReasonCode()` method.
 */

const size_t kXIABUFSIZE = 1024; //!< XIA API return code max buffer length.
const size_t kREASONSIZE = 2048; //!< Reason text max buffer length.

class CXIAException : public CException {
private:
  int m_reasonCode;     //!< XIA API function return value.
  std::string m_reason; //!< Full reason text.

public:
  /**
   * @brief Constructor from arguments.
   * @param msg User context message for the error.
   * @param fcn XIA API function name.
   * @param rv  XIA API function return value.
   */
  CXIAException(std::string msg, std::string fcn, int rv);

  /** @brief Destructor. Nothing to do here. */
  virtual ~CXIAException() {};

  // Exception interface:

  /**
   * @brief Get the reason code.
   * @return The reason code: non-zero return value from the XIA API.
   */
  virtual int ReasonCode() const;
  /**
   * @brief Get the return text.
   * @return Full context message describing the error.
   */
  virtual const char *ReasonText() const;
};

#endif

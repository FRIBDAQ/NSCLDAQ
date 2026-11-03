/**
 * @file CXIAException.h
 * @brief Class to wrap XIA return codes and messages in a CException.
 */

#ifndef CXIAEXCEPTION_H
#define CXIAEXCEPTION_H

#include <Exception.h>

#include <string>

const size_t kXIABUFSIZE = 1024; //!< XIA API return code max buffer length.

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
  virtual ~CXIAException(){};

  /**
   * @brief Format the full CXIAException reason text.
   * @param doing Context string from CException::WasDoing().
   * @param fcn   XIA API function name.
   * @param rv    XIA API function return value.
   * @param buf   NUL-terminated XIA reason text from PixieGetReturnCodeText().
   * @return The full formatted reason message.
   * @note (ASC 11/3/26): Static helper which can be tested to ensure
   * NUL-terminated messages.
   */
  static std::string formatReason(const char *doing, const std::string &fcn,
                                  int rv, const char *buf);

  // Exception interface:

  /**
   * @brief Get the reason code.
   * @return The reason code: non-zero return value from the XIA API.
   */
  virtual int ReasonCode() const { return m_reasonCode; };
  /**
   * @brief Get the return text.
   * @return Full context message describing the error.
   */
  virtual const char *ReasonText() const { return m_reason.c_str(); };
};

#endif

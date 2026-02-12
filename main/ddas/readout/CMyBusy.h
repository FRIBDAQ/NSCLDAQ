/**
 * @file CMyBusy.h
 * @brief Define a busy class for DDAS.
 */

#include <CBusy.h>

/**
 * @class CMyBusy
 * @brief Provides a busy command to override the SBS one. There is no concept
 * of "busy" for DDAS systems, so all of these class' functions are no-ops.
 */

class CMyBusy : public CBusy {
public:
  CMyBusy();    //!< Default constructor
  ~CMyBusy() {} //!< Destructor.

public:
  virtual void GoBusy();  //!< Do nothing.
  virtual void GoClear(); //!< Do nothing.
};

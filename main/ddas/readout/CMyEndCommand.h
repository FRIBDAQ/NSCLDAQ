/**
 * @file CMyEndCommand.h
 * @brief Define the end run command.
 */

#ifndef MYEND_H
#define MYEND_H

#include <tcl.h>

#include <CEndCommand.h>

class CTCLInterpreter;
class CTCLObject;
class CMyEventSegment;
class CExperiment;

/**
 * @class CMyEndCommand
 * @brief Provides an end command to override the SBS one.
 */

class CMyEndCommand : public CEndCommand {
public:
  /** @brief Struct encapsulating the command and Tcl event to end the run. */
  struct EndEvent {
    Tcl_Event s_rawEvent;     //!< Generic event for the Tcl event system.
    CMyEndCommand *s_thisPtr; //!< Pointer to this command.
  };

private:
  CMyEventSegment *m_pSeg; //!< End for modules in this segment.
  CExperiment *m_pExp;     //!< The experiment we're reading data from.
  int m_nModules;          //!< Number of modules in the event segment.

public:
  /**
   * @brief Constructor.
   * @param interp Reference to interpreter.
   * @param pSeg   Pointer to the event segment to manipulate.
   * @param pExp   Pointer to the experiment we're reading data from.
   */
  CMyEndCommand(CTCLInterpreter &interp, CMyEventSegment *pSeg,
                CExperiment *pExp);
  /** @brief Destructor. */
  ~CMyEndCommand();

private:
  /**
   * @brief Copy constructor.
   * @param rhs References the CMyEndCommand we are copy-constructing.
   */
  CMyEndCommand(const CMyEndCommand &rhs);
  /**
   * @brief operator=
   * @param rhs References the CMyEndCommand we are assigning to lhs.
   * @return Reference to left-hand side operand.
   */
  CMyEndCommand &operator=(const CMyEndCommand &rhs);
  /**
   * @brief operator==
   * @param rhs References the CMyEndCommand for comparison.
   */
  int operator==(const CMyEndCommand &rhs) const;
  /**
   * @brief operator!=
   * @param rhs References the CMyEndCommand for comparison.
   */
  int operator!=(const CMyEndCommand &rhs) const;

  // Class operations:
public:
  int transitionToInactive();
  int readOutRemainingData();
  int endRun();
  void rescheduleEndTransition();
  void rescheduleEndRead();
  virtual int operator()(CTCLInterpreter &interp,
                         std::vector<CTCLObject> &objv);

protected:
  static int handleEndRun(Tcl_Event *pEvt, int flags);
  static int handleReadOutRemainingData(Tcl_Event *pEvt, int flags);
};
#endif

/**
 * @file CMyTrigger.h
 * @brief Define a trigger class for DDAS.
 */

#ifndef CMYTRIGGER_H
#define CMYTRIGGER_H

#include <ctime>

#include <CEventTrigger.h>

/**
 * @class CMyTrigger
 * @brief Trigger class for DDAS.
 * @details
 * A trigger for DDAS systems intended to run inside a polling loop that asks
 * the trigger if it has enough data to read out. The trigger logic is defined
 * in the call operator which triggers a read for a crate of Pixie modules if
 * any module in the crate exceeds its trigger threshold (FIFO threshold value).
 */

class CMyTrigger : public CEventTrigger {
private:
  bool m_retrigger;         //!< Retrigger flag for Pixie buffer readout.
  unsigned int nFIFOWords;  //!< Words in Pixie output data buffer.
  int NumberOfModules;      //!< Number of Pixie modules.
  unsigned short ModNum;    //!< Pixie module number.
  unsigned m_fifoThreshold; //!< FIFO readout threshold.
  time_t m_lastTriggerTime; //!< Last time operator() returned true.
  unsigned int *m_wordsInEachModule; //!< Current FIFO sizes.

  // Constructors, destructors and other cannonical operations:
public:
  /** @brief Default constructor. */
  CMyTrigger();
  /** @brief Destructor. */
  ~CMyTrigger();

  // Mutators:
protected:
  // Class operations:
public:
  /** @brief Start the trigger timeout. */
  virtual void setup();
  /** @brief Called as data taking ends. */
  virtual void teardown();
  /**
   * @brief operator()
   * @return bool
   * @retval true Good trigger, pass control back to the event segment.
   * @retval false Not enough data to trigger.
   */
  virtual bool operator()();
  /**
   * @brief Setup the trigger and FIFO words array.
   * @param nummod The number of installed modules in the Pixie setup.
   *     Received from the event segment class.
   */
  virtual void Initialize(int nummod);
  /**
   * @brief Control for determing if trigger should poll modules or pass
   * control back to CEventSegment for processing the previous block of data.
   */
  void Reset();
  /**
   * @brief Get the number of words in each module.
   * @return Pointer to the array containing the number of words each
   *     module has.
   */
  unsigned int *getWordsInModules() const { return m_wordsInEachModule; };
};

#endif

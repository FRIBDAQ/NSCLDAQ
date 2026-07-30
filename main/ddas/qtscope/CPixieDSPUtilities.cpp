/**
 * @file CPixieDSPUtilities.cpp
 * @brief Implementation DSP utilities class.
 */

#include "CPixieDSPUtilities.h"

#include <iostream>
#include <sstream>

#include <CXIAException.h>
#include <config.h>
#include <config_pixie16api.h>

int CPixieDSPUtilities::AdjustOffsets(int module) {
  try {
    int retval = Pixie16AdjustOffsets(module);
    if (retval < 0) {
      std::stringstream msg;
      msg << "Failed to adjust offsets in module " << module;
      throw CXIAException(msg.str(), "Pixie16AdjustOffsets()", retval);
    }
  } catch (const CXIAException &e) {
    m_lastErrorMessage = e.ReasonText();
    std::cerr << m_lastErrorMessage << std::endl;
    return e.ReasonCode();
  }

  return 0;
}

/**
 * @details
 * Channel parameters are doubles. For a list of parameters and their units,
 * see the Pixie-16 Programmers Manual, pgs. 60-61.
 */
int CPixieDSPUtilities::WriteChanPar(int module, int channel, char *paramName,
                                     double value) {
  try {
    int retval = Pixie16WriteSglChanPar(paramName, value, module, channel);
    if (retval < 0) {
      std::stringstream msg;
      msg << "Failed to write channel parameter " << paramName << " to module "
          << module << " channel " << channel;
      throw CXIAException(msg.str(), "Pixie16WriteSglChanPar()", retval);
    }
  } catch (const CXIAException &e) {
    m_lastErrorMessage = e.ReasonText();
    std::cerr << m_lastErrorMessage << std::endl;
    return e.ReasonCode();
  }

  return 0;
}

/**
 * @details
 * Channel parameters are doubles. For a list of parameters and their units,
 * see the Pixie-16 Programmers Manual, pgs. 60-61.
 */
int CPixieDSPUtilities::ReadChanPar(int module, int channel, char *paramName,
                                    double &value) {
  try {
    int retval = Pixie16ReadSglChanPar(paramName, &value, module, channel);
    if (retval < 0) {
      std::stringstream msg;
      msg << "Failed to read channel parameter " << paramName << " from module "
          << module << " channel " << channel;
      throw CXIAException(msg.str(), "Pixie16ReadSglChanPar()", retval);
    }
  } catch (const CXIAException &e) {
    m_lastErrorMessage = e.ReasonText();
    std::cerr << m_lastErrorMessage << std::endl;
    return e.ReasonCode();
  }

  return 0;
}

/**
 * @details
 * Module parameters are unsigned ints. For a list of parameters and their
 * units, see the Pixie-16 Programmers Manual, pgs. 62-63.
 */
int CPixieDSPUtilities::WriteModPar(int module, char *paramName,
                                    unsigned int value) {
  try {
    int retval = Pixie16WriteSglModPar(paramName, value, module);
    if (retval < 0) {
      std::stringstream msg;
      msg << "Failed to write module parameter " << paramName << " to module "
          << module;
      throw CXIAException(msg.str(), "Pixie16WriteSglModPar()", retval);
    }
  } catch (const CXIAException &e) {
    m_lastErrorMessage = e.ReasonText();
    std::cerr << m_lastErrorMessage << std::endl;
    return e.ReasonCode();
  }

  return 0;
}

/**
 * @details
 * Module parameters are unsigned ints. For a list of parameters and their
 * units, see the Pixie-16 Programmers Manual, pgs. 62-63.
 */
int CPixieDSPUtilities::ReadModPar(int module, char *paramName,
                                   unsigned int &value) {
  try {
    int retval = Pixie16ReadSglModPar(paramName, &value, module);
    if (retval < 0) {
      std::stringstream msg;
      msg << "Failed to read module parameter " << paramName << " from module "
          << module;
      throw CXIAException(msg.str(), "Pixie16ReadSglModPar()", retval);
    }
  } catch (const CXIAException &e) {
    m_lastErrorMessage = e.ReasonText();
    std::cerr << m_lastErrorMessage << std::endl;
    return e.ReasonCode();
  }

  return 0;
}

extern "C" {
CPixieDSPUtilities *CPixieDSPUtilities_new() {
  return shimGuardNew("CPixieDSPUtilities_new",
                      []() { return new CPixieDSPUtilities(); });
}

int CPixieDSPUtilities_AdjustOffsets(CPixieDSPUtilities *utils, int mod) {
  return shimGuard(utils, "CPixieDSPUtilities_AdjustOffsets",
                   SHIM_UNEXPECTED_ERROR,
                   [=]() { return utils->AdjustOffsets(mod); });
}

int CPixieDSPUtilities_WriteChanPar(CPixieDSPUtilities *utils, int mod,
                                    int chan, char *pName, double val) {
  return shimGuard(
      utils, "CPixieDSPUtilities_WriteChanPar", SHIM_UNEXPECTED_ERROR,
      [=]() { return utils->WriteChanPar(mod, chan, pName, val); });
}

int CPixieDSPUtilities_ReadChanPar(CPixieDSPUtilities *utils, int mod, int chan,
                                   char *pName, double *val) {
  return shimGuard(
      utils, "CPixieDSPUtilities_ReadChanPar", SHIM_UNEXPECTED_ERROR,
      [=]() { return utils->ReadChanPar(mod, chan, pName, *val); });
}

int CPixieDSPUtilities_WriteModPar(CPixieDSPUtilities *utils, int mod,
                                   char *pName, unsigned int val) {
  return shimGuard(utils, "CPixieDSPUtilities_WriteModPar",
                   SHIM_UNEXPECTED_ERROR,
                   [=]() { return utils->WriteModPar(mod, pName, val); });
}

int CPixieDSPUtilities_ReadModPar(CPixieDSPUtilities *utils, int mod,
                                  char *pName, unsigned int *val) {
  return shimGuard(utils, "CPixieDSPUtilities_ReadModPar",
                   SHIM_UNEXPECTED_ERROR,
                   [=]() { return utils->ReadModPar(mod, pName, *val); });
}

const char *CPixieDSPUtilities_GetLastErrorMessage(CPixieDSPUtilities *utils) {
  return utils->GetLastErrorMessage();
}

void CPixieDSPUtilities_delete(CPixieDSPUtilities *utils) {
  try {
    delete utils;
  } catch (...) {
    std::cerr << "CPixieDSPUtilities_delete unknown exception" << std::endl;
  }
};
}

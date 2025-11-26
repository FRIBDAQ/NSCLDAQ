#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <string>
#include <iostream>
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include <stdexcept>
#include <CRunState.h>

using namespace std;

int main(int argc, char** argv)
{
  CppUnit::TextUi::TestRunner   
               runner; // Control tests.
  CppUnit::TestFactoryRegistry& 
               registry(CppUnit::TestFactoryRegistry::getRegistry());

  runner.addTest(registry.makeTest());

  bool wasSucessful;
  try {
    wasSucessful = runner.run("",false);
  } 
  catch(string& rFailure) {
    cerr << "Caught a string exception from test suites.: \n";
    cerr << rFailure << endl;
    wasSucessful = false;
  }
  return !wasSucessful;
}


/**
 * getConfigVal
 *    Return the value of a configuration item.
 *
 * @param interp - intepreter to use to bind the obj.
 * @param option - the option to look for.
 * @param configString - the string containing the configuration.
 * @return std::string - the option value or throws an exception if there's no such
 *                        option.
 */
std::string
getConfigVal(CTCLInterpreter& interp, const char* option, std::string configString)
{
  CTCLObject config;
  config.Bind(interp);
  config = configString;
  
  for (int i = 0; i < config.llength(); i++) {
    CTCLObject item  = config.lindex(i);
    item.Bind(interp);
    CTCLObject key   = item.lindex(0);
    CTCLObject value = item.lindex(1);
    key.Bind(interp); value.Bind(interp);

    if (std::string(option) == std::string(key)) {
      return std::string(value);
    }
  }
  throw std::runtime_error("No such key");
}

// Run state stubs: Not actually used in tests.
// just satisfy globals.

CRunState* CRunState::m_pTheInstance(nullptr);

CRunState::CRunState() {}
CRunState::~CRunState() {}

CRunState* CRunState::getInstance() {
  if (!m_pTheInstance) m_pTheInstance = new CRunState;
  return m_pTheInstance;
}


uint16_t
CRunState::getRunNumber() {
  return 0;
}
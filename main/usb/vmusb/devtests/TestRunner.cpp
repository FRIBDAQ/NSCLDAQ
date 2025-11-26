#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>

#include <string>
#include <iostream>
#include <CVMUSB.h>
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

/// stubs:

class CConfiguration;

namespace Globals
{
  int pConfig(0);
  int scalerPeriod(0);
  CVMUSB* pUSBController(0);
};
void* gpTCLApplication(0);


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
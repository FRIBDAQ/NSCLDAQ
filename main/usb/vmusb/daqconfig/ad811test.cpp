// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#include <string>


#include <TCLInterpreter.h>
#include <TCLObject.h>

#include "CConfiguration.h"
#include <CVMUSB.h>

// Stubs for the tests.

void* gpTCLApplication(0);

namespace Globals {
  CConfiguration* pConfig(0);
  CVMUSB*         pUSBController(0);
  unsigned        scalerPeriod(10);
};


// Test class

class Ad811Tests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(Ad811Tests);
  CPPUNIT_TEST(create);
  CPPUNIT_TEST(config);
  CPPUNIT_TEST_SUITE_END();


private:

public:
  void setUp() {
    ::Globals::pConfig = new CConfiguration;
  }
  void tearDown() {
    delete ::Globals::pConfig;
    ::Globals::pConfig = 0;
  }
protected:
  void create();
  void config();

};

CPPUNIT_TEST_SUITE_REGISTRATION(Ad811Tests);

//////////////////////////// Test utilities. ///////////////////////////////

 //////////////////////// The tests /////////////////////////////////////////
 
/**
 * Make an ad811 and ensure it has the right base address.
 */
void Ad811Tests::create() {
  CTCLInterpreter* pInterp = ::Globals::pConfig->getInterpreter();
  pInterp->GlobalEval("adc create testing 0x12340000");
  std::string configString = pInterp->GlobalEval("adc cget testing");

  EQ(std::string("0x12340000"), getConfigVal(*pInterp, "-base", configString));
}
// Test post creation configuration:

void Ad811Tests::config() {
  CTCLInterpreter* pInterp = ::Globals::pConfig->getInterpreter();
  pInterp->GlobalEval("adc create testing 0x12340000");
  pInterp->GlobalEval("adc config testing -geo 5");
  std::string configString = pInterp->GlobalEval("adc cget testing");
  EQ(std::string("5"), getConfigVal(*pInterp, "-geo", configString));
}


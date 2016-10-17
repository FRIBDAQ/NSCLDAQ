// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include <Asserts.h>

#include <tcl.h>
#include <TCLInterpreter.h>
#include <sstream>


class pkgloadTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(pkgloadTests);
  CPPUNIT_TEST(load);
  CPPUNIT_TEST(cmd);
  CPPUNIT_TEST_SUITE_END();


private:
  CTCLInterpreter* m_pInterpObj;
  Tcl_Interp*      m_pInterpRaw;
public:
  void setUp() {
    // Create the interpreter and fish out the raw interp for convenience.
    
    m_pInterpObj = new CTCLInterpreter();
    m_pInterpRaw  = m_pInterpObj->getInterpreter();
    Tcl_Init(m_pInterpRaw);
    
    // Add the Tcl package path to the intepreter:
    
    std::stringstream cmd;
    cmd << "lappend auto_path " << TCLLIBPATH << std::endl;
    m_pInterpObj->GlobalEval(cmd.str());
  }
  void tearDown() {
    delete m_pInterpObj;
  }
protected:
  void load();
  void cmd();
};

CPPUNIT_TEST_SUITE_REGISTRATION(pkgloadTests);


// Need to be able to load the package:

void pkgloadTests::load() {
  const char* version = Tcl_PkgRequire(m_pInterpRaw, "statusMessage", "1.0", 0);
  ASSERT(version);
}
// Loading the package defines the "RingStatistics" command

void pkgloadTests::cmd()
{
  Tcl_PkgRequire(m_pInterpRaw, "statusMessage", "1.0", 0);
  std::string cmds = m_pInterpObj->Eval("info commands RingStatistics");
  EQ(std::string("RingStatistics"), cmds);
}

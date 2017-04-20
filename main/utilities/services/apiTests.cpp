// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include "CServiceApi.h"
#include <CVarMgrFileApi.h>
#include <CVariableDb.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>


// Note that we will use a local (file) uri for the
// service api to simplify setup/teardown.

class ServerApiTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(ServerApiTests);
  CPPUNIT_TEST(existsDoes);
  CPPUNIT_TEST(existsDoesNot);
  
  CPPUNIT_TEST(canCreateDir);
  CPPUNIT_TEST(createProgOk);
  CPPUNIT_TEST(createProgDup);
  
  CPPUNIT_TEST(setHostOk);
  CPPUNIT_TEST(setHostNosuch);

  CPPUNIT_TEST(setCommandOk);
  
  CPPUNIT_TEST(setPos);
  CPPUNIT_TEST(getXpos);
  CPPUNIT_TEST(getYpos);
  
  
  CPPUNIT_TEST(removeSimple);
  CPPUNIT_TEST(removeWithSubinfo);
  
  CPPUNIT_TEST(listEmpty);
  CPPUNIT_TEST(list1);
  CPPUNIT_TEST(listsome);
  
  CPPUNIT_TEST(listByNameOk);
  
  // Tests of the new property interface.
  
  CPPUNIT_TEST(addProp);
  CPPUNIT_TEST(setExistingProp);
  CPPUNIT_TEST(setNonExProp);
  CPPUNIT_TEST(getExistingProp);
  CPPUNIT_TEST(getNonExProp);
  CPPUNIT_TEST_SUITE_END();


private:
  std::string     m_dbName;
  CVarMgrFileApi* m_pBaseApi;
  CServiceApi*    m_pSvcApi;
public:
  void setUp() {
    
    // Make the database and a base API.
    
    char nameTemplate[15];
    strcpy(nameTemplate, "/tmp/dbXXXXXX");
    int fd = mkstemp(nameTemplate);
    CVariableDb::create(nameTemplate);
    m_dbName = nameTemplate;
    m_pBaseApi = new CVarMgrFileApi(nameTemplate);
    
    
    // Make the svc API
    
    std::string svcUri = "file://";
    svcUri            += m_dbName;
    m_pSvcApi          = new CServiceApi(svcUri.c_str());
    close(fd);
  }
  void tearDown() {
    delete m_pBaseApi;
    delete m_pSvcApi;
    unlink(m_dbName.c_str());
  }
protected:
  void existsDoes();
  void existsDoesNot();
  
  void canCreateDir();
  
  void createProgOk();
  void createProgDup();
  
  void setHostOk();
  void setHostNosuch();
  
  void setCommandOk();
  void setPos();
  void getXpos();
  void getYpos();
  
  void removeSimple();
  void removeWithSubinfo();
  
  void listEmpty();
  void list1();
  void listsome();
  
  void listByNameOk();
  
  // Property API tests:
  
   void addProp();
   void setExistingProp();
   void setNonExProp();
   void getExistingProp();
   void getNonExProp();
private:
  void progMatch(
      std::map<std::string, std::pair<std::string, std::string> >& listing,
      const char* progname, const char* path, const char*host
  );
};

CPPUNIT_TEST_SUITE_REGISTRATION(ServerApiTests);

// Utilities:

/* See if a program listing has a program */

void ServerApiTests::progMatch(
  std::map<std::string, std::pair<std::string, std::string> >& listing,
  const char* progname, const char* path, const char*host
)
{
  ASSERT(listing.find(progname) != listing.end());
  std::pair<std::string, std::string> pInfo = listing[progname];
  
  EQ(std::string(path), pInfo.first);
  EQ(std::string(host), pInfo.second);
}

// The "/Services" directory exists:

void ServerApiTests::existsDoes() {
  m_pBaseApi->mkdir(CServiceApi::m_ServiceDir);
  
  ASSERT(m_pSvcApi->exists());
}

// The services directory does not exist:

void ServerApiTests::existsDoesNot()
{
  ASSERT(! m_pSvcApi->exists());
}

// Can create the directory:

void ServerApiTests::canCreateDir()
{
  m_pSvcApi->create();
  ASSERT(m_pSvcApi->exists());
}

// Can create a new program:

void ServerApiTests::createProgOk()
{
  m_pSvcApi->create();               // the directory
  
  m_pSvcApi->create("NewProgram", "acommand", "ahost");
  
  std::vector<std::string> dirs = m_pBaseApi->ls(CServiceApi::m_ServiceDir);
  EQ(size_t(1), dirs.size());
  EQ(std::string("NewProgram"), dirs[0]);
  
  m_pBaseApi->cd(CServiceApi::m_ServiceDir);
  m_pBaseApi->cd("NewProgram");
  
  EQ(std::string("acommand"), m_pBaseApi->get("path"));
  EQ(std::string("ahost"), m_pBaseApi->get("host"));
  EQ(std::string("0"), m_pBaseApi->get("editorx"));
  EQ(std::string("0"), m_pBaseApi->get("editory"));
  
}
// New program can't duplicate an old one:

void ServerApiTests::createProgDup()
{
  m_pSvcApi->create();               // the directory
  
  m_pSvcApi->create("NewProgram", "acommand", "ahost");
  
  CPPUNIT_ASSERT_THROW(
    m_pSvcApi->create("NewProgram", "anotherCommand", "anotherHost"),
    std::exception
  );
}

// Set new host ok.

void ServerApiTests::setHostOk()
{
  m_pSvcApi->create();               // the directory  
  m_pSvcApi->create("NewProgram", "acommand", "ahost");
  
  m_pSvcApi->setHost("NewProgram", "aNewHost");
  
  m_pBaseApi->cd(CServiceApi::m_ServiceDir);
  m_pBaseApi->cd("NewProgram");
  
  EQ(std::string("aNewHost"), m_pBaseApi->get("host"));
  
}
void ServerApiTests::setHostNosuch()
{
  m_pSvcApi->create();               // the directory
  
  CPPUNIT_ASSERT_THROW(
    m_pSvcApi->setHost("NewProgram", "ahost"),
    std::exception
  );
}
// setCommand ok:

void ServerApiTests::setCommandOk()
{
  m_pSvcApi->create();               // the directory  
  m_pSvcApi->create("NewProgram", "acommand", "ahost");

  m_pSvcApi->setCommand("NewProgram", "aNewCommand");
  m_pBaseApi->cd(CServiceApi::m_ServiceDir);
  m_pBaseApi->cd("NewProgram");

  EQ(std::string("aNewCommand"), m_pBaseApi->get("path"));  
}

void ServerApiTests::setPos()
{
  m_pSvcApi->create();
  m_pSvcApi->create("NewProgram", "acommand", "ahost");
  m_pSvcApi->setEditorPosition("NewProgram", 150, 200);
  
  m_pBaseApi->cd(CServiceApi::m_ServiceDir);
  m_pBaseApi->cd("NewProgram");
  
  EQ(std::string("150"), m_pBaseApi->get("editorx"));
  EQ(std::string("200"), m_pBaseApi->get("editory"));
}

void ServerApiTests::getXpos()
{
  m_pSvcApi->create();
  m_pSvcApi->create("NewProgram", "acommand", "ahost");
  m_pSvcApi->setEditorPosition("NewProgram", 150, 200);
  
  EQ(150, m_pSvcApi->getEditorXPosition("NewProgram"));
  
}

void ServerApiTests::getYpos()
{
  m_pSvcApi->create();
  m_pSvcApi->create("NewProgram", "acommand", "ahost");
  m_pSvcApi->setEditorPosition("NewProgram", 150, 200);
  
  EQ(200, m_pSvcApi->getEditorYPosition("NewProgram"));
  
}


// Simple remove means there's only vars in the directory.

void ServerApiTests::removeSimple()
{
  m_pSvcApi->create();               // the directory  
  m_pSvcApi->create("NewProgram", "acommand", "ahost");
  
  m_pSvcApi->remove("NewProgram");
  
  std::vector<std::string> dirs = m_pBaseApi->ls(CServiceApi::m_ServiceDir);
  
  
  
  EQ(size_t(0), dirs.size());
  
}
// Users could add additional information for the program...subdirs other
// vars etc.  make sure that all gets deleted properly.
//
void ServerApiTests::removeWithSubinfo()
{
  m_pSvcApi->create();               // the directory  
  m_pSvcApi->create("NewProgram", "acommand", "ahost");
  m_pBaseApi->cd("/Services/NewProgram");
  
  m_pBaseApi->declare("additionalVar", "integer");
  
  // Make a whole tree of stuff:
  
  m_pBaseApi->mkdir("asubdir");
  m_pBaseApi->mkdir("anotherSubdir");
  m_pBaseApi->mkdir("asubdir/asubsubdir");
  
  m_pBaseApi->declare("asubdir/anint", "integer");
  m_pBaseApi->declare("asubdir/asubsubdir/astring", "string", "hello");
  m_pBaseApi->declare("anotherSubdir/astring", "string", "world");
  m_pBaseApi->cd("/");
  
  m_pSvcApi->remove("NewProgram");
  EQ(size_t(0), m_pBaseApi->ls(CServiceApi::m_ServiceDir).size());
}

// List with no programs defined:

void ServerApiTests::listEmpty()
{
  m_pSvcApi->create();
  
  EQ(size_t(0), m_pSvcApi->list().size());
}
// list when 1 program is defined:

void ServerApiTests::list1()
{
  m_pSvcApi->create();
  m_pSvcApi->create("Test", "/bin/ls", "ahost");
  
  std::map<std::string, std::pair<std::string, std::string> > listing =
    m_pSvcApi->list();
    
  EQ(size_t(1), listing.size());
  progMatch(listing, "Test", "/bin/ls", "ahost");
  
}
// List when there are several programs:

void ServerApiTests::listsome()
{
  m_pSvcApi->create();
  m_pSvcApi->create("Test", "/bin/ls", "ahost");
  m_pSvcApi->create("atest", "/some/program", "another.host");
  m_pSvcApi->create("VarDbMgr", "/usr/opt/daq/11.0/bin/vardbServer", "this.host");
  
  std::map<std::string, std::pair<std::string, std::string> > listing =
    m_pSvcApi->list();
    
  EQ(size_t(3), listing.size());
  
  progMatch(listing, "Test", "/bin/ls", "ahost");
  progMatch(listing, "atest", "/some/program", "another.host");
  progMatch(listing, "VarDbMgr", "/usr/opt/daq/11.0/bin/vardbServer", "this.host");
}

// List program by name (exists).

void ServerApiTests::listByNameOk()
{
  m_pSvcApi->create();
  m_pSvcApi->create("Test", "/bin/ls", "ahost");
  
  std::pair<std::string, std::string> info = m_pSvcApi->list("Test");
  EQ(std::string("/bin/ls"), info.first);
  EQ(std::string("ahost"), info.second);
}

// Can set/create a propertyL:

void ServerApiTests::addProp()
{
  m_pSvcApi->create();
  m_pSvcApi->create("Test", "/bin/ls", "ahost");
  
  CPPUNIT_ASSERT_NO_THROW(
    m_pSvcApi->setProperty("Test", "aprop","avalue")
  );
  EQ(std::string("avalue"), m_pBaseApi->get("/Services/Test/aprop"));
}
// Given an existing property, setProperty can modify its value:

void ServerApiTests::setExistingProp()
{
  m_pSvcApi->create();
  m_pSvcApi->create("Test", "/bin/ls", "ahost");
  m_pSvcApi->setProperty("Test", "aprop","avalue");
  
  m_pSvcApi->setProperty("Test","aprop", "different value");
  EQ(std::string("different value"), m_pBaseApi->get("/Services/Test/aprop"));
}
// Setting a nonexisting property with create=false throws:

void ServerApiTests::setNonExProp()
{
  m_pSvcApi->create();
  m_pSvcApi->create("Test", "/bin/ls", "ahost");
  CPPUNIT_ASSERT_THROW(
    m_pSvcApi->setProperty("Test", "aprop", "avalue", false),
    std::exception
  );
}
//  If a property has been created it can be gotten:


void ServerApiTests::getExistingProp()
{
  m_pSvcApi->create();
  m_pSvcApi->create("Test", "/bin/ls", "ahost");
  m_pSvcApi->setProperty("Test", "aprop","avalue");
  EQ(std::string("avalue"), m_pSvcApi->getProperty("Test", "aprop"));
}
// std::exception is thrown if a nonexistent property is fetched:
void ServerApiTests::getNonExProp()
{
  m_pSvcApi->create();
  m_pSvcApi->create("Test", "/bin/ls", "ahost");

  CPPUNIT_ASSERT_THROW(
    m_pSvcApi->getProperty("Test", "aprop"),
    std::exception
  );
 
}
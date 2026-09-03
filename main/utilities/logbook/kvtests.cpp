/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Giordano Cerriza
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/** @file:  kvtests.cpp
 *  @brief: 
 */
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include "LogBook.h"
#include "logtestutils.hpp"
#include <CSqlite.h>
#include <CSqliteStatement.h>
#include <unistd.h>
#include <Asserts.h>
#include <iostream>

class kvtest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(kvtest);
    CPPUNIT_TEST(exists_1);
    CPPUNIT_TEST(exists_2);
    CPPUNIT_TEST(get_1);
    CPPUNIT_TEST(get_2);
    CPPUNIT_TEST(get_3);
    
    CPPUNIT_TEST(set_1);
    CPPUNIT_TEST(set_2);
    
    CPPUNIT_TEST(create_1);
    CPPUNIT_TEST(create_2);
    
    CPPUNIT_TEST(list_1);
    CPPUNIT_TEST_SUITE_END();
protected:
    void exists_1();
    void exists_2();
    
    void get_1();
    void get_2();
    void get_3();
    
    void set_1();
    void set_2();
    
    void create_1();
    void create_2();

    void list_1();
private:
    std::string m_database;
    LogBook*    m_pLogBook;
    CSqlite*    m_pConnection;
public:
    void setUp() {
        m_database = tempFilename("kvstoreXXXXXX");
        LogBook::create(m_database.c_str(), "Test", "Ron Fox", "Test kvstore");
        m_pLogBook    = new LogBook(m_database.c_str());
        m_pConnection = new CSqlite(m_database.c_str());
    }
    void tearDown() {
        delete m_pConnection;
        delete m_pLogBook;
        unlink(m_database.c_str());
    }

};

CPPUNIT_TEST_SUITE_REGISTRATION(kvtest);

void kvtest::exists_1()
{
    // exists - check for a key that does not exist.
    
    ASSERT(!m_pLogBook->kvExists("george"));
}
void kvtest::exists_2()
{
    // exists - check for a key that does exist.
    
    ASSERT(m_pLogBook->kvExists("experiment"));
}

void kvtest::get_1()
{
    // Get a nonexisting key throws LogBook::Exception
    
    CPPUNIT_ASSERT_THROW(
        m_pLogBook->kvGet("no-such-key"), LogBook::Exception
    );
}
void kvtest::get_2()
{
    // Get an existing key does not throw
    
    CPPUNIT_ASSERT_NO_THROW(
        m_pLogBook->kvGet("spokesperson")
    );
}
void kvtest::get_3()
{
    // get an existing key returns the right answer
    
    EQ(std::string("Ron Fox"), m_pLogBook->kvGet("spokesperson"));
}

void kvtest::set_1()
{
    // Setting an existing key modified it; does not create a duplicate one:
    
    CPPUNIT_ASSERT_NO_THROW(m_pLogBook->kvSet("experiment", "0400x"));
    EQ(std::string("0400x"), m_pLogBook->kvGet("experiment"));  // Value changed
    
    CSqliteStatement query(
        *m_pConnection,
        "SELECT COUNT(*) FROM kvstore WHERE key='experiment'"
    );
    ++query;
    EQ(1, query.getInt(0));                // Still only one.
}

void kvtest::set_2()
{
    // setting a new key creates it
    
    CPPUNIT_ASSERT_NO_THROW(m_pLogBook->kvSet("newKey", "NewValue"));
    std::string value;
    CPPUNIT_ASSERT_NO_THROW(value = m_pLogBook->kvGet("newKey"));
    EQ(std::string("NewValue"), value);
}
void kvtest::create_1()
{
    // Create for existing key is an error.
    
    CPPUNIT_ASSERT_THROW(
        m_pLogBook->kvCreate("experiment", "0400x"),
        LogBook::Exception
    );
}
void kvtest::create_2()
{
    // Create a new key is ok and does that.
    
    CPPUNIT_ASSERT_NO_THROW(m_pLogBook->kvCreate("newKey", "NewValue"));
    EQ(std::string("NewValue"), m_pLogBook->kvGet("newKey"));
}

void kvtest::list_1() {
    // The initial dtabase shoulid have all the stuff in it.

    std::vector<std::pair<std::string, std::string>> kv = m_pLogBook->kvList();

    EQ(size_t(4), kv.size());
    EQ(std::string("experiment"), kv[0].first);
    EQ(std::string("Test"), kv[0].second);

    EQ(std::string("spokesperson"), kv[1].first);
    EQ(std::string("Ron Fox"), kv[1].second);

    EQ(std::string("purpose"), kv[2].first);
    EQ(std::string("Test kvstore"), kv[2].second);

    EQ(std::string("version"), kv[3].first);
    EQ(std::string("1.0"), kv[3].second);

}
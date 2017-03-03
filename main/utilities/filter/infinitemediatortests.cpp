/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2014.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Jeromy Tompkins
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/


static const char* Copyright = "(C) Copyright Michigan State University 2014, All rights reserved";

#include <URL.h>

#include <CFileDataSource.h>
#include <CFileDataSink.h>
#include <CDataSinkFactory.h>
#include <CDataSourceFactory.h>
#include <V11/CTestFilter.h>

#include <cppunit/extensions/HelperMacros.h>

#define private public
#define protected public
#include "CInfiniteMediator.h"
#undef private
#undef protected


#include <fstream>
#include <ios>
#include <algorithm>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>


using namespace DAQ;

// A test suite 
class CInfiniteMediatorTest : public CppUnit::TestFixture
{

  private:
    V11::CFilter* m_pFilter;
    CDataSource* m_source;
    CDataSink* m_sink;
    CInfiniteMediator* m_mediator;

  public:
    CInfiniteMediatorTest();

    CPPUNIT_TEST_SUITE( CInfiniteMediatorTest );
    CPPUNIT_TEST ( testConstructor );
    CPPUNIT_TEST ( testSetMembers );
    CPPUNIT_TEST_SUITE_END();

  public:
    void setUp();
    void tearDown();

    void testConstructor();

    void testSetMembers();

};


// Register it with the test factory
CPPUNIT_TEST_SUITE_REGISTRATION( CInfiniteMediatorTest );

CInfiniteMediatorTest::CInfiniteMediatorTest()
    : m_pFilter(0),
    m_source(0),
    m_sink(0),
    m_mediator(0)
{}


void CInfiniteMediatorTest::setUp()
{
  std::vector<uint16_t> dummy;
  m_pFilter = new CTestFilter;
  m_source = new CFileDataSource(STDIN_FILENO, dummy);
  m_sink = new CFileDataSink(STDOUT_FILENO);

  m_mediator = new CInfiniteMediator(m_source,m_pFilter,m_sink);
//  m_mediator->setDataSource(m_source);
//  m_mediator->setFilter(m_pFilter);
//  m_mediator->setDataSink(m_sink);
}

void CInfiniteMediatorTest::tearDown()
{
  // Call the destructor to free
  // owned memory
  delete m_mediator; m_mediator=0;
}


void CInfiniteMediatorTest::testConstructor()
{
  CPPUNIT_ASSERT_EQUAL( m_source, m_mediator->m_pSource.get());
  CPPUNIT_ASSERT_EQUAL( m_pFilter, m_mediator->m_pFilter.get());
  CPPUNIT_ASSERT_EQUAL( m_sink, m_mediator->m_pSink.get());
}

void CInfiniteMediatorTest::testSetMembers()
{
    CFilter* new_pFilter = 0;
    CFilter* old_pFilter = m_mediator->setFilter(new_pFilter);
    CPPUNIT_ASSERT_EQUAL( m_pFilter, old_pFilter );
    CPPUNIT_ASSERT_EQUAL( new_pFilter, m_mediator->m_pFilter.get() );

    std::shared_ptr<CDataSource> new_source = nullptr;
    auto old_source = m_mediator->setDataSource(new_source);
    CPPUNIT_ASSERT_EQUAL( m_source,  old_source.get() );
    CPPUNIT_ASSERT_EQUAL( new_source, m_mediator->m_pSource );

    std::shared_ptr<CDataSink> new_sink = nullptr ;
    auto old_sink = m_mediator->setDataSink(new_sink);
    CPPUNIT_ASSERT_EQUAL( m_sink , old_sink.get() );
    CPPUNIT_ASSERT_EQUAL( new_sink , m_mediator->m_pSink );
}


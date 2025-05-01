/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox 
             Facility for Rare Isotope4s
             Michigan State University
             East Lansing, MI 48824-1321


@author Ron Fox <fox at frib dot msu dot edu>
@brief Test the utilities.cpp file functions.
*/
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "utilities.h"
#include "options.h"
#include <string>

class UtilityTests : public CppUnit::TestFixture {
        CPPUNIT_TEST_SUITE(UtilityTests);
        //computeOutFile:
        CPPUNIT_TEST(outfile_1);
        CPPUNIT_TEST(outfile_2);
        CPPUNIT_TEST_SUITE_END();

protected:
    void outfile_1();
    void outfile_2();
public:
    void setUp() {

    }
    void tearDown() {
        
    }

};

CPPUNIT_TEST_SUITE_REGISTRATION(UtilityTests);

/*------------------------- Tests for computeOutFile --------------------------------*/

// default output file:

void
UtilityTests::outfile_1() {
    // craft a parsed args struct with no outfile but an infile:
    gengetopt_args_info args;
    const char* files[] = {"input.tcl"};
    args.inputs = const_cast<char**>(files);
    args.inputs_num = 1;
    args.output_given = 0;

    // should be the input file with the ext repalced with 'yaml'.
    
    std::string outfile = computeOutfile(args);
    CPPUNIT_ASSERT_EQUAL(std::string("input.yaml"), outfile);
}

// Replace output file with one parsed:

void UtilityTests::outfile_2() {
    // craft the gengetopt struct:

    gengetopt_args_info args;
    const char* files[] = {"input.tcl"};
    args.inputs = const_cast<char**>(files);
    args.inputs_num = 1;
    args.output_given = 1;
    args.output_arg = const_cast<char*>("output.yaml");

    std::string outfile = computeOutfile(args);
    CPPUNIT_ASSERT_EQUAL(std::string("output.yaml"), outfile);
}



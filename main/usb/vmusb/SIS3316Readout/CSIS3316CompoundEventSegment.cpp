/**
 * @file CSIS3316CompoundEventSegment.cpp
 * @author Ron Fox <fox at frib dot msu dot edu>
 * @brief Implementation file for the compound event segement that contains CSIS3316EventSegment objects.
 * 
 *  This software is Copyright by the Board of Trustees of Michigan
 *  State University (c) Copyright 2025
*
*  You may use this software under the terms of the GNU public license
*   (GPL).  The terms of this license are described at:
*
*    http://www.gnu.org/licenses/gpl.txt
*
*    Author:
*            Ron Fox
*            Facility for Rare Isotop Beams
*            Michigan State University
*            East Lansing, MI 48824-1321
*
 */
#include "CSIS3316CompoundEventSegment.h"
#include "CSIS3316EventSegment.h"
#include <TCLInterpreter.h>
#include <TCLOBject.h>
#include <typeinfo>

// Implement the event segment

/**
 *  constructor
 *    @param pFilename - name of the configuration file that will be interpreted by initialize.
 */
CSIS3316CompoundEventSegment::CSIS3316CompoundEventSegment(const char* pFilename) :
    m_confrigFile(pFilename)
{}

/** 
 * destructor is a no-op for now:
 */
CSIS3316CompoundEventSegment::~CSIS3316CompoundEventSegment() {}


/**
 *  initialize 
 *    configure the modules and run the base class initializer:alignas
 * 
 */
void
CSIS3316CompoundEventSegment::initialize() {
    configureModule()
    CCompoundEventSegment::initialize();
}

/**
 * AddEventSegment
 *    Ensure the event segment is actually a CSIS3316EventSegment then use the base class:alignas
 * 
 * @param pSegment - event segment to add.
 */
void
CSIS3316CompoundEventSegment::AddSegment(CEventSegment* pSegment) {
    if (dynamic_cast<CSIS3316EventSegment*>(pSegment) == nullptr) {
        throw std::bad_cast();
    }
}

/**
 * configureModules 
 *    Here's where the rubber meets the road.  
 *    - Create a captive interpreter
 *    - Add an instance of CSIS3316Command to it.
 *    - Interpret the configuration file.
 *    - Tear all that stuff down again.
 * 
 */
void
CSIS3316CompoundEventSegment::configureModules() {
    CTCLInterpreter interp;
    auto pCommand = new CSIS3316Command(intepr, this);

    interp.EvalFile(m_configFile);

    delete pCommand;
}

// Implement the configuration command.


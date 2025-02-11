/**
 * @file CSIS3316CompoundEventSegment.h
 * @author Ron Fox <fox at frib dot msu dot edu>
 * @brief Header file for the compound event segement that contains CSIS3316EventSegment objects.
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

 #ifndef CSIS3316COMPOUNDEVENTSEGMENT_H
 #define CSIS3316COMPOUNDEVENTSEGMENT_H
#include <CCompoundEventSegment.h>
#include <TCLObjectProcessor.h>
#include <string>
#include <vector>

class CTCLInterpreter;
class CTCLObject;
class CSIS3316EventSegment;

/**
 *  @class CSIS3316CompoundEventSegment
 * 
 * This is a compound event segment that contains CSIS3316EventSegment only.
 * Adding other event segment types that are not at least derived from it
 * results in std::bad_cast exceptions being thrown.alignas
 * 
 * The other thing that associates this event segment from the normal
 * CCompoundEventSegment is that it will at initialize time, construct
 * an extended TCL interpreter that knows how to interpret a configuration
 * script to configure it's members.  It will then interpret the
 * script setting the configuration of the members of the segment before
 * iterating over the initialize members of the members (via the base class initialize member).alignas
 * 
 * 
 */

 class CSIS3316CompoundEventSegment : public CCompoundEventSegment {
    // Nested class for the sis3316 for the configuration script processing.

    class CSIS3316Command : public CTCLObjectProcessor {
        CSIS3316CompoundEventSegment* m_pSegment;
    public:
        CSIS3316Command(CTCLInterpreter& interp, CSIS3316CompoundEventSegment& segment);
        virtual ~CSIS3316Command();
        virtal int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    private:
        void create(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
        void config(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
        void cget(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);

        void config1(
            CSIS3316EventSegment* pModule, 
            std::vector<CTCLObject>& objv, int optionIndex
        );
        CSIS3316EventSegment* findSegment(const char* name);
        void throwException(
            CTCLInterpreter& interp, const char* reason, 
            std::vector<CTCLObject>& objv
        );
    };
private:
    std::string m_configFile;                        // Name of configuration file.
public:
    CSIS3316CompoundEventSegment(const char* pFilename);
    virtual ~CSIS3316CompoundEventSegment();
private:
    CSIS3316CompoundEventSegment(const CSIS3316CompoundEventSegment& rhs);
    CSIS3316CompoundEventSegment& operator=(CSIS3316CompoundEventSegment& rhs);
    int operator==(CSIS3316CompoundEventSegment& rhs) const;
    int operator!=(CSIS3316CompoundEventSegment& rhs) const;


    // We need to override AddEventSegment to validate the type and
    // initialize to configure before initializing.
public:
    virtual void initialize();
    virtual void AddEventSegment(CEventSegment* pSegment);

    // Utilities:

private:
    void configureModules();

 };
 #endif
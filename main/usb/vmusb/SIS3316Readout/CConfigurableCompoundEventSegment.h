/**
 * @file CConfigurableCompoundEventSegment.h
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

 #ifndef CCONFIGURABLECOMPOUNDEVENTSEGMENT_H
 #define CCONFIGURABLECOMPOUNDEVENTSEGMENT_H
#include <CCompoundEventSegment.h>
#include <TCLObjectProcessor.h>
#include <string>
#include <vector>

class CTCLInterpreter;
class CTCLObject;
class CSIS3316EventSegment;

/**
 *  @class CConfigurableCompoundEventSegment
 * 
 * This is a compound event segment that is Tcl configurable.
 * 
 * The other thing that associates this event segment from the normal
 * CCompoundEventSegment is that it will at initialize time, construct
 * an extended TCL interpreter that knows how to interpret a configuration
 * script to configure it's members.  It will then interpret the
 * script setting the configuration of the members of the segment before
 * iterating over the initialize members of the members (via the base class initialize member).alignas
 * 
 * @todo   
 *    The various command processors could actually be derived from a base class
 * with a virtual 'validate' method to determine if the event segment located
 * was of the right type... since as it is there is a defect it is possible
 * to create two identically named event segments as long as they are of different types.
 * This is actually properly handled with the dynamic casts being doin in findSegment.
 * All this would get us closer to DRY.
 */

 class CConfigurableCompoundEventSegment : public CCompoundEventSegment {
    // Nested class for the sis3316 for the configuration script processing.

    class CSIS3316Command : public CTCLObjectProcessor {
        CConfigurableCompoundEventSegment* m_pSegment;
    public:
        CSIS3316Command(CTCLInterpreter& interp, CConfigurableCompoundEventSegment& segment);
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
    // CAEN pattern register.
    class Cv977Command : public CTCLObjectProcessor {
        CConfigurableCompoundEventSegment* m_pSegment;
    public:
        Cv977Command(CTCLInterpreter& interp, CConfigurableCompoundEventSegment& segment);
        virtual ~Cv977Command();
        virtal int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
    private:
        void create(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
        void config(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
        void cget(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);

        void config1(
            CV977EventSegment* pModule, 
            std::vector<CTCLObject>& objv, int optionIndex
        );
        CV977EventSegment* findSegment(const char* name);
        void throwException(
            CTCLInterpreter& interp, const char* reason, 
            std::vector<CTCLObject>& objv
        );
    }; 
   
    // Add more classes here for specific device support e.g. SIS scaler,
    
    
private:
    std::string m_configFile;                        // Name of configuration file.
public:
    CConfigurableCompoundEventSegment(const char* pFilename);
    virtual ~CConfigurableCompoundEventSegment();
private:
    CConfigurableCompoundEventSegment(const CConfigurableCompoundEventSegment& rhs);
    CConfigurableCompoundEventSegment& operator=(CConfigurableCompoundEventSegment& rhs);
    int operator==(CConfigurableCompoundEventSegment& rhs) const;
    int operator!=(CConfigurableCompoundEventSegment& rhs) const;


    // We need to override AddEventSegment to validate the type and
    // initialize to configure before initializing.
public:
    virtual void initialize();

    // Utilities:

private:
    void configureModules();

 };
 #endif
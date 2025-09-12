/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2025.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox 
             Facility for Rare Isotope4s
             Michigan State University
             East Lansing, MI 48824-1321


@author Ron Fox <fox at frib dot msu dot edu>
@brief Main driver for turning VMUSBReadout daqconfig scripts -> mvlc fribdaq-readout .yaml configs
*/
#ifndef MVLC_GENERATOR_H
#define MVLC_GENERATOR_H
#include <string>
#include <yaml-cpp/yaml.h>
class TCLConfigParser;
class CStack;
class VMUSBCommand;
class VMUSBListCommand;

/**
 * @class MVLCGenereate
 *    This class takes a parsed VMUSB configuration file (parsed by a TclConfigParser) 
 * and genrates, as output, the YAML that can drive the mvlc readout I created,
 * fribdaq-readout, in the mesytec mvlc repository.  It makes use of a template
 * configuration file that is pointed to by the preprocessor definition
 * MVLC_TEMPLATE - a complete path to the file.
 * 
 * event0 is assumed to be the event readout stack and is triggered on NIM1.
 * event1 is assumed to be a scaler stack and is triggered every t2 seconds.
 * 
 * This class edits that configuration as follows:
 *  -    mcst_daq_start/event0.DAQ Start contents is filled in with the contents
 * of the event stack's Initialize operations.
 *  - crate/readout_stacks/name: event0/groups/name: readout is filled in with
 * the contents of the addRedoutList operations.
 *  - create/readout_stacks/name event0/name readout_end is filled with the contents of the
 *   onEndRun operations.
 * 
 * Similarly for event1 but from the scaler stack.
 * 
 * The result is then output to the configuration.
 *  
 * @note  the TclConfigParser passed into the constructor must have a lifetime
 *  at least until atfer generate is called.
 */
class MVLCGenerate {
private:
    std::string      m_outfile;            // Name of output file.
    TCLConfigParser* m_VMUSBConfig;        // parsed VMUSB configuration file.
    static const char* m_YamlTemplate;
    VMUSBCommand*      m_pVMUSBCommand;
    VMUSBListCommand*  m_pVMUSBListCommand;
public:
    MVLCGenerate(std::string outfile, TCLConfigParser* config);
    virtual ~MVLCGenerate();
private:
    MVLCGenerate(const MVLCGenerate&);
    MVLCGenerate& operator=(const MVLCGenerate&);
    int operator==(const MVLCGenerate&) const;
    int operator!=(const MVLCGenerate) const;

public:
    static void setTemplate(const char* templatePath);
    void generate();             // Generate/write the config file.
  
    // utilities:
private:
    
    YAML::Node loadTemplate();
    void fillReadoutStack(YAML::Node& doc, const char* name, CStack& stack);
    void fillInitStack(YAML::Node& doc, const char* name, CStack& stack);
    void fillEndStack(YAML::Node& doc, const char* name, CStack& stack);
    void createRdoIfNeeded(YAML::Node& stacks,  const char* stackname);
    void createInitIfNeeded(YAML::Node& stacks, const char* stackname);
};

#endif
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
#include "MVLCGenerator.h"
#include "TCLConfigParser.h"
#include "CStack.h"
#include <iostream>
#include <sstream>
#include <string.h>
#include <fstream>





const char* MVLCGenerate::m_YamlTemplate=MVLC_TEMPLATE;

static const char* stackDelay = "STACKDELAY";
/** 
 * constructor
 *    Construct the generator:alignas
 * @param outfile - output file we will generate to.
 * @param config  - parsed configuration.
 * 
 * The configuration parser must be in scope through the call to generate.
 */

MVLCGenerate::MVLCGenerate(std::string outfile, TCLConfigParser* config) :
    m_outfile(outfile), m_VMUSBConfig(config) {}

/**
 *  destructor
 */
MVLCGenerate::~MVLCGenerate() {}


/**
 *  generate
 *     Generate the yaml output file from the template in m_YamlTemplate using
 * the configuration in m_VMUSBConfig writing the output file m_outfile.
 */
void
MVLCGenerate::generate() {
    auto yaml = loadTemplate();
    setStackDelay(yaml);


    std::ofstream out(m_outfile);
    
    out << yaml;

}




/////////////////// private utilities 


/**
 *  loadTemplate
 *     Load the template file 
 * @return YAML::node - the loaded template.
 *    Throws on error?
 */
YAML::Node
MVLCGenerate::loadTemplate() {
    return YAML::LoadFile(m_YamlTemplate);
}
/**
 * setStackDelay
 * 
 * Pull the event stack delay from the configuration and substitute it for the value in the vme_write that
 * sets it in the trigger configuration.
 * This is somewhere in the soup of vme_write operations in crate/init_trigger_io/groups/name ""/contents.
 * In this both groups and contents are arrays.  The VME write we need to modify has its data set to be
 * "STACKDELAY"
 * 
 * @param doc - references the YAML we're building.
 * 
 */
void
MVLCGenerate::setStackDelay(YAML::Node& doc) {
    // If there's an event stack it's -delay option is used, if not 0.

    unsigned delay = 0;
    auto event = m_VMUSBConfig->getEventStack();
    if(event) {
        delay = event->getDelay();
    }
    
    // Now find the thing to modify:

    auto trigger = doc["crate"]["init_trigger_io"];
    auto groups = trigger["groups"];
    for (unsigned i = 0; i < groups.size(); i++) {
        auto name = groups[i]["name"];
        if (name && name.as<std::string>() == "") {
            auto contents = groups[i]["contents"];
            
            // Need to find the one that has a vlue of 
            // STACKDELAY and fix it up with delay.

            for (unsigned w = 0; w < contents.size(); w++) {
                std::string writeCmd = contents[w].as<std::string>();
                size_t sloc = writeCmd.find(stackDelay);
                if (sloc != writeCmd.npos) {
                    writeCmd.replace(
                        sloc, strlen(stackDelay), std::to_string(delay)
                    );
                    contents[w] = writeCmd;
                }
            }
        }
    }
    
}
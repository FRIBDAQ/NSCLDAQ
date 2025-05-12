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
#include <CVMUSB.h>
#include <CVMUSBReadoutList.h>





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

    // The readout stacks:
    auto eventStack = m_VMUSBConfig->getEventStack();
    auto scalerStack = m_VMUSBConfig->getScalerStack();
    if (eventStack) fillReadoutStack(yaml, "event0", *eventStack);
    if (scalerStack) fillReadoutStack(yaml, "event1", *scalerStack);

    // Initialization:

    if (eventStack) fillInitStack(yaml, "event0.init", *eventStack);
    if (scalerStack) fillInitStack(yaml, "event1.init", *scalerStack);

    // end of run

    if (eventStack) fillEndStack(yaml, "event0.stop", *eventStack);
    if (scalerStack) fillEndStack(yaml, "event1.stop", *scalerStack);

    // Generate the output file:  
    std::ofstream out(m_outfile);
    out << yaml << std::endl;

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
 * @param[inout] doc - references the YAML we're building.
 *
 *  Note the units of -delay are us while those in the MVLC are ns.
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
                        sloc, strlen(stackDelay), std::to_string(10000*delay)
                    );
                    contents[w] = writeCmd;
                }
            }
        }
    }
    
}
/**
 *  fillReadoutStack
 *     Fills in the readout stack contents for either the scaler or event stacks:
 * 
 * @param[inout] doc - the YAML document to be edited.
 * @param name - Namne of the stack: event0 for science and event1 for scaler.
 * @param stack - references the config stack to fill in.
 */
void 
MVLCGenerate::fillReadoutStack(YAML::Node& doc, const char* name,  CStack& stack)  {
    // generate the vector of operations:

    CVMUSBReadoutList list;
    stack.addReadoutList(list);
    auto ops = list.dumpForMvlc();      // Ops is a vector of operations lines.

    // locate what we fill in:

    auto stacks = doc["crate"]["readout_stacks"];
    // Need to find the correct stack:

    for (int i =0; i < stacks.size(); i++) {
        auto sname = stacks[i]["name"];
        if (sname.as<std::string>() == name) {
            // in that stack we need to filli n the readout list:

            auto groups = stacks[i]["groups"];
            for (int g =0; g < groups.size(); g++) {
                if (groups[g]["name"].as<std::string>() == "readout") {
                    auto contents = groups[g]["contents"];
                    for (auto line : ops) {
                        contents.push_back(line);
                    }
                }
            }
        }
    }
}
/**
 * fillInitStack
 *     Locates the initialization stacks and fills them in.alignas
 * @param doc - refernces the yaml document we're modifying.
 * @param name - name of the stack: event0.init  event1.init for the event and scaler respectively.
 */
void
MVLCGenerate::fillInitStack(YAML::Node& doc, const char* name, CStack& stack) {
    // Generate the operations:

    CVMUSB controller; 
    stack.Initialize(controller);
    auto ops = controller.getRecordedOperations();

    // Locate the stack we fill in:

    auto stacks = doc["crate"]["init_commands"]["groups"];
    for (int g = 0; g < stacks.size(); g++) {
        if (stacks[g]["name"].as<std::string>() == name) {
            auto stack = stacks[g]["contents"];
            for (auto line: ops) {
                stack.push_back(line);
            }
        }
    }
}
/** fillEndStack
 * 
 *    Fill in any run end operations for a stack.\
 * 
 * @param doc - the YAML document that's being filled in.
 * @param name - Name of the stack  event0.stop or event1.stop for event and scaler respenctively.a
 * @param stack - references the appropriate stack to creat the operations.
 * 
 */
void
MVLCGenerate::fillEndStack(YAML::Node& doc, const char* name, CStack& stack) {
    // Generate the ops:

    CVMUSB controller;
    stack.onEndRun(controller);
    auto ops = controller.getRecordedOperations();

    // Find and fill in the appropriate stack:

    auto stacks = doc["create"]["stop_commands"]["groups"];
    for (int g = 0; g < stacks.size(); g++) {
        auto sname = stacks[g]["name"];
        if (sname.as<std::string>() == name) {
            auto stack = stacks[g]["contents"];
            for (auto op : ops) {
                stack.push_back(op);
            }
        }
    }
}

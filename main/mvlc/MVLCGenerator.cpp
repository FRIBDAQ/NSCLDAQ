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
#include "VMUSBCommand.h"
#include "VMUSBListCommand.h"





const char* MVLCGenerate::m_YamlTemplate=MVLC_TEMPLATE;


/** 
 * constructor
 *    Construct the generator:alignas
 * @param outfile - output file we will generate to.
 * @param config  - parsed configuration.
 * 
 * The configuration parser must be in scope through the call to generate.
 */

MVLCGenerate::MVLCGenerate(std::string outfile, TCLConfigParser* config) :
    m_outfile(outfile), m_VMUSBConfig(config), m_pVMUSBCommand(0), m_pVMUSBListCommand(0) {
        m_pVMUSBCommand = new VMUSBCommand(*m_VMUSBConfig->getInterpreter());
        m_pVMUSBListCommand = new VMUSBListCommand(*m_VMUSBConfig->getInterpreter());
    }

/**
 *  destructor
 */
MVLCGenerate::~MVLCGenerate() {
    delete m_pVMUSBCommand;
}
/**
 * setTemplate
 *    Override the default template file 
 * 
 * @param path - template file path  This must remain in scope for the lifetime of this
 * object.
 */
void
MVLCGenerate::setTemplate(const char* templatePath) {
    m_YamlTemplate = templatePath;
}

/**
 *  generate
 *     Generate the yaml output file from the template in m_YamlTemplate using
 * the configuration in m_VMUSBConfig writing the output file m_outfile.
 */
void
MVLCGenerate::generate() {
    auto yaml = loadTemplate();


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
    m_pVMUSBListCommand->setList(list);
    stack.addReadoutList(list);
    auto ops = list.dumpForMvlc();      // Ops is a vector of operations lines.

    // locate what we fill in:

    auto stacks = doc["crate"]["readout_stacks"];
    createRdoIfNeeded(stacks, name);

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
    m_pVMUSBCommand->setController(controller);
    stack.Initialize(controller);
    auto ops = controller.getRecordedOperations();

    // Locate the stack we fill in:

    auto stacks = doc["crate"]["init_commands"]["groups"];
    createInitIfNeeded(stacks, name);
    for (int g = 0; g < stacks.size(); g++) {
        if (stacks[g]["name"].as<std::string>() == name) {
            auto stack = stacks[g]["contents"];
            for (auto line: ops) {
                stack.push_back(line);
            }
        }
    }
    m_pVMUSBCommand->clearController();
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
    m_pVMUSBCommand->setController(controller);
    stack.onEndRun(controller);
    auto ops = controller.getRecordedOperations();

    // Find and fill in the appropriate stack:

    auto stacks = doc["crate"]["stop_commands"]["groups"];
    createStopIfNeeded(stacks, name);
    for (int g = 0; g < stacks.size(); g++) {
        auto sname = stacks[g]["name"];
        if (sname.as<std::string>() == name) {
            auto stack = stacks[g]["contents"];
            for (auto op : ops) {
                stack.push_back(op);
            }
        }
    }
    m_pVMUSBCommand->clearController();
}
/**
 * Given the readout_stacks node, if the given stack isn't in that,
 * generate its skeleton.
 * 
 * @param doc - [crate][readout_stacks] node
 * @param stackname -name of the stack to create ifneeded.
 * 
 * The structure we create is:
 * 
 * - name: {stackname}
 *   groups:
 *    -name: readout
 *     contents: []
 *    -name: readout_end
 *     contents: []
 * 
 */
void
MVLCGenerate::createRdoIfNeeded(YAML::Node& stacks, const char* stackname) {
    // if stacks already has stackname, do nothing:

    for (int s = 0; s < stacks.size(); s++) {
        auto sname = stacks[s]["name"];
        if (sname.as<std::string>() == stackname) {
            return;                     // Already have it, assume it's complete.
        }
    }
    // Have to add it:

    std::stringstream nodeText;
    nodeText << "name: " << stackname << std::endl;
    nodeText << "groups:\n";
    nodeText << "  - name: readout\n";
    nodeText << "    contents:\n";
    nodeText << "  - name: readout_end\n";
    nodeText << "    contents:\n";
    std::string nodestring = nodeText.str();

    YAML::Node stack = YAML::Load(nodestring);
    stacks.push_back(stack);

}
/**
 * createInitIfNeeded
 *     If it does not yet exist, create an empty init entry for a stack.
 * 
 * @param node the ["crate"]["init_commands"]["groups"] node
 * @param stackname - name of the stack to create.
 * 
 */
void
MVLCGenerate::createInitIfNeeded(YAML::Node& node, const char* stackname) {
    // If the stack already exists, assume its complete:

    for (int i = 0; i < node.size(); i++) {
        if (node[i]["name"].as<std::string>() == stackname) {
            return;                 // already exists.
        }
    }

    // the YAML we want to add to node is:

    std::stringstream yamlstream;
    yamlstream << "name: " << stackname << std::endl;
    yamlstream << "contents:\n";

    std::string yamlstring = yamlstream.str();
    YAML::Node yaml = YAML::Load(yamlstring);
    node.push_back(yaml);
}
/**
 *  createStopIfNeeded
 *    If it does not exist yet, create an empty stop stack.
 * 
 * @param node the [crate][stop_commands][groups] node.
 * @param stackname - name of the stack to generate.
 */
void
MVLCGenerate::createStopIfNeeded(YAML::Node& node, const char* stackname) {
    // If the stack is already there assume its template is complete and 
    // return.

    for (int i =0; i < node.size(); i++) {
        if (node[i]["name"].as<std::string>() == stackname) {
            return;
        }
    }
    // Generate this teamplate:

    std::stringstream yamlstream;
    yamlstream << "name: " << stackname << std::endl;
    yamlstream << "contents:\n";

    std::string yamlstring = yamlstream.str();
    YAML::Node yaml = YAML::Load(yamlstring);

    node.push_back(yaml);
}

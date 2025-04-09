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
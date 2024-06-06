/** 
 * @file CMyScaler.cpp
 * @brief Implement the DDAS scaler class.
 */

#include "CMyScaler.h"

#include <math.h>
#include <string.h>

#include <iostream>
#include <fstream> 
#include <string>

#include <config.h>
#include <config_pixie16api.h>
#include <CXIAException.h>

CMyScaler::CMyScaler(unsigned short mod, unsigned short crate) :
    m_module(mod), m_crate(crate)
{
    for (int i = 0; i < 16; i++) {
	m_prevIC[i] = 0;
	m_prevOC[i] = 0;
    }
    clearCounters(m_statistics.s_cumulative);
    clearCounters(m_statistics.s_perRun);
  
    std::cout << "Scalers know crate ID = " << m_crate << std::endl;
}

CMyScaler::~CMyScaler()
{}

void
CMyScaler::initialize() 
{
    for (int i = 0; i < 16; i++) {
	m_prevIC[i] = 0;
	m_prevOC[i] = 0;
    }
    clearCounters(m_statistics.s_perRun); // New run.
}

/**
 * @details
 * Now we need to calculate the # of events from the last read of the scalers.
 * NSCL scaler buffers just expect the # events since the last read. However, 
 * Pixie-16 statistics cannot be cleared, so we need to do some math and store 
 * the counts from our previous read.
 * 
 * Input counts (IC) and rate (ICR) are fast triggers. Output counts (OC) and 
 * rate (OCR) are accepted triggers.
 */
std::vector<uint32_t>
CMyScaler::read()
{
    try {
	std::vector<unsigned int> statistics(Pixie16GetStatisticsSize(),0);  
	int retval = Pixie16ReadStatisticsFromModule(
	    statistics.data(), m_module
	    );
	
	if (retval < 0) {
	    std::string msg("Error accessing scalar statistics from module ");
	    msg += m_module;
	    throw CXIAException(msg, "Pixie16ReadStatisticsFromModule", retval);
	}    

  	double inputCounts[16] = {0};
	double outputCounts[16] = {0};
	unsigned long scalerData[33] = {0};  
	scalerData[0] = (unsigned long)m_crate;
	
	for (int i = 0; i < 16; i++) {
	    // Raw input counts (number of fast triggers seen by FPGA):
	    inputCounts[i] = Pixie16ComputeRawInputCount(
		statistics.data(), m_module, i
		);
	    
	    // Raw output counts (validated events handled by DSP,
	    // "live" counts):
	    outputCounts[i] = Pixie16ComputeRawOutputCount(
		statistics.data(), m_module, i
		);
	    
	    // Finally compute the events since the last scaler read:
	    scalerData[(2*i + 1)]
		= (unsigned long)(inputCounts[i] - m_prevIC[i]);
	    scalerData[(2*i + 2)]
		= (unsigned long)(outputCounts[i] - m_prevOC[i]);

	    // Reset counters since last read:
	    m_prevIC[i] = inputCounts[i];
	    m_prevOC[i] = outputCounts[i];      
	}

	// Copy scaler information into the output vector
	m_scalers.clear();
	m_scalers.insert(m_scalers.end(), scalerData, scalerData + 33);
  
	// Figure out the statistics by summing over the scalerData (it's
	// incremental so we can just add it all in). Channel data come in
	// pairs starting at 1:
  
	int idx = 1; 
	for (int i = 0; i < 16; i++) {
	    m_statistics.s_cumulative.s_nTriggers += scalerData[idx];
	    m_statistics.s_perRun.s_nTriggers     += scalerData[idx];
    
	    m_statistics.s_cumulative.s_nAcceptedTriggers += scalerData[idx+1];
	    m_statistics.s_perRun.s_nAcceptedTriggers     += scalerData[idx+1];
    
	    idx += 2;
	}

	return m_scalers;
	
    } catch (const CXIAException& e) {
	std::cerr << e.ReasonText() << std::endl;	
	return m_scalers;
    } catch(...) {
	std::cerr << "Unexpected exception encountered in CMyScaler::read!\n";
	return m_scalers;
    }
}

void
CMyScaler::clearCounters(Counters& c)
{
    memset(&c, 0, sizeof(Counters));
}

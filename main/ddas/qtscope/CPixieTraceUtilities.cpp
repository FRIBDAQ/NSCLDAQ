/**
 * @file  CPixieTraceUtilities.cpp
 * @brief Implementation of the trace utilities class.
 */

#include "CPixieTraceUtilities.h"

#include <iostream>
#include <sstream>
#include <algorithm>
#include <cmath>

#include <config.h>
#include <config_pixie16api.h>
#include <CXIAException.h>

#include "CDataGenerator.h"

/**
 * @details
 * The CPixieTraceUtilities class has ownership of a CDataGenerator object 
 * and is responsible for managing it.
 */
CPixieTraceUtilities::CPixieTraceUtilities() :
    m_useGenerator(false),
    m_validAmplitude(20)
{}

/**
 * @details
 * Traces are "validated" (_not_ triggered!) if:
 * 1. The max/min value exceeds the median value +/- 10*SD where SD is the 
 *    standard deviation estimated from the median absolute deviation.
 * 2. The max/min value differs from the median value by at least 
 *    m_validAmplitude ADC units.
 * @todo (ASC 6/14/24): Improved baseline estimation for validation. 
 */ 
int
CPixieTraceUtilities::ReadTrace(int module, int channel)
{
    int maxAttempts = 100;  // Reacquire attempts.
    bool goodTrace = false; // Trace meets validation requirements.

    int attempt = 0;  
    while ((goodTrace == false) && (attempt < maxAttempts)) {
	try {
	    AcquireADCTrace(module, channel);
    
	    // Check for good trace (signal likely present) and validate.
	    // Median is more robust measure of baseline than mean for signals
	    // with long decay time e.g. HPGe detectors, Si.      
	    double median = GetMedianValue(m_trace);
	    std::vector<double> traceMAD; // To hold the med. abs. dev. values.
	    for (const auto &ele : m_trace) {
		traceMAD.push_back(std::abs(ele-median));
	    }
	    // Unbiased estimator of Gaussian SD.
	    double sigma = 1.4826*GetMedianValue(traceMAD); 

	    // iterators
	    auto max = std::max_element(m_trace.begin(), m_trace.end());
	    auto min = std::min_element(m_trace.begin(), m_trace.end());
    
	    // 10 standard deviations ought to do it for a good signal. Check
	    // negative as well in case the signal polarity is wrong.      
	    if (
		(*max > median + 10.0*sigma) || (*min < median - 10.0*sigma)
		) {
		if (
		    ((*max - median) > m_validAmplitude)
		    || (std::abs(*min-median) > m_validAmplitude)
		    ) { // Some (small) minimum amplitude.
		    goodTrace = true;
		}
	    }
      
	    // Try again
	    attempt++;
	}
	catch (const CXIAException& e) {
	    std::cerr << e.ReasonText() << std::endl;      
	    return -1;
	}
	catch (const std::invalid_argument& e) {
	    std::cerr << e.what() << std::endl;      
	    return -2;
	}
    }
  
    return 0;
}

/**
 * @details
 * Read an ADC trace without signal validation.
 */
int
CPixieTraceUtilities::ReadFastTrace(int module, int channel)
{
    try {
	AcquireADCTrace(module, channel);
    }
    catch (const CXIAException& e) {
	std::cerr << e.ReasonText() << std::endl;    
	return -1; 
    }
  
    return 0;
}

///
// Private methods
//

/**
 * @details
 * This function is used internally by the public-facing class members to 
 * manage the internal trace storage, acquire, and read out single channel
 * ADC traces from the module. All exceptions are raised to the caller.
 */
void
CPixieTraceUtilities::AcquireADCTrace(int module, int channel)
{
    // Fill internal DSP memory prior to trace read:
    int retval = Pixie16AcquireADCTrace(module);
    
    if (retval < 0) {
	std::stringstream msg;
	msg << "Failed to allocate memory for trace in module " << module;
	throw CXIAException(msg.str(), "Pixie16AcquireADCTrace()", retval);
    }

    if (!m_useGenerator) {
	unsigned int len;
	PixieGetTraceLength(module, channel, &len);
	ResetTrace(len);

	retval = Pixie16ReadSglChanADCTrace(
	    m_trace.data(), len, module, channel
	    );
	    
	if (retval < 0) {
	    std::stringstream msg;
	    msg << "Failed to read trace from module " << module;
	    throw CXIAException(
		msg.str(), "PixieReadSglChanADCTrace()", retval
		);
	}
    } else {
	std::cerr << "Offline data generation using the generator is not"
		  << " supported for XIA API 3+" << std::endl;
    }
}

/**
 * @details
 * By default the trace length is 8192 samples. This function will calculate 
 * the median value for any trace length, whether or not the number of samples
 * is even or odd. All exceptions are raised to the caller.
 */
template<typename T> double
CPixieTraceUtilities::GetMedianValue(std::vector<T> v)
{  
    if (v.empty()) {
	std::stringstream errmsg;
	errmsg << "CPixieTraceUtilities::GetMedianValue() failed";
	errmsg << " to calculate the median value: the trace is empty";
	errmsg << " and the median is undefined";
	throw std::invalid_argument(errmsg.str());
    }
  
    const auto midItr = v.begin() + v.size()/2;
    std::nth_element(v.begin(), midItr, v.end());
  
    if ((v.size() % 2) == 0) { // Even number of samples (default 8192).
	const auto leftItr = std::max_element(v.begin(), midItr);
	return 0.5*(*leftItr + *midItr);
    } else { // Odd number of samples, just in case someone changes it.
	return (double)(*midItr);
    }
}

/**
 * @details
 * Resize the trace storage if necessary based on the length of the data to be
 * read. Reset stored trace values to 0.
 */
void
CPixieTraceUtilities::ResetTrace(unsigned int len)
{
    if (m_trace.size() != len) {
	m_trace.resize(len);
    }
    std::fill(m_trace.begin(), m_trace.end(), 0);
}

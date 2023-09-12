/**
 * @file CPixieRunUtilities.cpp
 * @brief Implementation of the run utilities class.
*/

#include "CPixieRunUtilities.h"

#include <iostream>
#include <sstream>
#include <thread>
#include <chrono>
#include <vector>
#include <algorithm>

#include <config.h>
#include <config_pixie16api.h>

#include "CDataGenerator.h"

/**
 * @details
 * The CPixieRunUtilities class has ownership of a CDataGenerator object and is 
 * responsible for managing it.
 */
CPixieRunUtilities::CPixieRunUtilities() :
    m_histogram(MAX_HISTOGRAM_LENGTH, 0),
    m_baseline(MAX_HISTOGRAM_LENGTH, 0),
    m_baselineHistograms(
	16, std::vector<unsigned int>(MAX_HISTOGRAM_LENGTH, 0)
	),
    m_genHistograms(
	16, std::vector<unsigned int>(MAX_HISTOGRAM_LENGTH, 0)
	),
    m_runActive(false),
    m_useGenerator(false),
    m_pGenerator(new CDataGenerator)
{}

/**
 * @details
 * Delete the CDataGenerator object owned by this class.
 */
CPixieRunUtilities::~CPixieRunUtilities()
{
    delete m_pGenerator;
}

/**
 * @todo Disable multiple modules from running in non-sync mode.
 */
int
CPixieRunUtilities::BeginHistogramRun(int module)
{
    // Reset internal histogram data:  
    std::fill(m_histogram.begin(), m_histogram.end(), 0);
    for (auto& v : m_genHistograms) {
	std::fill(v.begin(), v.end(), 0);
    }  
  
    // Set the "infinite" run time of 99999 seconds:  
    std::string paramName = "HOST_RT_PRESET";
    int retval = Pixie16WriteSglModPar(
	paramName.c_str(), Decimal2IEEEFloating(99999), module
	);
  
    if (retval < 0) {
	std::cerr << "Run time not properly set. CPixieRunUtilities::BeginHistogramRun() failed to write parameter: " << paramName << " to module " << module  << " with retval " << retval << std::endl;    
	return retval;
    }

    // If the run time is properly set, begin a histogram run for this module
    // turn off synchronization (0):  
    paramName = "SYNCH_WAIT";
    retval = Pixie16WriteSglModPar(paramName.c_str(), 0, module);
  
    if (retval < 0) {
	std::cerr << "CPixieRunUtilities::BeginHistogramRun() failed to disable " << paramName << " in module " << module << " with retval " << retval << std::endl;
	return retval;    
    }

    // Begin the run:  
    retval = Pixie16StartHistogramRun(module, NEW_RUN);
  
    if (retval < 0) {
	std::cerr << "CPixieRunUtilities::BeginHistogramRun() failed to start run module " << module << " with retval " << retval << std::endl;
    } else {
	std::cout << "Beginning histogram run in Mod. " << module << std::endl;
	m_runActive = true;
    }
  
    return retval;
}

/**
 * @details
 * If the run cannot be ended on the first attempt, retry 10 times before 
 * reporting that the run could not be ended properly. Generally speaking, this
 * is caused when one or more channels has a very high trigger rate.
 */
int
CPixieRunUtilities::EndHistogramRun(int module)
{
    int retval = Pixie16EndRun(module);
  
    if (retval < 0) {
	std::cerr << "CPixieRunUtilities::EndHistogramRun() failed to communicate end run operation to module " << module << " with retval " << retval << std::endl;
    }

    bool runEnded = false;
    int nRetries = 0;
    const int maxRetries = 10;
    while ((runEnded == false) && (nRetries < maxRetries)) {
	retval = Pixie16CheckRunStatus(module);    
	if (retval < 0) {
	    std::cerr << "CPixieRunUtilities::EndHistogramRun() failed to get current run status in module " << module << " with retval " << retval << std::endl;
	}    
	runEnded = (retval == 0); // True if run ended.
	nRetries++;    
	// Wait before checking again:    
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  
    if (nRetries == maxRetries) {
	std::cout << "CPixieRunUtilities::EndHistogramRun() failed to end run in module " << module << std::endl;
    } else if (runEnded) {
	std::cout << "Ended histogram run in Mod. " << module << std::endl;
	m_runActive = false;
    }
  
    return 0;
}

/**
 * @details
 * Histogram data comes either from the module itself if running in online
 * mode or from the data generator.
 */
int
CPixieRunUtilities::ReadHistogram(int module, int channel)
{
    // Allocate data structure for histogram and grab it or use the generator:
  
    int retval = -1;
  
    if (!m_useGenerator) {
	retval = Pixie16ReadHistogramFromModule(
	    m_histogram.data(), MAX_HISTOGRAM_LENGTH, module, channel
	    );
    } else {
	retval = m_pGenerator->GetHistogramData(
	    m_genHistograms[channel].data(), MAX_HISTOGRAM_LENGTH
	    );
	m_histogram = m_genHistograms[channel];
    }
  
    if (retval < 0) {
	std::cerr << "CPixieRunUtilities::ReadHistogram() failed to read histogram from module " << module << " channel " << channel << " with retval " << retval << std::endl;
    }

    return retval;
}

/**
 * @details
 * Baseline acquisition is not a "run" in the same sense that histogram runs
 * or list mode data taking is a "run" to the API (no begin/end functions, 
 * no run status change). However, in order for a user to accumulate enough 
 * baseline statistics to make judgements about e.g. manually setting 
 * baseline cuts, it needs to be treated as such in our manager. The active
 * run flag is set to true when taking a baseline "run."
 *
 * The baseline data itself is stored internally as a histogram of values in 
 * [0, MAX_HISTOGRAM_LENGTH). This data structure is reset on begin.
 */
int
CPixieRunUtilities::BeginBaselineRun(int module)
{
    std::cout << "Beginning baseline run in Mod. " << module << std::endl;
    // Clear data vectors and set run active:  
    for (auto& v : m_baselineHistograms) {
	std::fill(v.begin(), v.end(), 0);
    }  
    std::fill(m_baseline.begin(), m_baseline.end(), 0);  
    m_runActive = true;
  
    return 0;
}

/**
 * @details
 * Really all we need to do here is set the active run flag to false.
 */
int
CPixieRunUtilities::EndBaselineRun(int module)
{
    m_runActive = false;
    std::cout << "Ended baseline run in Mod. " << module << std::endl;    
    return 0;
}

/**
 * @details
 * Acquire baseline values for all channels on a module using 
 * Pixie16AcquireBaselines() and update the internal storage for baseline data.
 * The single channel baseline data we want, specified by the input channel 
 * parameter, is copied into a local variable which is accessible via a getter 
 * function.
 * 
 * @todo (ASC 7/14/23): Why not just have the getter take a channel as an 
 * input parameter and return the correct baseline data. It seems unnecessary 
 * to maintain a separate copy.
 */
int
CPixieRunUtilities::ReadBaseline(int module, int channel)
{  
    // Fill internal DSP memory prior to trace read:  
    int retval = Pixie16AcquireBaselines(module);
  
    if (retval < 0) {
	std::cerr << "CPixieRunUtilities::ReadBaseline() failed to allocate memory for trace in module " << module << " with retval " << retval << std::endl;
	return -1;
    }

    // Baseline data is an array of baseline values, not a histogram. To treat
    // this like a run, make cumulative histogram of read values:  
    try {
	UpdateBaselineHistograms(module);
    }
    catch (std::runtime_error& e) {
	std::cerr << e.what() << std::endl;    
	return -2;
    }

    // The baseline we want (other channels are also updated):
    m_baseline = m_baselineHistograms[channel];
  
    return 0;
}

/**
 * @details
 * Statistics size is different between XIA API version 2 and 3. 3.x provides
 * a Pixie16GetStatisticsSize() so we don't have to worry about calculating
 * the statistics size ourselves or use a hardcoded value. Accessing the run
 * statistics using the wrong method results in a segfault.
 *
 * @todo Confirm end of run and handle if not ended properly.
 */
int
CPixieRunUtilities::ReadModuleStats(int module)
{  
    // Where to read the statistics into, size depends on XIA API version:  
#if XIAAPI_VERSION >= 3
    std::vector<unsigned int> statistics(Pixie16GetStatisticsSize(), 0);
#else
    std::vector<unsigned int> statistics(448, 0); // see any v11.3.
#endif
  
    int retval = Pixie16ReadStatisticsFromModule(statistics.data(), module);
  
    if (retval < 0) {
	std::cerr << "CPixieRunUtilities::ReadModuleStats() error accessing scaler statistics " << "from module " << module << " with retval " << retval << std::endl;
	return retval;
    } else {
	double realTime = Pixie16ComputeRealTime(statistics.data(), module);
	for (int i = 0; i < 16; i++) {
	    double inpRate = Pixie16ComputeInputCountRate(
		statistics.data(), module, i
		);
	    double outRate = Pixie16ComputeOutputCountRate(
		statistics.data(), module, i
		);
	    double liveTime = Pixie16ComputeLiveTime(
		statistics.data(), module, i
		);      
	    std::cout << "Module " << module << " channel " << i
		      << " input " << inpRate << " output " << outRate
		      << " livetime " << liveTime << " runtime " << realTime
		      << std::endl;
	}
    }
  
    return retval;
}

/**
 * @details
 * Update baseline histograms using data read from the module or the data
 * generator. Note that the internal histogram maintained by this class has 
 * MAX_HISTOGRAM_LENGTH bins, [), 1 ADC unit/bin. Values outside this range 
 * are dropped and not dispayed. This may result in partial or no data being 
 * displayed for a baseline run depending on how the baseline looks.
 *
 * @todo (ASC 7/14/23): Handle out of range values better, at least warning 
 * the user that something has been dropped.
 */
void
CPixieRunUtilities::UpdateBaselineHistograms(int module)
{
    int retval = -1;
  
    for (int i = 0; i < 16; i++) {
	std::vector<double> baselines(MAX_NUM_BASELINES, 0);
	std::vector<double> timestamps(MAX_NUM_BASELINES, 0);    
	// Allocate data structure for baselines and grab them or use the
	// data generator to get data for testing:    
	if (!m_useGenerator) {
	    retval = Pixie16ReadSglChanBaselines(
		baselines.data(), timestamps.data(), MAX_NUM_BASELINES,
		module, i
		);      
	} else {
	    retval = m_pGenerator->GetBaselineData(
		baselines.data(), MAX_NUM_BASELINES
		);
	}
  
	if (retval < 0) {
	    std::stringstream errmsg;
	    errmsg << "CPixieRunUtilities::UpdateBaselineHistograms() failed to read baseline from module " << module << " channel " << i << " with retval " << retval;
	    throw std::runtime_error(errmsg.str());
	}
    
	// If we have the baseline, update its histogram for valid values:    
	for (const auto &ele : baselines) {
	    int bin = static_cast<int>(ele);
	    if (bin >= 0 && bin < MAX_HISTOGRAM_LENGTH) {
		m_baselineHistograms[i][bin]++;
	    } 
	} 
    
    }  
}

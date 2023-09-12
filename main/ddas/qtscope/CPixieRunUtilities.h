/**
 * @file CPixieRunUtilities.h
 * @brief Defines a class for managing list-mode and baseline runs and a 
 * ctypes interface for the class.
 */

#ifndef CPIXIERUNUTILITIES_H
#define CPIXIERUNUTILITIES_H

#include <vector>
#include <random>

class CDataGenerator;

/**
 * @addtogroup utilities libPixieUtilities.so
 * @{ 
 */

/** 
 * @class CPixieRunUtilities CPixieRunUtilities.h
 * @brief Manage list-mode histogram and baseline runs for a 
 * Pixie-16 system. 
 * 
 * This class provides functionality to start and stop runs as well 
 * as read data from the modules and return it to the caller.
 */

class CPixieRunUtilities
{
public:
    /** @brief Constructor. */
    CPixieRunUtilities();
    /** @brief Destructor. */
    ~CPixieRunUtilities();

    /**
     * @brief Begin a histogram (MCA) run for a single module. Explicitly sets 
     *   module synchronization to OFF.
     * @param module Module number.
     * @return int  
     * @retval 0   Success.
     * @retval !=0 XIA API error code.
     */
    int BeginHistogramRun(int module);
    /**
     * @brief End a histogram (MCA) run for a single module. Assumes module 
     * synchronization is OFF __but__ only stops a run in a single module.
     * @param module Module number.
     * @return int
     * @retval 0 Always returns 0 even if the run ended improperly.
     */
    int EndHistogramRun(int module);
    /**
     * @brief Read energy histogram from single channel.
     * @param module  Module number.
     * @param channel Channel number on module to read histogram from.
     * @return int  
     * @retval 0   Success.
     * @retval !=0 XIA API error code.
     */    
    int ReadHistogram(int module, int channel);

    /**
     * @brief Begin a baseline run.
     * @return int
     * @retval 0 Always.
     */
    int BeginBaselineRun(int module);
    /**
     * @brief "End" a baseline run.
     * @param module Module number.
     * @return int
     * @retval 0 Always.
     */
    int EndBaselineRun(int module);
    /**
     * @brief Acquire baselines and read baseline data from a single channel.
     * @param module  Module number.
     * @param channel Channel number on the module.
     * @return int 
     * @retval 0  Success.
     * @retval -1 If baseline memory cannot be allocated.
     * @retval -2 If updating the baseline histograms fails.
     */
    int ReadBaseline(int module, int channel);
    /**
     * @brief Read statistics for a single module after a run is ended.
     * @param module  Module number.
     * @return int  
     * @retval 0  Success.
     * @retval !=0  XIA API error code.
     * @todo Confirm end of run and handle if not ended properly.
     */
    int ReadModuleStats(int module);
    /**
     * @brief Get the histogram data from a list-mode run.
     * @return unsigned int* Pointer to the underlying histogram storage.
     */
    unsigned int* GetHistogramData() {return m_histogram.data();};
    /**
     * @brief Get the baseline run data.
     * @return unsigned int* Pointer to the underlying baseline storage.
     */
    unsigned int* GetBaselineData() {return m_baseline.data();};
    /**
     * @brief Get the current run status.
     * @return bool  True if a run is active, false otherwise.
     */
    bool GetRunActive() {return m_runActive;};
    /** 
     * @brief Set the use of the generator for offline data.
     * @param mode  Set the generator use flag to this value.
     */
    void SetUseGenerator(bool mode) {m_useGenerator = mode;};

private:
    std::vector<unsigned int> m_histogram; //!< Single channel histogram.
    std::vector<unsigned int> m_baseline;  //!< Single channel baseline
                                           //!< histogram.
    std::vector<std::vector<unsigned int>> m_baselineHistograms; //!< All
                                                                 //!< chans.
    std::vector<std::vector<unsigned int>> m_genHistograms; //!< Generated run
                                                            //!< data histos
                                                            //!< for all chans.
    bool m_runActive;    //!< True when running.
    bool m_useGenerator; //!< True to use generator test data.
    CDataGenerator* m_pGenerator; //!< Test data for debugging/offline mode.
    /**
     * @brief Update baseline histograms for all channels on a single module.
     * @param module Module number.
     * @throw std::runtime_error If the baseline read fails.
     */
    void UpdateBaselineHistograms(int module);
};

/** @} */

extern "C" {
    /** @brief Wrapper for the class constructor. */
    CPixieRunUtilities* CPixieRunUtilities_new()
    {
	return new CPixieRunUtilities();
    }

    /** @brief Wrapper to begin a list-mode histogram data run. */
    int CPixieRunUtilities_BeginHistogramRun(
	CPixieRunUtilities* utils, int mod
	)
    {
	return utils->BeginHistogramRun(mod);
    }
    /** @brief Wrapper to end a list-mode histogram data run. */
    int CPixieRunUtilities_EndHistogramRun(
	CPixieRunUtilities* utils, int mod
	)
    {
	return utils->EndHistogramRun(mod);
    }
    /** @brief Wrapper to read histogram data. */
    int CPixieRunUtilities_ReadHistogram(
	CPixieRunUtilities* utils, int mod, int chan
	)
    {
	return utils->ReadHistogram(mod, chan);
    }
  
    /** @brief Wrapper to begin a baseline data run. */
    int CPixieRunUtilities_BeginBaselineRun(
	CPixieRunUtilities* utils, int mod
	)
    {
	return utils->BeginBaselineRun(mod);
    }
    /** @brief Wrapper to end a baseline data run. */
    int CPixieRunUtilities_EndBaselineRun(CPixieRunUtilities* utils, int mod)
    {
	return utils->EndBaselineRun(mod);
    }
    /** @brief Wrapper to read the baseline data. */
    int CPixieRunUtilities_ReadBaseline(
	CPixieRunUtilities* utils, int mod, int chan
	)
    {
	return utils->ReadBaseline(mod, chan);
    }
  
    /** @brief Wrapper to read run statistics from the module. */
    int CPixieRunUtilities_ReadModuleStats(CPixieRunUtilities* utils, int mod)
    {
	return utils->ReadModuleStats(mod);
    }
    /** @brief Wrapper to marshall the histogram data. */  
    unsigned int* CPixieRunUtilities_GetHistogramData(
	CPixieRunUtilities* utils
	)
    {
	return utils->GetHistogramData();
    }
    /** @brief Wrapper to marshall the baseline data. */
    unsigned int* CPixieRunUtilities_GetBaselineData(CPixieRunUtilities* utils)
    {
	return utils->GetBaselineData();
    }  
    /** @brief Wrapper to get the run active status. */
    bool CPixieRunUtilities_GetRunActive(CPixieRunUtilities* utils)
    {
	return utils->GetRunActive();
    }
    /** @brief Wrapper to setup the offline data generator. */
    void CPixieRunUtilities_SetUseGenerator(
	CPixieRunUtilities* utils, bool mode
	)
    {
	return utils->SetUseGenerator(mode);
    }
  
    /** @brief Wrapper for the class constructor. */  
    void CPixieRunUtilities_delete(CPixieRunUtilities* utils)
    {
	if(utils) {
	    delete utils;
	    utils = nullptr;
	}
    };
}

#endif

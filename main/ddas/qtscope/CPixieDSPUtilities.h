/**
 * @file CPixieDSPUtilities.h
 * @brief Defines a class to read and write settings to XIA Pixie modules 
 * and a ctypes interface for the class.
 */

#ifndef CPIXIEDSPUTILITIES_H
#define CPIXIEDSPUTILITIES_H

/**
 * @addtogroup utilities libPixieUtilities.so
 * @{ 
 */

/**
 * @class CPixieDSPUtilities CPixieDSPUtilities.h
 * @brief Read and writes both channel-level and module-level DSP settings.
 *
 * @details
 * The class also contains a function to adjust the DC offsets on a single 
 * module, as the DC offset is itself a channel parameter.
 */

class CPixieDSPUtilities
{
public:
    /** Constructor. */
    CPixieDSPUtilities() {};
    /** Destructor. */
    ~CPixieDSPUtilities() {};

    /**
     * @brief Adjust DC offsets of all channels for a single module.
     * @param module Module number.
     * @return int
     * @retval 0    Success.
     * @retval !=0  XIA API error code.
     */
    int AdjustOffsets(int module);
    /**
     * @brief Write a channel parameter for a single channel. 
     * @param module    Module number.
     * @param channel   Channel number on module.
     * @param paramName XIA API chanel parameter name.
     * @param value     Parameter value to write.
     * @return int  
     * @retval 0   Success.
     * @retval !=0 XIA API error code.
     */
    int WriteChanPar(int module, int channel, char* paramName, double value);
    /**
     * @brief Read a channel parameter for a single channel.
     * @param[in] module    Module number.
     * @param[in] channel   Channel number on module.
     * @param[in] paramName XIA API channel parameter name.
     * @param[in,out] value Reference to read parameter value.
     * @return int  
     * @retval 0   Success.
     * @retval !=0 XIA API error code.
     */
    int ReadChanPar(int module, int channel, char* paramName, double& value);
    /**
     * @brief Write a module parameter for a single module. 
     * @param module    Module number.
     * @param paramName XIA API chanel parameter name.
     * @param value     Parameter value to write.
     * @return int  
     * @retval 0   Success.
     * @retval !=0 XIA API error code.
     */
    int WriteModPar(int module, char* paramName, unsigned int value);
    /**
     * @brief Read a module parameter for a single module. 
     * @param[in] module     Module number.
     * @param[in] paramName  XIA API chanel parameter name.
     * @param[in,out] value  Reference to read parameter value.
     * @return int  
     * @retval 0  Success.
     * @retval !=0  XIA API error code.
     */
    int ReadModPar(int module, char* paramName, unsigned int& value);
};

/** @} */

extern "C" {
    /** @brief Wrapper for the class constructor. */
    CPixieDSPUtilities* CPixieDSPUtilities_new()
    {
	return new CPixieDSPUtilities();
    }

    /** @brief Wrapper to adjust DC offsets. */
    int CPixieDSPUtilities_AdjustOffsets(
	CPixieDSPUtilities* utils, int mod
	)
    {
	return utils->AdjustOffsets(mod);
    }
    /**  @brief Wrapper to write a channel parameter. */
    int CPixieDSPUtilities_WriteChanPar(
	CPixieDSPUtilities* utils, int mod, int chan, char* pName, double val
	)
    {
	return utils->WriteChanPar(mod, chan, pName, val);
    }
    /**  @brief Wrapper to read a channel parameter. */
    int CPixieDSPUtilities_ReadChanPar(
	CPixieDSPUtilities* utils, int mod, int chan, char* pName, double& val
	)
    {
	return utils->ReadChanPar(mod, chan, pName, val);
    }
    /**  @brief Wrapper to write a module parameter. */
    int CPixieDSPUtilities_WriteModPar(
	CPixieDSPUtilities* utils, int mod, char* pName, unsigned int val
	)
    {
	return utils->WriteModPar(mod, pName, val);
    }
    /**  @brief Wrapper to read a module parameter. */
    int CPixieDSPUtilities_ReadModPar(
	CPixieDSPUtilities* utils, int mod, char* pName, unsigned int& val
	)
    {
	return utils->ReadModPar(mod, pName, val);
    }
  
    /**  @brief Wrapper for the class destructor. */
    void CPixieDSPUtilities_delete(CPixieDSPUtilities* utils)
    {
	if(utils) {
	    delete utils;
	    utils = nullptr;
	}
    };
}

#endif

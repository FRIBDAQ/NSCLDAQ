/**
 * @file CDataGenerator.h
 * @brief Defines a class for generating offline data for testing/debugging 
 * and a ctypes interface for the class.
 */

#ifndef CDATAGENERATOR_H
#define CDATAGENERATOR_H

#include <random>

/**
 * @addtogroup utilities libPixieUtilities.so
 * @brief Pixie-16 utilities for QtScope.
 *
 * This utility library is used by QtScope. It contains a number of classes
 * which call other parts of the DDAS code to boot and manage the modules. 
 * This library defines an API by which the pure-Python QtScope code can 
 * interact with the C/C++ FRIBDAQ and XIA API code needed to run a system of 
 * Pixie modules.
 * @{ 
 */

/**
 * @class CDataGenerator
 * @brief A class to generate test pulse, run, and baseline data for offline 
 * operation of QtScope.
 */

class CDataGenerator
{
public:
    CDataGenerator();
    ~CDataGenerator();

    int GetTraceData(unsigned short* data, int dataSize, double binWidth);
    int GetHistogramData(unsigned int* data, int dataSize);
    int GetBaselineData(double* data, int dataSize);
  
private:
    std::mt19937 m_engine; //!< Random number generator engine.
  
    unsigned short SinglePulse(
	double C, double A, double t0, double rise, double decay,
	int sample, double binWidth
	);
};

/** @} */

#endif

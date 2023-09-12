/**
 * @file CDataGenerator.cpp
 * @brief Implementation of the offline data generation class.
 */

#include "CDataGenerator.h"

#include <iostream>
#include <cmath>

/**
 * @brief Constructor.
 */
CDataGenerator::CDataGenerator() :
    m_engine((std::random_device())())
{}

/**
 * @brief Destructor.
 */
CDataGenerator::~CDataGenerator() {}

/**
 * @brief Generate test trace data.
 *
 * Params are a pointer to the start of the data storage and a size, as is 
 * done in the XIA API for easier integration/consistency.
 *
 * @param[in,out] data  Pointer to the start of the trace data storage.
 * @param[in] dataSize  How many data points to store.
 * @param[in] binWidth  Histogram bin width in microseconds.
 *
 * @return int  
 * @retval 0  Success.
 */
int
CDataGenerator::GetTraceData(
    unsigned short* data, int dataSize, double binWidth
    )
{
    std::uniform_real_distribution<double> C(1000, 2000);
    std::uniform_real_distribution<double> A(100, 10000);
    std::uniform_real_distribution<double> t0(0.05*dataSize, 0.95*dataSize);
    std::normal_distribution<double> rise(0.5, 0.05);
    std::normal_distribution<double> decay(5, 0.05);

    double myC = C(m_engine);         // ADC units.
    double myA = A(m_engine);         // ADC units.
    double myT0 = t0(m_engine);       // Sample number.
    double myRise = rise(m_engine);   // Microseconds.
    double myDecay = decay(m_engine); // Microseconds.
  
    for (int i = 0; i < dataSize; i++) {
	data[i] = SinglePulse(myC, myA, myT0, myRise, myDecay, i, binWidth);
    }
  
    return 0;
}

/**
 * @brief Generate test gaussian peaks.
 *
 * Params are a pointer to the start of the data storage and a size, as is 
 * done in the XIA API for easier integration/consistency.
 *
 * @param[in,out] data  Pointer to the start of the baseline data storarge.
 * @param[in] dataSize  How many data points to store.
 *
 * @return int  
 * @retval 0  Success.
 */
int
CDataGenerator::GetHistogramData(unsigned int* data, int dataSize)
{
    int nEvents = 500;
    std::uniform_real_distribution<double> rand(0, 1);
  
    // Location is 1/3 and 2/3 of the histogram size:  
    std::normal_distribution<double> p1(0.33*dataSize, 10); // Mean, stddev.
    std::normal_distribution<double> p2(0.67*dataSize, 10); // Mean, stddev.

    // nEvents into histogram per read:  
    double p2Fraction = 0.67; // Fraction of total intensity in p2.
    int ene = 0; // Event energy.
    for (int i = 0; i < nEvents; i++) {
	double roll = rand(m_engine);
	if (roll < p2Fraction) {
	    ene = static_cast<unsigned int>(p1(m_engine));
	} else {
	    ene = static_cast<unsigned int>(p2(m_engine));
	}
	data[ene]++;	
    }  

    return 0;  
}

/**
 * @brief Generate randomly distributed test baseline data.
 *
 * Params are a pointer to the start of the data storage and a size, as is 
 * done in the XIA API for easier integration/consistency.
 *
 * @param[in,out] data  Pointer to the start of the baseline data storarge.
 * @param[in] dataSize  How many data points to store.
 *
 * @return int  
 * @retval 0  Success.
 */
int
CDataGenerator::GetBaselineData(double* data, int dataSize)
{
    std::uniform_real_distribution<double> dist(4500, 5500); // range
    for (int i = 0; i < dataSize; i++) {
	data[i] = dist(m_engine);
    }

    return 0;  
}

/**
 * @brief Analytical function for a single pulse with exponential rise and 
 * decay constants.
 *
 * @param C       Constant baseline.
 * @param A       Pulse amplitude.
 * @param t0      Start of the pulse.
 * @param rise    Pulse risetime in ns.
 * @param decay   Pulse exponential decay time in ns.
 * @param sample  Sample number where we compute the pulse.
 *
 * @return unsigned short  Pulse value at input sample number.
 */
unsigned short
CDataGenerator::SinglePulse(
    double C, double A, double t0, double rise, double decay,
    int sample, double binWidth
    )
{
    // Convert position to dt in us using the binWidth determined by the XDT
    // channel parameter value:
  
    double dt = (sample - t0)*binWidth;
  
    std::normal_distribution<double> noise(0, 10); // Mean, stddev.
    unsigned short pval = 0;  
  
    if (sample < t0) {
	pval =  static_cast<unsigned short>(C + noise(m_engine));
    } else {
	pval =  static_cast<unsigned short>(
	    C + A*(1-std::exp(-dt/rise))*std::exp(-dt/decay) + noise(m_engine)
	    );
    }

    return pval;
}

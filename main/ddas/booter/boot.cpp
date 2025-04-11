/**
 * @file boot.cpp
 * @brief Generate a configuration and perform a default (full) boot.
 */

#include "SystemBooter.h"

#include <Configuration.h>

using namespace DAQ::DDAS;

/**
 * @brief boot main.
 *
 * @details
 * - Create a SystemBooter object.
 * - Generate a basic configuration.
 * - Attempt a system boot for the configuration.
 *
 * @return 0 Successful boot, no exceptions, etc.
 */
int main()
{
    SystemBooter booter;
    auto pConfig = Configuration::generate("cfgPixie16.txt");
    booter.boot(*pConfig, SystemBooter::FullBoot);

    return 0;
}

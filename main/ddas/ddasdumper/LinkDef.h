/**
 * @addtogroup libddaschannel libddaschannel.so
 * @{
 */

/**
 * @file LinkDef.h
 * @brief Tell rootcling to add our custom classes DDASEvent, ddaschannel and 
 * a std::vector<ddaschannel> object to the dictionary. Necessary for I/O of 
 * custom classes in ROOT.
 */

#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ class DDASEvent+;
#pragma link C++ class std::vector<ddaschannel*>!;
#pragma link C++ class ddaschannel+;

#endif

/** @} */

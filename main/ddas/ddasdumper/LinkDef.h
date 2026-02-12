/**
 * @addtogroup libddasrootformat libddasrootformat.so
 * @{
 */

/**
 * @file LinkDef.h
 * @brief Tell rootcling to add our custom classes DDASRootEvent, DDASRootHit
 * and a std::vector<DDASRootHit*> object to the dictionary. Necessary for I/O
 * of custom classes in ROOT.
 */

#ifdef __CINT__

#pragma link C++ nestedclasses;
#pragma link C++ nestedtypedefs;
#pragma link C++ class DDASRootEvent + ;
#pragma link C++ class std::vector < DDASRootHit *> !;
#pragma link C++ class DDASRootHit + ;
#pragma link C++ class ddasfmt::DDASHit + ;

#endif

/** @} */

// Converter.h
//
// Author : Jeromy Tompkins
// Date   : 5/7/2013
//
// Purpose: Defines the base class from which all concrete converter
//          objects derive. See more information below.

#ifndef CONVERTER_H
#define CONVERTER_H

#include <string>

class TFile;
class TTree;

namespace ufmt {
    class CRingItem;
    class CPhysicsEventItem;
}
    
/** \class Converter
 * \brief Base class for all converter objects
 *
 * This is an abstract class and cannot be instantiated. It provides the interface that must be
 * implemented by all derived classes. At the moment these are the RootConverter and
 * RootConverter2 classes. The BufdumpMain class only deals with the interface defined here
 * such that the use of different conversion objects in the ddasdumper program is transparent to
 * the remainder of the program. 

 * There is one pure virtual member, DumpData(const CRingItem&), that exists because there is no 
 * sensible default implementation. 
 *
 * This base class does not merely define an interface though, instead it provides some basic 
 * functionality that will promote consistency between output data of all concretely defined 
 * classes. The basic functionality defined by this class is the generation of the TFile 
 * that will be used to store the outputted data and also the TTree that contains the converted
 * data. The actual structure of the TTree is not defined here and its definition is left as a 
 * responsibility derived classes.
*/
class Converter
{

    protected:
        TFile *m_fileout;  /**< pointer to output root file */
        TTree *m_treeout;  /**< pointer to output tree */

    public:
        /** \brief Default constructor
         * 
         * This initializes the pointers to both member pointers as
         * null.
         */
        Converter() : m_fileout(0), m_treeout(0) {}

        /** \brief Default destructor
         *  
         * Calls the Close() method if the object's file pointer is
         * not null. Because this object's tree pointer is owned by the
         * the file it lives in, it is not deleted. Closing the file
         * is enough to free the memory block associated with it.
         * Note that any time this class closes the file, 
         * the file pointer is set to null. 
         */
        virtual ~Converter()
        {
            if (m_fileout!=0) Close();        
        }

        /** \brief Open output root file and creates the tree structure.
         *  
         * Creates and opens the root file in which the tree will be stored
         * and creates the tree. There are no branches added to the tree.
         * Adding the branches is left as a responsibility to the derived classes. 
         * 
         * *** Derived classes MUST CALL THIS METHOD BEFORE ADDING TO THE TREE! ***
         *
         *  @param in_fname does not serve any functional purpose 
         *  @param fileout name of the root file to create for output
         */
        virtual void Initialize(std::string in_fname, std::string fileout);

        /** \brief Dump the data from ring into a object in a root file
         *
         *  Conversion method. This is a purely virtual method. All derived classes
         *  must implement this to support the conversion of the data stored in the
         *  CRingItem. This method is also where data is added to the TTree by means
         *  of the TTree::Fill() method. The user is presented with the full 
         *  CRingItem, including its header. The structure of these can be found in
         *  in the file /path/to/nscldaq/version/./include/DataFormat.h
         *
         *  @param item is a reference to the data
         */
    virtual void DumpData(const ufmt::CPhysicsEventItem& item) = 0;

        /** \brief Close ROOT file
         * 
         * Closes the root file if it exists. When the file is closed, the pointer
         * is reset to null.
         */
        virtual void Close();

        /** \brief Access the output file
        */
        TFile* GetFile() { return m_fileout;}

        /** \brief Access the output tree 
        */
        TTree* GetTree() { return m_treeout;}

};

#endif

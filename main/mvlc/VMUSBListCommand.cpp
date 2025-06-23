/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox 
             Facility for Rare Isotope4s
             Michigan State University
             East Lansing, MI 48824-1321

@file VMUSBListComand.cpp
@author Ron Fox <fox at frib dot msu dot edu>
@brief Implementaition for Tcl  encapsulation of CVMUSBReadoutList
*/
#include "VMUSBListCommand.h"
#include <TCLInterpreter.h>
#include <TCLObject.h>
#include "CVMUSBReadoutList.h"
#include <tcl.h>
#include <stdexcept>
#include <sstream>
#include <Exception.h>
#include <string>

// This is a vector of the variable names and values
// we need to set.  It drivers createAmodVars.
// note that setAmodVar prepends the namespace (CVMUSBReadoutList).
// rather than using CTCLVariable, we'll use Tcl_ObjSetVar so that Tcl
// can deal with the string representation.

typedef std::pair<std::string, uint8_t> VarInfo;
static std::vector<VarInfo> amodVariables = {
   {"a32UserData", CVMUSBReadoutList::a32UserData}, 
   {"a32UserProgram", CVMUSBReadoutList::a32UserProgram},  // A32 user mode amods.
   {"a32UserBlock", CVMUSBReadoutList::a32UserBlock},

   {"a32PrivData", CVMUSBReadoutList::a32PrivData},
   {"a32PrivProgram", CVMUSBReadoutList::a32PrivProgram},   // A32 privileged mode.
   {"a32PrivBlock", CVMUSBReadoutList::a32PrivBlock},

   {"a16User", CVMUSBReadoutList::a16User},                 // Short IO.
   {"a16Priv", CVMUSBReadoutList::a16Priv},   

   {"a24UserData", CVMUSBReadoutList::a24UserData},
   {"a24UserProgram", CVMUSBReadoutList::a24UserProgram},   // A24 user.
   {"a24UserBlock", CVMUSBReadoutList::a24UserBlock},

   {"a24PrivData", CVMUSBReadoutList::a24PrivData},
   {"a24PrivProgram", CVMUSBReadoutList::a24PrivProgram},  // A24 privileged.
   {"a24PrivBlock", CVMUSBReadoutList::a24PrivBlock}
};


/**
 *  constructor
 *     Construct the command object
 * 
 * @param interp  - reference to the intepreter on which to register.
 * @note we null the list and before using the command, setList must be invoked to
 * encapsulate the VMUSBReadoutList that will be manipulated.
 */

VMUSBListCommand::VMUSBListCommand(CTCLInterpreter& interp)  :
   CTCLObjectProcessor(interp, "mvlclist", TCLPLUS::kfTRUE),
   m_pList(0) {
      createAmodVars();      // Create the address modifier variables.
   }

/**
 *  destructor
 *     The list ownership does not transer in to us so we don't delete it.
 */
VMUSBListCommand::~VMUSBListCommand() {}


/**
 * setList
 *    Prior to executing the mvlclist command, a list must be encapsulated
 *  This is done by invoking setList here.  Ownership of the list is not transferred
 * to us.  clearList should be called when the list goes out of scope or does not need
 * to be encapsulated for any other reason.
 * 
 * @param list - The list to encapsulste.
 */
void
VMUSBListCommand::setList(CVMUSBReadoutList& list) {
   m_pList = &list;
}
/**
 *  getList
 *    return the, possible null, pointer to the encapsulated list.
 * A null return value means no list is currently encapsulated.
 * 
 * @return CVMUSBReadoutList*
 */
CVMUSBReadoutList*
VMUSBListCommand::getList() {
   return m_pList;
}


/**
 * clearList
 *     Nulls out the encapsulated list.  Before using the mvlclist command again, setList must be called.
 */
void
VMUSBListCommand::clearList() {
   m_pList = nullptr;
}

/**
 *  operator()
 *     Executes the mvlc command:
 *  - Binds all command line objects to the interpreter running the command.
 *  - Ensures there is an encapsulated list.
 *  - Ensures there is a subcommand and dispatches to the appropriate subcommand handler.
 * 
 * @note very early things are setup to that errors can be thrown via exceptions/
 * 
 * @param interp -  interpreter running the command.
 * @param objv   -  encapsulated command words.
 * @return int   - TCL_OK on success or TCL_ERROR if not.  TCL_ERROR returns
 *  set an error message in the result.
 */
int
VMUSBListCommand::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv) {
   bindAll(interp, objv);

   try {
      throwIfNoList();
      requireAtLeast(objv, 2, "The mvlclist command requires a subcommand");
      std::string subcommand = objv[1];

      if (subcommand == "addWrite32") {
         addWrite32(interp, objv);
      } else if (subcommand == "addWrite16") {
         addWrite16(interp, objv);
      } else if (subcommand == "addBlockwrite32") {
         addBlockWrite32(interp, objv);
      } else if (subcommand == "addBlockRead32") {
         addBlockRead32(interp, objv);
      } else if (subcommand == "addFifoRead32") {
         addFifoRead32(interp, objv);
      } else if (subcommand == "addBlockCountRead16") {
         addBlockCountRead16(interp, objv);
      } else if (subcommand == "addBlockCountRead32") {
         addBlockCountRead32(interp, objv);
      } else if (subcommand == "addMaskedCountBlockRead32" ) {
         addMaskedCountBlockRead32(interp, objv);
      } else if (subcommand == "addMaskedCountFifoRead32") {
         addMaskedCountFifoRead32(interp, objv);
      } else if (subcommand == "addDelay") {
         addDelay(interp, objv);
      } else if (subcommand == "addMarker") {
         addMarker(interp, objv);
      } else if (subcommand == "addLoopUntil32") {
         addLoopUntil32(interp, objv);
      } else if (subcommand == "addLoopUntil16") {
         addLoopUntil16(interp, objv);
      } else {
         std::stringstream smsg;
         smsg << subcommand << " is not a valid subcommand for " << std::string(objv[0]);
         std::string msg(smsg.str());
         throw std::invalid_argument(msg);
      }
   }
   catch (std::exception& e) {
      interp.setResult(e.what());
      return TCL_ERROR;
   }
   catch (CException& e) {
      interp.setResult(e.ReasonText());
      return TCL_ERROR;
   } 
   catch(std::string msg) {
      interp.setResult(msg);
      return TCL_ERROR;
   }
   return TCL_OK;
}

/////////////////////////// Subcommand handlers (private):

/**
 * addWrite32 - add a 32 bit write to the data.  This is of the form: 
 * mvlc addWrite32 address amod data
 * 
 * @param interp - interpreter running the command.
 * @param objv   - command words.
 */
void
VMUSBListCommand::addWrite32(CTCLInterpreter& interp, std::vector<CTCLObject>& objv) {
   auto params = getWriteParams(objv);             // Also validates parameter count.
   
   m_pList->addWrite32(std::get<0>(params), std::get<1>(params), std::get<2>(params));
}
/**
 * addWrite16
 *    Same as above but the write is a 16 bit write:
 */
void
VMUSBListCommand::addWrite16(CTCLInterpreter& interp, std::vector<CTCLObject>& objv) {
   auto params = getWriteParams(objv);

   m_pList->addWrite16(
      std::get<0>(params), std::get<1>(params), uint16_t(std::get<2>(params))
   );
}
/**
 * addBlockWrite32
 *     Adds a write of a block of data. This is of the form mvlclist addBlockWrite32 base amod data-list
 * where data-list is a list of integer data for the write.
 * 
 * @param interp - interpreter running the command
 * @param objv   - The command words.
 */
void
VMUSBListCommand::addBlockWrite32(CTCLInterpreter& interp, std::vector<CTCLObject>& objv) {
   requireExactly(objv, 5, "Usage: mvlc addBlockWrite32 base amod data-list");

   // This stuff just gets the start address and the amod.

   std::vector<CTCLObject> addr(objv.begin(), objv.begin()+3);
   auto addrInfo = getReadParams(addr);

   // This gets the list of data marshalled into a vector.

   auto writeData = listToWriteBlock(objv[4]);

   // Now we can add the item to the list.

   m_pList->addBlockWrite32(std::get<0>(addrInfo), std::get<1>(addrInfo), writeData.data(), writeData.size());
}

/**
 * addRead32
 *    Add a 32 bit read to the list.  The format of this is mvlclist addRead32 addr amod.
 * 
 * @param interp - interpreter running the command.
 * @param objv   - The command words.
 */
void
VMUSBListCommand::addRead32(CTCLInterpreter& interp, std::vector<CTCLObject>& objv) {
   auto addrInfo = getReadParams(objv);

   m_pList->addRead32(std::get<0>(addrInfo), std::get<1>(addrInfo));
}
/**
 *  addRead16 - same as above but adds a 16 bit read:
 */
void
VMUSBListCommand::addRead16(CTCLInterpreter& interp, std::vector<CTCLObject>& objv) {
   auto addrInfo = getReadParams(objv);

   m_pList->addRead16(std::get<0>(addrInfo), std::get<1>(addrInfo));
}

/**
 * addBlockRead32
 *    Add a block read of a fixed number of 32 bit words.  This is of the form:
 * mvlclist addBlockRead32 base amod count
 * 
 * where count is the numbe of longwords to read.  Note that the MVLC may terminate
 * the read early if a BERR is raised as many modules to when read to empty
 * 
 * @param interp - interpreter running the command.
 * @param objv   - Command words.
 * 
 * @note - you should, where possible, specify a bloack read address modifier as that reduces
 * the number of address cycles that will be performed by the hardware.
 */
void
VMUSBListCommand::addBlockRead32(CTCLInterpreter& interp, std::vector<CTCLObject>& objv) {
   auto params = getBlockReadParams(objv);

   m_pList->addBlockRead32(std::get<0>(params), std::get<1>(params), std::get<2>(params));
}
/**
 *  addFifoRead32
 *    Same as above, however the address presented on the bus is always the same.  The assumption
 * is that it corresponds to a 32 bit wide FIFO port.
 */
void
VMUSBListCommand::addFifoRead32(CTCLInterpreter& interp, std::vector<CTCLObject>& objv) {
   auto params = getBlockReadParams(objv);

   m_pList->addFifoRead32(std::get<0>(params), std::get<1>(params), std::get<2>(params));
}

/**
 * addBlockCountRead16
 *   Add  a 16 bit read to the list which will, rather than being placed in the event buffer,
 * be used as the transfer count for an immediate subsequenty addMaskedCountBlockRead32 or
 * addMaskedCountFIfoRead32.  The form of this is:
 * 
 * mvlclist addBlockCountRead16 address mask amod
 * 
 * The mask specifies a field of contiguous bits that contains the count.  E.g. suppose the mask is
 * 0x00f0 and the data read are 0xfedc  the count will be 0xd, the bits in the mask.
 * 
 * @param interp - interpreter running the command.
 * @param objv   - the command words.
 */
void
VMUSBListCommand::addBlockCountRead16(CTCLInterpreter& interp, std::vector<CTCLObject>& objv) {
   auto params = getBlockCountReadParams(objv);
   m_pList->addBlockCountRead16(
      std::get<0>(params), std::get<1>(params), std::get<2>(params)
   );
}
/**
 *  addBlockCountRead32
 *     Same as above but the read ia a 32 bit wide read.   E.g. if the mask is 0xf0000000 and the adata are
 * 0x76543210 the count will be 7.
 */
void
VMUSBListCommand::addBlockCountRead32(CTCLInterpreter& interp, std::vector<CTCLObject>& objv) {
   auto params = getBlockCountReadParams(objv);
   m_pList->addBlockCountRead32(
      std::get<0>(params), std::get<1>(params), std::get<2>(params)
   );
}
/**
 *  addMaskedCountBlockRead
 * 
 *    Do block read with the count gotten from the last addBlockCountReadxx operation. Form:
 * mvlclist addMakedCountBlockRead base-address amod
 * 
 * @param interp - interpreter running the command.
 * @param objv   - The command words.
 */
void
VMUSBListCommand::addMaskedCountBlockRead32(CTCLInterpreter& interp, std::vector<CTCLObject>& objv) {
   auto params = getReadParams(objv);

   m_pList->addMaskedCountBlockRead32(std::get<0>(params), std::get<1>(params));
}
/**
 * addMaskedCountFifoRead
 * 
 * Same as above but all reads are directed at the same address.
 */
void
VMUSBListCommand::addMaskedCountFifoRead32(CTCLInterpreter& interp, std::vector<CTCLObject>& objv) {
   auto params = getReadParams(objv);

   m_pList->addMaskedCountFifoRead32(std::get<0>(params), std::get<1>(params));
}
/**
 * addDelay
 *    Add a delay to the stack.  Format:  mvlclist addDelay how-long
 * 
 * @param interp - interpreter running the command.
 * @param objv   - Command words.
 */
void
VMUSBListCommand::addDelay(CTCLInterpreter& interp, std::vector<CTCLObject>& objv) {
   requireExactly(objv, 3, "Usage: mvlclist addDelay how-long");

   int delay = objv[2];
   m_pList->addDelay((uint32_t)(delay));
}
/**
 *  addMarker
 *     Add a marker to the stack. Note that in VMUSB markers are 16 bit items but
 * for the MVLC they are 32 bit items.
 * 
 * @param interp - interpreter running the command.
 * @param objv   - command line words
 */
void
VMUSBListCommand::addMarker(CTCLInterpreter& interp, std::vector<CTCLObject>& objv) {
   int value = objv[2];
   m_pList->addMarker((uint32_t)(value));
}

/**
 *  addLoopUntil32
 *     Adds elements to the stack to loop until some condition occurs. Format:
 * 
 * mvlclist addLoopUntil32 addr, amod mask value
 * 
 * This loops reading the 32 bit value att addr:amod until the value masked by the mask parameter is equal to value.
 * Suppose mask and value of 0x1 and 1 respectively, a read value of 0x12345678 will loop again but 0x123456879 will.
 * 
 * @param interp - interpreter executing the command.
 * @param objv  - the command words.
 * 
 */
void
VMUSBListCommand::addLoopUntil32(CTCLInterpreter& interp, std::vector<CTCLObject>& objv) {
   auto params = getLoopUntilParams(objv);

   m_pList->addLoopUntil32(
      std::get<0>(params), std::get<1>(params), std::get<2>(params), std::get<3>(params)
   );
}
/**
 *  addLoopUntil16 - same as above but the read is a 16 bit read.
 */
void
VMUSBListCommand::addLoopUntil16(CTCLInterpreter& interp, std::vector<CTCLObject>& objv) {
   auto params = getLoopUntilParams(objv);

   m_pList->addLoopUntil16(
      std::get<0>(params), std::get<1>(params), std::get<2>(params), std::get<3>(params)
   );
}
//////////////////////////// General utilities.

/**
 * throwIfNoList
 *     If m_pList is null, throws an std::logic_error with a message that makes it
 * clear this is a bug.
 */
void
VMUSBListCommand::throwIfNoList() {
   if (!m_pList) {
      throw std::logic_error("*BUG* prior to using the mvlclist command a list must be bound into the command.");
   }
}
/**
 * createAmodVars
 *   1. Create the CVMUSBReadoutList namespace.
 *   2. For each pair in amodVariables; call setAmodVar to create/set the value of 
 * the appropriate variable.
 */
void
VMUSBListCommand::createAmodVars() {
   Tcl_Interp* pInterp = getInterpreter()->getInterpreter();

   Tcl_CreateNamespace(pInterp, "CVMUSBReadoutList", nullptr, nullptr);
   for (auto v : amodVariables) {
      setAmodVar(v.first.c_str(), v.second);
   }
}
/**
 * setAmodVar
 *      Set the value of an address modifier variable.  
 * 
 * @param name - unscoped name of the variable ...we prepend CVMUSBReadoutList::
 * @param value - value to give to the variable (an address modifier).
 */
void
VMUSBListCommand::setAmodVar(const char* name, uint8_t value) {
   std::string scopedName = "CVMUSBReadoutList::";
   scopedName  += name;
   CTCLObject oName;
   oName.Bind(getInterpreter());
   oName = scopedName;

   CTCLObject oValue;
   oValue.Bind(getInterpreter());
   oValue = (int)value;

   auto varObj = Tcl_ObjSetVar2(
      getInterpreter()->getInterpreter(), oName.getObject(), nullptr, oValue.getObject(),
      TCL_GLOBAL_ONLY | TCL_LEAVE_ERR_MSG
   );

   if (!varObj) {
      std::stringstream smsg;
      smsg << " Could not set variable: " << scopedName << " to 0x" << std::hex << value
         << " : " << Tcl_GetStringResult(getInterpreter()->getInterpreter());
      std::string msg(smsg.str());
      throw std::runtime_error(msg);
   }
}
/**
 *  decodeAmod
 *     Given a command word; decode it as an addresss modifier.
 *     Address modifiere must decode asn integers and fit into
 *     a uint8_t.
 * @param obj - the command word.
 * @return uint8_t - the address modifier.
 * @throw CTCLException - if the obj does not decode to an integer.
 * @throw std::invalid_argument if the decoded integer is not a valid amod.
 */
uint8_t
VMUSBListCommand::decodeAmod(CTCLObject& obj) {
   int rawValue = obj;
   if (rawValue != (rawValue & 0xff)) {
      std::stringstream smsg;
      smsg << std::hex << "0x"  << rawValue << " is not a valid address modfier";
      std::string msg(smsg.str());
      throw std::invalid_argument(msg);
   }
   return uint8_t(rawValue);
}

/**
 * getWriteParames
 *    A write operation looks like: mvlclist <some-write-comand> address amod value
 * 
 * We:
 *    - Ensure the command has exactly 5 command words
 *    - Decode the addresss, modifier and value.
 *    - return them as a tuple containing in order address, modifier and value.
 * 
 * @parma objv - the command words.
 * @return std::tuple<uint32_t, uint8_t, uint32_t> - address, amod and value.
 * @throw CTCLException - if conversions fail.
 * @throw std::string - the parameter count is incorrect.
 * @throw any exception from decodeAmod above.
 */
std::tuple<uint32_t, uint8_t, uint32_t>
VMUSBListCommand::getWriteParams(std::vector<CTCLObject>& objv) {
   requireExactly(objv, 5, "Write operations require only an address, amod and write data");

   uint32_t address = int(objv[2]);
   uint8_t  amod    = decodeAmod(objv[3]);
   uint32_t data    = int(objv[4]);

   return std::make_tuple(address, amod, data);
}
/**
 * getReadParams
 *    Read operations look like mvlclist <some-read-command> address amod
 * 
 * We:
 *    - Ensure the command has the correct number of parameters.
 *    - Decode the address and modifier.
 *    - return a tuple containng, in order, tha address and modifier.
 * 
 * @param objv - the command words.
 * @return std::tuple<uint32_t, uint8_t> - the address and modifier.
 * @throw CTCLException - if conversions fail.
 * @throw std::string - the parameter count is incorrect.
 * @throw any exception from decodeAmod above.
 */
std::tuple<uint32_t, uint8_t> 
VMUSBListCommand::getReadParams(std::vector<CTCLObject>& objv) {
   requireExactly(objv, 4, "Read operations require only an addresss and its modifier");

   uint32_t addr = int(objv[2]);
   uint8_t  amod = decodeAmod(objv[3]);

   return std::make_tuple(addr, amod);
}
/**
 * listToWriteBlock
 *    Converts a Tcl formatted list into a vector of integers.  This is use by
 * the block write operation.
 * 
 * @param obj  -The object containing the list.
 * @return std::vecftor<uint32_t> - the list data as a vector.
 * @throw CTCLExxception - if one of the items in the list does not convert to an integer.
 */
std::vector<uint32_t>
VMUSBListCommand::listToWriteBlock(CTCLObject& obj) {
   std::vector<uint32_t> result;

   auto list = obj.getListElements();
   CTCLInterpreter* interp = getInterpreter();
   for(auto& item: list) {
      item.Bind(interp);
      uint32_t value = int(item);
      result.push_back(value);
   }
   return result;
}
/**
 * getBlockReadParams
 *    Block and FIFO reads are of the form mvlclist <blk-read-op> adress amod numxfers
 * 
 * This method takes all of the command lines parameters for such a command and
 * returns a tuple containing, in order, that address, modifier an transver count.
 * 
 * @param objv - the command words.
 * @return std::tuple<uint32_t, uint8_t, size_t> - the address, its modifier and the tranfer count.
 * @throw CTCLException - if conversions fail.
 * @throw std::string - the parameter count is incorrect.
 * @throw any exception from decodeAmod above.
 */
std::tuple<uint32_t, uint8_t, size_t> 
VMUSBListCommand::getBlockReadParams(std::vector<CTCLObject>& objv) {
   requireExactly(
      objv, 5, "Block reads require only an address, its modifier, and the transfer count"
   );

   uint32_t addr = int(objv[2]);
   uint8_t  amod = decodeAmod(objv[3]);
   size_t   xfers = int(objv[4]);

   return std::make_tuple(addr, amod, xfers);
}
/**
 *  getBlockCountReadParams
 * 
 *   Block count read operatios (operations that read a count for a subsequent transfer), 
 * look like this:  mvlclist <operation> address mask address-modifier
 * 
 * This method decodes the command line into a tuple containig, in order, tha ddress, mask and address 
 * modifier/
 * 
 * @param objv - the command words.
 * @return std::tuple<uint32_t, uint32_t, uint8_t> - the address, field mask and address modifier.
 * @throw CTCLException - if conversions fail.
 * @throw std::string - the parameter count is incorrect.
 * @throw any exception from decodeAmod above.
 */
std::tuple<uint32_t, uint32_t, uint8_t> 
VMUSBListCommand::getBlockCountReadParams(std::vector<CTCLObject>& objv) {
   requireExactly(
      objv, 5, 
      "Block read count reads require only the address, field mask and address modifier"
   );

   uint32_t addr = int(objv[2]);
   uint32_t mask = int(objv[3]);
   uint8_t  amod = decodeAmod(objv[4]);

   return std::make_tuple(addr, mask, amod);
}

/**
 *  getLoopUntilParams
 *   Loop until operations look like this:  mvlclist <loop-until-op> addr amod mask value
 *   This method decodes these items from the command.
 * 
 * @param objv - the words in the command line.
 * @return std::tuple<uint32_t, uint8_t, uint32_t, uint32_t> in order the address, amod, mask and value.
 * @throw CTCLException - if conversions fail.
 * @throw std::string - the parameter count is incorrect.
 * @throw any exception from decodeAmod above.
 */
std::tuple<uint32_t, uint8_t, uint32_t, uint32_t> 
VMUSBListCommand::getLoopUntilParams(std::vector<CTCLObject>& objv) {
   requireExactly(
      objv, 6,
      "Loop until operations require exactly the address, modifier,  field mask and value"
   );

   uint32_t addr = int(objv[2]);
   uint8_t  amod = decodeAmod(objv[3]);
   uint32_t mask = int(objv[4]);
   uint32_t value = int(objv[5]);

   return std::make_tuple(addr, amod, mask, value);
}
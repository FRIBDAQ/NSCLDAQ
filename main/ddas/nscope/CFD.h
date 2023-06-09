#ifndef CFD_H_
#define CFD_H_
#include "Table.h"
#include <config.h>

class CFD: public Table
{
public:
	CFD(const TGWindow * p, const TGWindow * main, 
	    /*char **/string name, int columns=5, int rows=16, int NumModules=13);
	virtual ~CFD();
	Bool_t ProcessMessage (Long_t msg, Long_t parm1, Long_t parm2);
	int change_values (Long_t mod);
	int load_info (Long_t mod);
	void SetModuleNumber(int moduleNr) {
	  modNumber = (short)moduleNr;
	  numericMod->SetIntNumber(modNumber);
	};
	void setDisable(bool value);

	short int chanNumber;
	short int modNumber; 
	TGNumberEntry* chanCopy;
	bool Load_Once;
	char tmp[10];
	float cfddelay,cfdscale, cfdthresh;
	float cfdresetdelay;

};

#endif /*CFD_H_*/

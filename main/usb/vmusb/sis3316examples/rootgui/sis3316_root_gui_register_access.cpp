/***************************************************************************/
/*                                                                         */
/*  Filename: sis3316_root_gui_register_access.cpp                         */
/*                                                                         */
/*  Funktion:                                                              */
/*                                                                         */
/*  Autor:                TH                                               */
/*  date:                 06.09.2021                                       */
/*  last modification:    10.09.2021                                       */
/*                                                                         */
/* ----------------------------------------------------------------------- */
/*                                                                         */
/*  SIS  Struck Innovative Systeme GmbH                                    */
/*                                                                         */
/*  Harksheider Str. 102A                                                  */
/*  22399 Hamburg                                                          */
/*                                                                         */
/*  Tel. +49 (0)40 60 87 305 0                                             */
/*  Fax  +49 (0)40 60 87 305 20                                            */
/*                                                                         */
/*  https://www.struck.de                                                  */
/*                                                                         */
/*  © 2021                                                                 */
/*                                                                         */
/***************************************************************************/


#include "sis3316_root_gui_register_access.h"
#include "sis3316_class.h"
//#include "sis3316_ethernet_access_class.h"

extern sis3316_adc* gl_sis3316_adc1;

#define BTN_FPGA_BIN_BROWSE     10
#define BTN_VERIFY_FPGA_FLASH   11
#define BTN_PROG_FPGA_FLASH     12
#define BTN_PROG_FPGA_EXIT      13

#define BTN_REGISTER_READ       10
#define BTN_REGISTER_WRITE      11
#define BTN_REGISTER_EXIT       12
#define ENTRY_READ_ADDR			13
#define ENTRY_WRITE_ADDR		14

sis3316_adc* class_sis3316_adc_device;


sis3316_root_gui_register_access::sis3316_root_gui_register_access(const TGWindow* p, void* sis3316_adc_device, UInt_t w, UInt_t h, Bool_t* open_window_flag) : TGMainFrame(p, w, h)
//sis3316_root_gui_register_access::sis3316_root_gui_register_access(const TGWindow* p, sis3316_adc* sis3316_adc_device, UInt_t w, UInt_t h, Bool_t* open_window_flag) : TGMainFrame(p, w, h)
//sis3316_root_gui_register_access::sis3316_root_gui_register_access(const TGWindow* p, UInt_t w, UInt_t h, Bool_t* open_window_flag) : TGMainFrame(p, w, h)
{

	int i;
	//unsigned int addr;
	char s[128];
	//class_sis3316_adc_device = (sis3316_adc*)sis3316_adc_device;
	class_sis3316_adc_device = gl_sis3316_adc1;
	// use hierarchical cleaning
	SetCleanup(kDeepCleanup);
	if(open_window_flag){
		fBopen_window_flag = open_window_flag;
		*fBopen_window_flag = kTRUE;
	}
	// main window icon and general setup
	SetWindowName("SIS3316 - Register Access");
	SetIconPixmap("sis1.png");
 

	// vertical frame
	fVF = new TGVerticalFrame(this, GetDefaultWidth(), GetDefaultHeight());
	AddFrame(fVF, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
	this->SetCleanup(kDeepCleanup);

	TGLabel* fLabel_tips[7];


 

	//*****************
	fGF_Tips = new TGGroupFrame(fVF, "Tip");
	fVF->AddFrame(fGF_Tips, new TGLayoutHints(kLHintsExpandX, 5, 5, 25, 10)); // hints, left, right, top, bottom

	for (i = 0; i < 7; i++) {
		fLabel_tips[i] = new TGLabel(fGF_Tips, " ");
		fLabel_tips[i]->SetTextJustify(kTextLeft + kTextCenterX);
		fLabel_tips[i]->SetMargins(0, 0, 0, 0);
		fLabel_tips[i]->SetWrapLength(-1);
		//fGF_Tips->AddFrame(fLabel_tips[i], new TGLayoutHints(kLHintsExpandX, 2, 2, 5, 2));
	}
	fGF_Tips->AddFrame(fLabel_tips[0], new TGLayoutHints(kLHintsExpandX, 2, 2, 5, 2));
	sprintf(s, "The up/down cursor keys increments/decrements the ");
	fLabel_tips[0]->SetText(s);

	fGF_Tips->AddFrame(fLabel_tips[1], new TGLayoutHints(kLHintsExpandX, 2, 2, 2, 2));
	sprintf(s, "numerical values. ");
	fLabel_tips[1]->SetText(s);

	fGF_Tips->AddFrame(fLabel_tips[2], new TGLayoutHints(kLHintsExpandX, 2, 2, 5, 2));
	sprintf(s, "The step size can be selected with control and shift keys:");
	fLabel_tips[2]->SetText(s);

	fGF_Tips->AddFrame(fLabel_tips[3], new TGLayoutHints(kLHintsExpandX, 2, 2, 4, 2));
	sprintf(s, "--:                         small step (1) ");
	fLabel_tips[3]->SetText(s);

	fGF_Tips->AddFrame(fLabel_tips[4], new TGLayoutHints(kLHintsExpandX, 2, 2, 2, 2));
	sprintf(s, "shift:                    medium step (10 units)");
	fLabel_tips[4]->SetText(s);

	fGF_Tips->AddFrame(fLabel_tips[5], new TGLayoutHints(kLHintsExpandX, 2, 2, 2, 2));
	sprintf(s, "control:               large step (100 units)");
	fLabel_tips[5]->SetText(s);

	fGF_Tips->AddFrame(fLabel_tips[6], new TGLayoutHints(kLHintsExpandX, 2, 2, 2, 5));
	sprintf(s, "shift-control:      huge step (1000 units)");
	fLabel_tips[6]->SetText(s);


//*****************

	fHF = new TGHorizontalFrame(fVF, 200, 100);
	fVF->AddFrame(fHF, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));

	/* Register Read */
	fGF = new TGGroupFrame(fHF, "Register Read");
	fHF->AddFrame(fGF, new TGLayoutHints(kLHintsTop | kLHintsNormal, 5, 5, 20, 5)); // hints, left, right, top, bottom

	fLblDescription = new TGLabel(fGF, "Address (hex)");
	fGF->AddFrame(fLblDescription, new TGLayoutHints(kLHintsNormal, 0, 0, 10, 0));
	fNuEnRdAddr = new TGNumberEntry(fGF, 0 /* value */, 8 /* width */, ENTRY_READ_ADDR /* irq */, (TGNumberFormat::kNESHex));
	fNuEnRdAddr->SetButtonToNum(kTRUE);
	this->register_read_addr = 4;
	fNuEnRdAddr->SetHexNumber(this->register_read_addr);
	//fNuEnRdAddr->IncreaseNumber(TGNumberFormat::EStepSize::kNSSHuge, 4, kFALSE); // (step, sign, logstep)
	fNuEnRdAddr->Associate(this);
	fGF->AddFrame(fNuEnRdAddr, new TGLayoutHints(kLHintsExpandX, 0, 0, 5, 0));

	fLblDescription = new TGLabel(fGF, "Value (hex)");
	fGF->AddFrame(fLblDescription, new TGLayoutHints(kLHintsNormal, 0, 0, 10, 0));
	fNuEnRdValue = new TGNumberEntry(fGF, 0 /* value */, 8 /* width */, 999 /* irq */, (TGNumberFormat::kNESHex));
	fNuEnRdValue->SetButtonToNum(kFALSE);
	fNuEnRdValue->Associate(this);
	fNuEnRdValue->SetHexNumber(0);
	fGF->AddFrame(fNuEnRdValue, new TGLayoutHints(kLHintsExpandX, 0, 0, 5, 0));

	fButRead = new TGTextButton(fGF, "READ", BTN_REGISTER_READ);
	fButRead->ChangeOptions(fButRead->GetOptions() | kFixedWidth);
	fButRead->SetWidth(120);
	fButRead->Associate(this);
	fGF->AddFrame(fButRead, new TGLayoutHints(kLHintsExpandX | kLHintsCenterX, 5, 5, 15, 5));




	/* Register Write */
	fGF = new TGGroupFrame(fHF, "Register Write");
	fHF->AddFrame(fGF, new TGLayoutHints(kLHintsTop | kLHintsNormal, 5, 5, 20, 5)); // hints, left, right, top, bottom

	fLblDescription = new TGLabel(fGF, "Address (hex)");
	fGF->AddFrame(fLblDescription, new TGLayoutHints(kLHintsNormal, 0, 0, 10, 0));
	fNuEnWrAddr = new TGNumberEntry(fGF, 0 /* value */, 8 /* width */, ENTRY_WRITE_ADDR /* irq */, (TGNumberFormat::kNESHex));
	fGF->AddFrame(fNuEnWrAddr, new TGLayoutHints(kLHintsExpandX, 0, 0, 5, 0));
	this->register_write_addr = 0;
	fNuEnWrAddr->SetHexNumber(this->register_write_addr);
	fNuEnWrAddr->Associate(this);

	fLblDescription = new TGLabel(fGF, "Value (hex)");
	fGF->AddFrame(fLblDescription, new TGLayoutHints(kLHintsNormal, 0, 0, 10, 0));
	fNuEnWrValue = new TGNumberEntry(fGF, 0 /* value */, 8 /* width */, 999 /* irq */, (TGNumberFormat::kNESHex));
	fGF->AddFrame(fNuEnWrValue, new TGLayoutHints(kLHintsExpandX, 0, 0, 5, 0));
	fNuEnWrValue->SetHexNumber(0);

	fButWrite = new TGTextButton(fGF, "WRITE", BTN_REGISTER_WRITE);
	fButWrite->ChangeOptions(fButWrite->GetOptions() | kFixedWidth);
	fButWrite->SetWidth(120);
	fButWrite->Associate(this);
	fGF->AddFrame(fButWrite, new TGLayoutHints(kLHintsExpandX | kLHintsCenterX, 5, 5, 15, 5));


	fButExit = new TGTextButton(fVF, "&Exit", BTN_REGISTER_EXIT);
	fButExit->ChangeOptions(fButExit->GetOptions() | kFixedWidth);
	fButExit->SetWidth(120);
	fButExit->Associate(this);
	fButExit->SetEnabled(kTRUE);

	fVF->AddFrame(fButExit, new TGLayoutHints(kLHintsCenterX, 5, 5, 10, 15));


	
	// draw everything
	SetWMSizeHints(w, h, w, h, 1, 1);
	//Resize(GetDefaultSize());   // resize to default size
	Resize(350, 320);
	MapSubwindows();
	MapWindow();
 
}


Bool_t sis3316_root_gui_register_access::ProcessMessage(Long_t msg, Long_t parm1, Long_t c)
{
	uint32_t addr;
	uint32_t data;

	switch(GET_MSG(msg))
	{
		case kC_COMMAND:
			switch (GET_SUBMSG(msg))
			{
			case kCM_BUTTON:
				//printf("\sis3316_root_gui_register_access::ProcessMessage:case kC_COMMAND-kCM_BUTTON parm1 = %d \n",parm1);
				switch (parm1) {
				case BTN_REGISTER_READ:
					addr = (uint32_t)fNuEnRdAddr->GetHexNumber() & 0xfffffffc; // only addresses on a 4-byte boundary are valid
					fNuEnRdAddr->SetHexNumber(addr);
//					gl_sis3316_adc1->register_read(addr, &data);
					class_sis3316_adc_device->register_read(addr, &data);
					fNuEnRdValue->SetHexNumber(data);
					break;

				case BTN_REGISTER_WRITE:
					addr = (uint32_t)fNuEnWrAddr->GetHexNumber() & 0xfffffffc; // only addresses on a 4-byte boundary are valid
					fNuEnWrAddr->SetHexNumber(addr);
					class_sis3316_adc_device->register_write(addr, (uint32_t)fNuEnWrValue->GetHexNumber());
					break;

				case BTN_REGISTER_EXIT:
					CloseWindow();
					break;

				default:
					break;
				}
				break;
			}
			break;
			

		case kC_TEXTENTRY:
			//printf("kC_TEXTENTRY item %ld activated\n", parm1);
			switch (GET_SUBMSG(msg)) {
			case kTE_TEXTCHANGED:
				switch (parm1) {

				case ENTRY_READ_ADDR: // Read Addr
					addr = (uint32_t)fNuEnRdAddr->GetHexNumber();
					if (addr == (this->register_read_addr + 1)) { // increment by one
						addr = (addr + 3) & 0xfffffffc; // only addresses on a 4-byte boundary are valid
					}
					if (addr == (this->register_read_addr - 1)) { // decrement by one
						addr = (addr ) & 0xfffffffc; // only addresses on a 4-byte boundary are valid
					}
					this->register_read_addr = addr;
					fNuEnRdAddr->SetHexNumber(addr);
					addr = (uint32_t)fNuEnRdAddr->GetHexNumber();
					break;


				case ENTRY_WRITE_ADDR: // Write Addr
					addr = (uint32_t)fNuEnWrAddr->GetHexNumber();
					if (addr == (this->register_write_addr + 1)) { // increment by one
						addr = (addr + 3) & 0xfffffffc; // only addresses on a 4-byte boundary are valid
					}
					if (addr == (this->register_write_addr - 1)) { // decrement by one
						addr = (addr ) & 0xfffffffc; // only addresses on a 4-byte boundary are valid
					}
					this->register_write_addr = addr;
					fNuEnWrAddr->SetHexNumber(addr);
					addr = (uint32_t)fNuEnWrAddr->GetHexNumber();
					break;

				default:
					break;

				}
			}



		default:
			break;

	}




	return kTRUE;
}



#ifdef raus
void sis3316_udp_properties::updateBar(int percent){
}

void sis3316_udp_properties::progressProgCallback(int percent){
	ptr->fBar_prog->SetPosition((float)percent);
	gSystem->ProcessEvents();  // handle GUI events
}
void sis3316_udp_properties::progressVerifyCallback(int percent){
	ptr->fBar_verify->SetPosition((float)percent);
	gSystem->ProcessEvents();  // handle GUI events
}
#endif


void sis3316_root_gui_register_access::CloseWindow()
{
	*fBopen_window_flag = kFALSE;
	DeleteWindow();
}
sis3316_root_gui_register_access::~sis3316_root_gui_register_access(void)
{
	*fBopen_window_flag = kFALSE;
	DeleteWindow();
}

/***************************************************************************/
/*                                                                         */
/*  Filename: sis3316_root_gui_udp_properties.cpp                          */
/*                                                                         */
/*  Funktion:                                                              */
/*                                                                         */
/*  Autor:                TH                                               */
/*  date:                 06.09.2021                                       */
/*  last modification:    29.10.2021                                       */
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


#include "sis3316_root_gui_udp_properties.h"
#include "sis3316_class.h"

extern sis3316_adc* gl_sis3316_adc1;


sis3316_udp_properties::sis3316_udp_properties(const TGWindow *p, UInt_t w, UInt_t h, Bool_t *b) : TGMainFrame(p, w, h)
{

	int i;
	unsigned int data;

	// use hierarchical cleaning
	SetCleanup(kDeepCleanup);
	if(b){
		fBSetup = b;
		*fBSetup = kTRUE;
	}
	// main window icon and general setup
	SetWindowName("SIS3316 - UDP Properties");
	SetIconPixmap("sis1.png");
	fClient->GetColorByName("green", this->green);
	fClient->GetColorByName("red", this->red);
	reboot_prevent_flag = 0;


	// vertical frame
	fVert1 = new TGVerticalFrame(this, GetDefaultWidth(), GetDefaultHeight());
	AddFrame(fVert1, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));


//*****************


	// DHCP selection
	fGrpDHCP = new TGGroupFrame(fVert1, "DHCP selection");  // hints, left, right, top, bottom
	fVert1->AddFrame(fGrpDHCP, new TGLayoutHints(kLHintsExpandX, 5, 5, 20, 5));

	fHor1 = new TGHorizontalFrame(fGrpDHCP, 200, 30);
	fGrpDHCP->AddFrame(fHor1, new TGLayoutHints(kLHintsExpandX, 2, 2, 12, 2));

	fCombo_SetSelect_DHCP_Mode = new TGComboBox(fHor1, SIS3316UdpDialog_kCM_COMBOBOX_IRQ_NO_30);
	fHor1->AddFrame(fCombo_SetSelect_DHCP_Mode, new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 3, 2, 10, 10));// hints, left, right, top, bottom
	for (i = 0; i < 3; i++) {
		fCombo_SetSelect_DHCP_Mode->AddEntry(entry_Select_DHCP_Mode[i], i);
	}

	gl_sis3316_adc1->register_read(SIS3316_SERIAL_NUMBER_REG, &data);
	this->dhcp_option = (data & 0xff000000) >> 24;
	if (this->dhcp_option > 2) {
		this->dhcp_option = 0;
	}
	fCombo_SetSelect_DHCP_Mode->Select(this->dhcp_option, kTRUE); //  Clear DHCP option value (DHCP mode depends on Switch SW80-4)
	fCombo_SetSelect_DHCP_Mode->Resize(430, 25);

	fCombo_SetSelect_DHCP_Mode->Associate(this); // Event (IRQ) anmelden

	printf("\read dhcp_option = %d \n", this->dhcp_option);



	fGrpReboot = new TGGroupFrame(fVert1, "Save and Reboot");  // hints, left, right, top, bottom
	fVert1->AddFrame(fGrpReboot, new TGLayoutHints(kLHintsExpandX, 5, 5, 20, 5));

	fButReboot = new TGTextButton(fGrpReboot, "Save DHCP option and Reboot SIS3316 FPGAs", SIS3316UdpDialog_kCM_BUTTON_IRQ_NO_30);
	fButReboot->Associate(this);
	//fButReboot->ChangeBackground(this->red);
	fButReboot->ChangeBackground(GetDefaultFrameBackground());
	reboot_prevent_flag = 1;
	fGrpReboot->AddFrame(fButReboot, new TGLayoutHints(kLHintsExpandX, 5, 5, 10, 15));

	fLabel_reboot_note = new TGLabel(fGrpReboot,"Note: the watchdog has to be enabled -> SW80-7 ON ");
	fLabel_reboot_note->SetTextJustify(kTextLeft + kTextCenterX );
	fLabel_reboot_note->SetMargins(0,0,0,0);
	fLabel_reboot_note->SetWrapLength(-1);
	fGrpReboot->AddFrame(fLabel_reboot_note, new TGLayoutHints(kLHintsExpandX,2,2,15,2));

	// draw everything
	SetWMSizeHints(w, h, w, h, 1, 1);
	Resize(GetDefaultSize());   // resize to default size
	MapSubwindows();
	MapWindow();
}



Bool_t sis3316_udp_properties::ProcessMessage(Long_t msg, Long_t parm1, Long_t c)
{
	unsigned char uchar_data[32];
 	int rc;

	switch(GET_MSG(msg))
	{
	case kC_COMMAND:
		switch(GET_SUBMSG(msg))
		{
			case kCM_BUTTON:
				//printf("\SIS3316UdpDialog::ProcessMessage:case kC_COMMAND;kCM_CHECKBUTTON parm1 = %d \n",parm1);
				switch (parm1) {
					case SIS3316UdpDialog_kCM_BUTTON_IRQ_NO_30:
						if (reboot_prevent_flag == 0) {

							this->dhcp_option = fCombo_SetSelect_DHCP_Mode->GetSelected();
							printf("write dhcp_option = %d \n", this->dhcp_option);

							uchar_data[0] = this->dhcp_option;
							rc = gl_sis3316_adc1->write_ow_dhcp_option(uchar_data);
							//printf("write_ow_dhcp_option = 0x%02x      rc = 0X%08x   \n", uchar_data[0], rc);
							usleep(100);
							fButReboot->ChangeBackground(GetDefaultFrameBackground());
							//fButReboot->ChangeBackground(this->red);
							reboot_prevent_flag = 1;
							usleep(100);
							gl_sis3316_adc1->register_write(SIS3316_INTERFACE_ACCESS_ARBITRATION_CONTROL, 0x80000000); // kill other "Interface Grant"
							gl_sis3316_adc1->register_write(SIS3316_INTERFACE_ACCESS_ARBITRATION_CONTROL, 0x1); // set own "Interface Grant"
							gl_sis3316_adc1->register_write(SIS3316_CONTROL_STATUS, 0x8000); // reboot FPGAs ; switch SW80-7 has to be on (Watchdog)
						}
						break;

					default:
						break;
				}
				break; // kCM_BUTTON
				
			case kCM_COMBOBOX:
				//printf("\SIS3316UdpDialog::ProcessMessage:case kC_COMMAND;kCM_COMBOBOX parm1 = %d \n", parm1);
				switch (parm1) {
					case SIS3316UdpDialog_kCM_COMBOBOX_IRQ_NO_30:
						fButReboot->ChangeBackground(this->green);
						reboot_prevent_flag = 0;
						break;
					default:
						break;
				}
				break;// kCM_COMBOBOX
		}
		//
		break; //kC_COMMAND
#ifdef not_used_yet
	case kC_TEXTENTRY:
		//printf("kC_TEXTENTRY item %ld activated\n", parm1);
		switch (GET_SUBMSG(msg)) {
			case kTE_TEXTCHANGED:
				switch (parm1) {

					default:
						break;
				}
				break;
		}
		break; //kC_TEXTENTRY
#endif

	}

	return kTRUE;
}


sis3316_udp_properties::~sis3316_udp_properties(void)
{
	*fBSetup = kFALSE;
}

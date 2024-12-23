/***************************************************************************/
/*                                                                         */
/*  Filename: sis3316_root_gui_flash.cpp                                   */
/*                                                                         */
/*  Funktion:                                                              */
/*                                                                         */
/*  Autor:                TH                                               */
/*  date:                 27.03.2015                                       */
/*  last modification:    21.08.2020                                       */
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
/*  http://www.struck.de                                                   */
/*                                                                         */
/*  © 2020                                                                 */
/*                                                                         */
/***************************************************************************/


#include "sis3316_root_gui_flash.h"
#include "sis3316_class.h"


extern sis3316_adc *gl_sis3316_adc1 ;

sis3316_flash *ptr;


enum BtnIdents{
	BTN_VME_BROWSE,
	BTN_VME_PROG,
	BTN_ADC1_BROWSE,
	BTN_ADC2_BROWSE,
	BTN_ADC3_BROWSE,
	BTN_ADC4_BROWSE,
	BTN_ADC_PROG,
	BTN_REBOOT
};

sis3316_flash::sis3316_flash(const TGWindow *p, UInt_t w, UInt_t h, Bool_t *b) : TGMainFrame(p, w, h)
{
	// use hierarchical cleaning
	SetCleanup(kDeepCleanup);
	if(b){
		fBSetup = b;
		*fBSetup = kTRUE;
	}
	// main window icon and general setup
	SetWindowName("SIS3316 - Flash Programmer");
	SetIconPixmap("sis1.png");

	fClient->GetColorByName("green", this->green);
	fClient->GetColorByName("red", this->red);
	reboot_prevent_flag = 0;

	// vertical frame
	fVert1 = new TGVerticalFrame(this, GetDefaultWidth(), GetDefaultHeight());
	AddFrame(fVert1, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));


	// VME FPGA Programming section
	fGrpVME = new TGGroupFrame(fVert1, "VME FPGA Programming");  // hints, left, right, top, bottom
	fVert1->AddFrame(fGrpVME, new TGLayoutHints(kLHintsExpandX, 5, 5, 20, 5));
	
	fLblVMEImg = new TGLabel(fGrpVME, "VME FPGA Image");
	fGrpVME->AddFrame(fLblVMEImg, new TGLayoutHints(kLHintsNormal, 0, 0, 10, 5));
	
	fHor1 = new TGHorizontalFrame(fGrpVME);
	fGrpVME->AddFrame(fHor1, new TGLayoutHints(kLHintsExpandX));

	fVMEImgPath = new TGTextEntry(fHor1, "test VME Image.bin");
	fHor1->AddFrame(fVMEImgPath, new TGLayoutHints(kLHintsNormal, 5, 5, 5, 5));
	fVMEImgPath->SetWidth(240);
	
	fButVMEBrowse = new TGTextButton(fHor1, "...", BTN_VME_BROWSE);
	fButVMEBrowse->ChangeOptions(fButVMEBrowse->GetOptions() | kFixedWidth);
	fButVMEBrowse->SetWidth(50);
	fButVMEBrowse->Associate(this);
	fHor1->AddFrame(fButVMEBrowse, new TGLayoutHints(kLHintsRight, 5, 5, 5, 5));

	fButVMEProg = new TGTextButton(fGrpVME, "Program VME FPGA", BTN_VME_PROG);
	fButVMEProg->Associate(this);
	fGrpVME->AddFrame(fButVMEProg, new TGLayoutHints(kLHintsExpandX, 5, 5, 10, 15));



	// ADC FPGA Programming section
	fGrpADC = new TGGroupFrame(fVert1, "ADC FPGA Programming");
	//fVert1->AddFrame(fGrpADC, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 5, 5, 10, 5));
	fVert1->AddFrame(fGrpADC, new TGLayoutHints(kLHintsExpandX, 5, 5, 20, 5));
	
	// ADC1
	fLblADC1Img = new TGLabel(fGrpADC, "ADC FPGA Image");
	fGrpADC->AddFrame(fLblADC1Img, new TGLayoutHints(kLHintsNormal, 0, 0, 10, 5));
	
	fHorADC1 = new TGHorizontalFrame(fGrpADC);
	fGrpADC->AddFrame(fHorADC1, new TGLayoutHints(kLHintsExpandX));

	fADC1ImgPath = new TGTextEntry(fHorADC1, "test ADC Image.bin");
	fHorADC1->AddFrame(fADC1ImgPath, new TGLayoutHints(kLHintsNormal, 5, 5, 5, 5));
	fADC1ImgPath->SetWidth(240);
	
	fButADC1Browse = new TGTextButton(fHorADC1, "...", BTN_ADC1_BROWSE);
	fButADC1Browse->ChangeOptions(fButADC1Browse->GetOptions() | kFixedWidth);
	fButADC1Browse->SetWidth(50);
	fButADC1Browse->Associate(this);
	fHorADC1->AddFrame(fButADC1Browse, new TGLayoutHints(kLHintsRight, 5, 5, 5, 5));

	fButADCProg = new TGTextButton(fGrpADC, "Program ADC FPGAs", BTN_ADC_PROG);
	fGrpADC->AddFrame(fButADCProg, new TGLayoutHints(kLHintsExpandX, 5, 5, 10, 15));
	fButADCProg->Associate(this);

#ifdef NOT_IMPLEMENTED	
	// ADC2
	fLblADC2Img = new TGLabel(fGrpADC, "ADC2 FPGA Image");
	fGrpADC->AddFrame(fLblADC2Img, new TGLayoutHints(kLHintsNormal, 0, 0, 5, 0));

	fHorADC2 = new TGHorizontalFrame(fGrpADC);
	fGrpADC->AddFrame(fHorADC2, new TGLayoutHints(kLHintsExpandX));

	fADC2ImgPath = new TGTextEntry(fHorADC2, "test ADC2 Image.bin");
	fHorADC2->AddFrame(fADC2ImgPath, new TGLayoutHints(kLHintsNormal, 5, 5, 5, 5));
	fADC2ImgPath->SetWidth(187);

	fButADC2Browse = new TGTextButton(fHorADC2, "...", BTN_ADC2_BROWSE);
	fButADC2Browse->ChangeOptions(fButADC2Browse->GetOptions() | kFixedWidth);
	fButADC2Browse->SetWidth(50);
	fButADC2Browse->Associate(this);
	fHorADC2->AddFrame(fButADC2Browse, new TGLayoutHints(kLHintsNormal, 5, 5, 5, 5));
	
	// ADC3
	fLblADC3Img = new TGLabel(fGrpADC, "ADC3 FPGA Image");
	fGrpADC->AddFrame(fLblADC3Img, new TGLayoutHints(kLHintsNormal, 0, 0, 5, 0));

	fHorADC3 = new TGHorizontalFrame(fGrpADC);
	fGrpADC->AddFrame(fHorADC3, new TGLayoutHints(kLHintsExpandX));

	fADC3ImgPath = new TGTextEntry(fHorADC3, "test ADC2 Image.bin");
	fHorADC3->AddFrame(fADC3ImgPath, new TGLayoutHints(kLHintsNormal, 5, 5, 5, 5));
	fADC3ImgPath->SetWidth(187);

	fButADC3Browse = new TGTextButton(fHorADC3, "...", BTN_ADC3_BROWSE);
	fButADC3Browse->ChangeOptions(fButADC3Browse->GetOptions() | kFixedWidth);
	fButADC3Browse->SetWidth(50);
	fButADC3Browse->Associate(this);
	fHorADC3->AddFrame(fButADC3Browse, new TGLayoutHints(kLHintsNormal, 5, 5, 5, 5));
	
	// ADC4
	fLblADC4Img = new TGLabel(fGrpADC, "ADC4 FPGA Image");
	fGrpADC->AddFrame(fLblADC4Img, new TGLayoutHints(kLHintsNormal, 0, 0, 5, 0));

	fHorADC4 = new TGHorizontalFrame(fGrpADC);
	fGrpADC->AddFrame(fHorADC4, new TGLayoutHints(kLHintsExpandX));

	fADC4ImgPath = new TGTextEntry(fHorADC4, "test ADC4 Image.bin");
	fHorADC4->AddFrame(fADC4ImgPath, new TGLayoutHints(kLHintsNormal, 5, 5, 5, 5));
	fADC4ImgPath->SetWidth(187);

	fButADC4Browse = new TGTextButton(fHorADC4, "...", BTN_ADC4_BROWSE);
	fButADC4Browse->ChangeOptions(fButADC4Browse->GetOptions() | kFixedWidth);
	fButADC4Browse->SetWidth(50);
	fButADC4Browse->Associate(this);
	fHorADC4->AddFrame(fButADC4Browse, new TGLayoutHints(kLHintsNormal, 5, 5, 5, 5));
	

	fChkADCImgSelect = new TGCheckButton(fGrpADC, "ADC FPGAs have unique images");
	fChkADCImgSelect->SetState(kButtonDown)   ; // is ON !
	fChkADCImgSelect->SetEnabled(kFALSE); //
	//fChkADCImgSelect->Associate(this);
	fGrpADC->AddFrame(fChkADCImgSelect);

#endif	
	//fGrpProgram = new TGGroupFrame(fVert1, "Programming");
	//fVert1->AddFrame(fGrpProgram, new TGLayoutHints(kLHintsNormal | kLHintsExpandX | kLHintsExpandY, 10, 10, 0, 0));

	fGrpBarProg = new TGGroupFrame(fVert1, "Programming progress");  // hints, left, right, top, bottom
	fVert1->AddFrame(fGrpBarProg, new TGLayoutHints(kLHintsExpandX, 5, 5, 20, 5));


	fBar_prog = new TGHProgressBar(fGrpBarProg);
	fBar_prog->ShowPosition(kTRUE, kTRUE);
	fBar_prog->SetBarColor("blue");
	fBar_prog->SetForegroundColor(0x000000);
	fGrpBarProg->AddFrame(fBar_prog, new TGLayoutHints(kLHintsNormal | kLHintsExpandX, 10, 10, 20, 10)); // hints, left, right, top, bottom


	fGrpBarVerify = new TGGroupFrame(fVert1, "Verifying progress");  // hints, left, right, top, bottom
	fVert1->AddFrame(fGrpBarVerify, new TGLayoutHints(kLHintsExpandX, 5, 5, 20, 5));

	fBar_verify = new TGHProgressBar(fGrpBarVerify);
	fBar_verify->ShowPosition(kTRUE, kTRUE);
	fBar_verify->SetBarColor("blue");
	fBar_verify->SetForegroundColor(0x000000);
	fGrpBarVerify->AddFrame(fBar_verify, new TGLayoutHints(kLHintsNormal | kLHintsExpandX, 10, 10, 20, 20)); // hints, left, right, top, bottom

	fGrpReboot = new TGGroupFrame(fVert1, "Reboot");  // hints, left, right, top, bottom
	fVert1->AddFrame(fGrpReboot, new TGLayoutHints(kLHintsExpandX, 5, 5, 20, 5));

	fButReboot = new TGTextButton(fGrpReboot, "Reboot SIS3316 FPGAs", BTN_REBOOT);
	fButReboot->Associate(this);
	fButReboot->ChangeBackground(this->green);
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
	// initially hidden
#ifdef NOT_IMPLEMENTED	
	fGrpADC->HideFrame(fHorADC2);
	fGrpADC->HideFrame(fHorADC3);
	fGrpADC->HideFrame(fHorADC4);
	fGrpADC->HideFrame(fLblADC2Img);
	fGrpADC->HideFrame(fLblADC3Img);
	fGrpADC->HideFrame(fLblADC4Img);
#endif
	MapWindow();
 
}

Bool_t sis3316_flash::ProcessMessage(Long_t msg, Long_t parm1, Long_t c)
{
	int return_code ;
	static const char *gDefTypes[] = { "binary files", "*.bin",
									   "all files",    "*",
                                        0,             0 };
	switch(GET_MSG(msg))
	{
	case kC_COMMAND:
		switch(GET_SUBMSG(msg))
		{
		case kCM_BUTTON:
			switch(parm1){
				case BTN_VME_BROWSE:
					fiVME.fFileTypes = gDefTypes;
					new TGFileDialog(gClient->GetRoot(), this, kFDOpen, &fiVME);
					if(fiVME.fFilename){
						fVMEImgPath->SetText(fiVME.fFilename);
					}else{
						fVMEImgPath->SetText("");
					}
					break;

				case BTN_VME_PROG:
					if(fiVME.fFilename != ""){
						fBar_prog->SetBarColor(0x99ffff); // light blue
						ptr = this;
	 					return_code = gl_sis3316_adc1->update_firmware(fiVME.fFilename, 0, progressProgCallback);
						if(return_code != 0) {
							fBar_prog->SetBarColor(0xff6666); // light red
							fButReboot->ChangeBackground(this->red);
							reboot_prevent_flag = 1 ;
						}
						else {
							fBar_verify->SetBarColor(0x99ffff); // light blue
							return_code = gl_sis3316_adc1->verify_firmware(fiVME.fFilename, 0, progressVerifyCallback);
							if(return_code != 0) {
								fBar_verify->SetBarColor(0xff6666);
								fButReboot->ChangeBackground(this->red);
								reboot_prevent_flag = 1 ;
							}
							else {
								fBar_verify->SetBarColor(0x00ff00); // green
								fButReboot->ChangeBackground(this->green);
								reboot_prevent_flag = 0 ;
							}
						}

					}
					else {
						fBar_prog->SetBarColor(0xff6666);
						fButReboot->ChangeBackground(this->red);
						reboot_prevent_flag = 1 ;
					}
					break;


				case BTN_ADC1_BROWSE:
					fiADC1.fFileTypes = gDefTypes;
					new TGFileDialog(gClient->GetRoot(), this, kFDOpen, &fiADC1);
					if(fiADC1.fFilename){
						fADC1ImgPath->SetText(fiADC1.fFilename);
					}else{
						fADC1ImgPath->SetText("");
					}
					break;

				case BTN_ADC_PROG:
					if(fiADC1.fFilename != ""){
						fBar_prog->SetBarColor(0x99ffff); // light blue
						ptr = this;
						return_code = gl_sis3316_adc1->update_firmware(fiADC1.fFilename, 0x400000, progressProgCallback);
						if(return_code != 0) {
							fBar_prog->SetBarColor(0xff6666); // light red
							fButReboot->ChangeBackground(this->red);
							reboot_prevent_flag = 1 ;
						}
						else {
							fBar_verify->SetBarColor(0x99ffff); // light blue
	 						return_code = gl_sis3316_adc1->verify_firmware(fiADC1.fFilename, 0x400000, progressVerifyCallback);
							if(return_code != 0) {
								fBar_verify->SetBarColor(0xff6666);
								fButReboot->ChangeBackground(this->red);
								reboot_prevent_flag = 1 ;
							}
							else {
								fBar_verify->SetBarColor(0x00ff00); // green
								fButReboot->ChangeBackground(this->green);
								reboot_prevent_flag = 0 ;
							}
						}
					}
					else {
						fBar_prog->SetBarColor(0xff6666);
						fButReboot->ChangeBackground(this->red);
						reboot_prevent_flag = 1 ;
					}
					break;

				case BTN_REBOOT:
					if(reboot_prevent_flag == 0) {
						gl_sis3316_adc1->register_write(SIS3316_INTERFACE_ACCESS_ARBITRATION_CONTROL, 0x80000000); // kill other "Interface Grant"
						gl_sis3316_adc1->register_write(SIS3316_INTERFACE_ACCESS_ARBITRATION_CONTROL, 0x1); // set own "Interface Grant"
						gl_sis3316_adc1->register_write(SIS3316_CONTROL_STATUS, 0x8000); // reboot FPGAs ; switch SW80-7 has to be on (Watchdog)
					}
					break;
				}
	

			}
			//
			break;

#ifdef NOT_IMPLEMENTED	
		case kCM_CHECKBUTTON:
			if(fChkADCImgSelect->GetState() == kButtonUp){
				fGrpADC->HideFrame(fHorADC2);
				fGrpADC->HideFrame(fHorADC3);
				fGrpADC->HideFrame(fHorADC4);
				fGrpADC->HideFrame(fLblADC2Img);
				fGrpADC->HideFrame(fLblADC3Img);
				fGrpADC->HideFrame(fLblADC4Img);
			}else{
				fGrpADC->ShowFrame(fHorADC2);
				fGrpADC->ShowFrame(fHorADC3);
				fGrpADC->ShowFrame(fHorADC4);
				fGrpADC->ShowFrame(fLblADC2Img);
				fGrpADC->ShowFrame(fLblADC3Img);
				fGrpADC->ShowFrame(fLblADC4Img);
			}
			break;
#endif


	}
	return kTRUE;
}
 
void sis3316_flash::updateBar(int percent){
}

void sis3316_flash::progressProgCallback(int percent){
	ptr->fBar_prog->SetPosition((float)percent);
	gSystem->ProcessEvents();  // handle GUI events
}
void sis3316_flash::progressVerifyCallback(int percent){
	ptr->fBar_verify->SetPosition((float)percent);
	gSystem->ProcessEvents();  // handle GUI events
}

sis3316_flash::~sis3316_flash(void)
{
	*fBSetup = kFALSE;
}

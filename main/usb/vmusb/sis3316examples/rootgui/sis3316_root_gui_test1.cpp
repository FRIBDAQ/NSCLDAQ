//
/***************************************************************************/
/*                                                                         */
/*  Filename: sis3316_root_gui_test1.cpp                                   */
/*                                                                         */
/*  Funktion:                                                              */
/*                                                                         */
/*  Autor:                TH                                               */
/*  date:                 27.03.2015                                       */
/*  last modification:    08.03.2024 (SIS3316-2 adaptation)                */
/*                                                                         */
/* ----------------------------------------------------------------------- */
/*                                                                         */
/*  SIS  Struck Innovative Systeme GmbH                                    */
/*                                                                         */
/*  Harksheider Str. 102A                                                  */
/*  22399 Hamburg                                                          */
/*                                                                         */
/*  Tel. +49 (0)40 60 87 305 0                                             */
/*                                                                         */
/*  https://www.struck.de                                                  */
/*                                                                         */
/*  � 2024                                                                 */
/*                                                                         */
/***************************************************************************/

#include "project_system_define.h"		//define LINUX or WINDOWS
#include "project_interface_define.h"   //define Interface (sis1100/sis310x, sis3150usb or Ethernet UDP)

#include "sis3316_root_gui_test1.h"		// 
#include "sis3316_class.h"


#ifdef WINDOWS1
#pragma comment (lib, "libRio")
#pragma comment (lib, "libcore")
#pragma comment (lib, "libHist")
#pragma comment (lib, "libTree")
#pragma comment (lib, "libgpad")
//#pragma comment (lib, "libCint") remove with root_v6.xx.xx
#pragma comment (lib, "libGraf")
#pragma comment (lib, "libGui")
#endif

#ifdef LINUX
using namespace std;
//#include <time.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <linux/fs.h>
#include <time.h>
#include "sys/time.h"

#endif


#ifdef WINDOWS

using namespace std;
#include <iostream>
#include <iomanip>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
//#include <winsock2.h>

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h> // portable: uint64_t   MSVC: __int64 


int gettimeofday(struct timeval* tp, struct timezone* tzp)
{
	// Note: some broken versions only have 8 trailing zero's, the correct epoch has 9 trailing zero's
	// This magic number is the number of 100 nanosecond intervals since January 1, 1601 (UTC)
	// until 00:00:00 January 1, 1970 
	static const uint64_t EPOCH = ((uint64_t)116444736000000000ULL);

	SYSTEMTIME  system_time;
	FILETIME    file_time;
	uint64_t    time;

	GetSystemTime(&system_time);
	SystemTimeToFileTime(&system_time, &file_time);
	time = ((uint64_t)file_time.dwLowDateTime);
	time += ((uint64_t)file_time.dwHighDateTime) << 32;

	tp->tv_sec = (long)((time - EPOCH) / 10000000L);
	tp->tv_usec = (long)(system_time.wMilliseconds * 1000);
	return 0;
}
#endif


extern sis3316_adc *gl_sis3316_adc1 ;
 

/*******************************************************************************************************************************/
/*******************************************************************************************************************************/
// open Test dialog
SIS3316TestDialog::SIS3316TestDialog(const TGWindow *p, const TGWindow *main, UInt_t w,  UInt_t h, Bool_t *b, Bool_t  *run_flag, unsigned int uint_main_vme_base_addr, char* char_main_config_file, UInt_t options)
					   : TGTransientFrame(p, main, w, h, options)
{
  int i;
  unsigned int return_code ;
  unsigned int data;
  char s[128];

  this->uint_test_vme_base_addr = uint_main_vme_base_addr;

  // configuratin file ?
  if (char_main_config_file[0] == '\0') {
	  this->char_test_config_file[0] = '\0';
  }
  else {
	  strcpy(this->char_test_config_file, char_main_config_file); // 
  }


  this->SetCleanup(kDeepCleanup);
  this->SetIconPixmap("sis1_sis3316.png");

	params = new sis3316_get_configuration_parameters();
	fBSetup = b;
	*fBSetup = kTRUE;

	fBTest1_Run_Busy = run_flag;
	*fBTest1_Run_Busy = kFALSE;

	fB_openfCanvas1WindowFlag = kFALSE; // Raw data
	fB_openfCanvas2WindowFlag = kFALSE; // Histogram
	fB_openfCanvas3WindowFlag = kFALSE; // FFT
	fB_openfCanvas4WindowFlag = kFALSE; // MAW
//	fB_openfCanvas5WindowFlag = kFALSE; // Setup


/**********************************************************************************************************************/



	// allocating DMA Read Memory buffer
	this->dma_data_buffer = (unsigned int*)malloc(MAX_NUMBER_LWORDS_64MBYTE * 4);
	if (this->dma_data_buffer == NULL) {
		printf("Error allocating dma_data_buffer !\n");
	}

	// define ch buffer array -> divide dma_data_buffer in 16 buffers for Single Event Mode
	for (i = 0; i < SIS3316_CHANNEL_COUNT; i++) {
		this->ushort_adc_buffer_array_ptr[i] = (unsigned short*)&dma_data_buffer[i * SINGLE_EVENT_CH_BUFFER_LENGTH];
	}

	// allocating Root Graph X-,Y-buffer
	this->root_gr_x = (Int_t*)malloc(MAX_ROOT_PLOT_LENGTH * 4);
	if (this->root_gr_x == NULL) {
		printf("Error allocating Root Graph X buffer !\n");
	}
	for (i = 0; i < MAX_ROOT_PLOT_LENGTH; i++) {
		this->root_gr_x[i] = i;
	}

	this->root_gr_y = (Int_t*)malloc(MAX_ROOT_PLOT_LENGTH * 4);
	if (this->root_gr_y == NULL) {
		printf("Error allocating Root Graph Y buffer !\n");
	}
	for (i = 0; i < MAX_ROOT_PLOT_LENGTH; i++) {
		this->root_gr_y[i] = 0;
	}


	/****************************************************/

	// allocating Root MAW X-,Y-buffer
	this->root_gr_maw_x = (Int_t*)malloc(MAX_ROOT_PLOT_MAW_LENGTH * 4);
	if (this->root_gr_maw_x == NULL) {
		printf("Error allocating Root MAW X buffer !\n");
	}

	this->root_gr_maw_y = (Int_t*)malloc(MAX_ROOT_PLOT_MAW_LENGTH * 4);
	if (this->root_gr_maw_y == NULL) {
		printf("Error allocating Root MAW Y buffer !\n");
	}

	for (i = 0; i < MAX_ROOT_PLOT_MAW_LENGTH; i++) {
		this->root_gr_maw_x[i] = i;
		this->root_gr_maw_y[i] = 0;
	}


	/****************************************************/
	

	// allocating FFT-buffer
	this->root_float_fft_x = (float*)malloc((MAX_ROOT_PLOT_LENGTH / 2) * sizeof(float));
	if (this->root_float_fft_x == NULL) {
		printf("Error allocating root_float_fft_x buffer !\n");
	}
	this->root_float_fft_y = (float*)malloc((MAX_ROOT_PLOT_LENGTH / 2) * sizeof(float));
	if (this->root_float_fft_y == NULL) {
		printf("Error allocating root_float_fft_y buffer !\n");
	}
	this->root_float_fft_y1 = (float*)malloc((MAX_ROOT_PLOT_LENGTH / 2) * sizeof(float));
	if (this->root_float_fft_y1 == NULL) {
		printf("Error allocating root_float_fft_y1 buffer !\n");
	}


	for (i = 0; i < (MAX_ROOT_PLOT_LENGTH / 2) / 2; i++) {
		this->root_float_fft_x[i] = 0.0;
		this->root_float_fft_y[i] = 0.0;
	}
	for (i = 0; i < (MAX_ROOT_PLOT_LENGTH / 2) / 2; i++) {
		this->root_float_fft_y1[i] = 10.0;	// max y-axis: 
	}
	this->root_float_fft_y1[40] = -140.0;  // min y-axis: used for non Autoscale  

	/**** *****************************/

	this->root_double_window_weight = (double*)malloc((MAX_ROOT_PLOT_LENGTH) * sizeof(double));
	if (this->root_double_window_weight == NULL) {
		printf("Error allocating root_double_window_weight buffer !\n");
	}
	

	this->root_double_fft_spectrum = (double*)malloc((MAX_ROOT_PLOT_LENGTH) * sizeof(double));
	if (this->root_double_fft_spectrum == NULL) {
		printf("Error allocating root_double_fft_spectrum buffer !\n");
	}

	this->root_int_save_adc_buffer = (int*)malloc((MAX_ROOT_PLOT_LENGTH) * sizeof(int));
	if (this->root_int_save_adc_buffer == NULL) {
		printf("Error allocating root_int_save_adc_buffer buffer !\n");
	}

	for (i = 0; i < (MAX_ROOT_PLOT_LENGTH / 2) / 2; i++) {
		this->root_double_window_weight[i] = 0.0;
		this->root_double_fft_spectrum[i] = 0.0;
		this->root_int_save_adc_buffer[i] = 0 ;
	}

	/****************************************************/

	for (i = 0; i < 17; i++) {
		fGraph_ch[i] = new TGraph(MAX_ROOT_PLOT_LENGTH, this->root_gr_x, this->root_gr_y);
	}
	for (i = 0; i < 16; i++) {
		fGraph_Text_ch[i] = new TLatex(MAX_ROOT_PLOT_LENGTH, 10, "Ch ");;
	}

	fGraph_ch[16]->SetLineColor(DefineCanvasBackgroundColor);
	fGraph_ch[0]->SetLineColor(DefineChannel_1_Color);
	fGraph_ch[1]->SetLineColor(DefineChannel_2_Color);
	fGraph_ch[2]->SetLineColor(DefineChannel_3_Color);
	fGraph_ch[3]->SetLineColor(DefineChannel_4_Color);
	fGraph_ch[4]->SetLineColor(DefineChannel_5_Color);
	fGraph_ch[5]->SetLineColor(DefineChannel_6_Color);
	fGraph_ch[6]->SetLineColor(DefineChannel_7_Color);
	fGraph_ch[7]->SetLineColor(DefineChannel_8_Color);
	fGraph_ch[8]->SetLineColor(DefineChannel_9_Color);
	fGraph_ch[9]->SetLineColor(DefineChannel_10_Color);
	fGraph_ch[10]->SetLineColor(DefineChannel_11_Color);
	fGraph_ch[11]->SetLineColor(DefineChannel_12_Color);
	fGraph_ch[12]->SetLineColor(DefineChannel_13_Color);
	fGraph_ch[13]->SetLineColor(DefineChannel_14_Color);
	fGraph_ch[14]->SetLineColor(DefineChannel_15_Color);
	fGraph_ch[15]->SetLineColor(DefineChannel_16_Color);

	fGraph_Text_ch[0]->SetTextColor(DefineChannel_1_Color);
	fGraph_Text_ch[1]->SetTextColor(DefineChannel_2_Color);
	fGraph_Text_ch[2]->SetTextColor(DefineChannel_3_Color);
	fGraph_Text_ch[3]->SetTextColor(DefineChannel_4_Color);
	fGraph_Text_ch[4]->SetTextColor(DefineChannel_5_Color);
	fGraph_Text_ch[5]->SetTextColor(DefineChannel_6_Color);
	fGraph_Text_ch[6]->SetTextColor(DefineChannel_7_Color);
	fGraph_Text_ch[7]->SetTextColor(DefineChannel_8_Color);
	fGraph_Text_ch[8]->SetTextColor(DefineChannel_9_Color);
	fGraph_Text_ch[9]->SetTextColor(DefineChannel_10_Color);
	fGraph_Text_ch[10]->SetTextColor(DefineChannel_11_Color);
	fGraph_Text_ch[11]->SetTextColor(DefineChannel_12_Color);
	fGraph_Text_ch[12]->SetTextColor(DefineChannel_13_Color);
	fGraph_Text_ch[13]->SetTextColor(DefineChannel_14_Color);
	fGraph_Text_ch[14]->SetTextColor(DefineChannel_15_Color);
	fGraph_Text_ch[15]->SetTextColor(DefineChannel_16_Color);
 

// Histograms
	if (gl_sis3316_adc1->adc_125MHz_flag == 1) {
		this->root_histo_xmax_absolute = 0x10000;
	}
	else {
		this->root_histo_xmax_absolute = 0x4000;
	}
	this->root_histo_xmax = root_histo_xmax_absolute;
	this->root_histo_xmin = 0;



	for (i = 0; i < 16; i++) {
		iHistoAdc[i] = new TH1I(AdcHistogramLabel[i], AdcHistogramLabel[i], this->root_histo_xmax_absolute, 0, this->root_histo_xmax_absolute - 1);
		//iHistoAdc[i]->SetName("b");
		gStyle->SetStatFormat("6.6g");
		gStyle->SetStatH((Float_t)0.36);
		gStyle->SetStatW((Float_t)0.3);
		gStyle->SetStatX((Float_t)0.99);
		gStyle->SetStatY((Float_t)1.00);
	}
	for (i = 0; i < 16; i++) {
		histo_pave_text[i] = new TPaveText(0.1, 0.75, 0.40, 0.92, "brNDC"); //x1,y1,x2,y2
		histo_pave_text[i]->SetFillColor(DefineCanvasBackgroundColor);
		histo_pave_text[i]->SetTextAlign(12);
		histo_pave_text[i]->SetBorderSize(1);
	}



/**********************************************************************************************************************/

 

#ifndef RUN_WITHOUT_HARDWARE
	return_code = gl_sis3316_adc1->register_read( SIS3316_MODID, &data);
	if (return_code == 0) {
		printf("SIS3316_MODID                    = 0x%08x\n\n", data);

		gl_sis3316_adc1->register_write( SIS3316_ADC_CH1_4_INPUT_TAP_DELAY_REG, 0x400 ); // Clear Link Error Latch bits
		gl_sis3316_adc1->register_write( SIS3316_ADC_CH5_8_INPUT_TAP_DELAY_REG, 0x400 ); // Clear Link Error Latch bits
		gl_sis3316_adc1->register_write( SIS3316_ADC_CH9_12_INPUT_TAP_DELAY_REG, 0x400 ); // Clear Link Error Latch bits
		gl_sis3316_adc1->register_write( SIS3316_ADC_CH13_16_INPUT_TAP_DELAY_REG, 0x400 ); // Clear Link Error Latch bits

		gl_sis3316_adc1->register_read( SIS3316_ADC_CH1_4_FIRMWARE_REG, &data);
		printf("SIS3316_ADC_CH1_4_FIRMWARE_REG   = 0x%08x \n", data);
		this->adc_fpga_firmware_version = data & 0xffff;

		gl_sis3316_adc1->register_read( SIS3316_ADC_CH5_8_FIRMWARE_REG, &data);
		printf("SIS3316_ADC_CH5_8_FIRMWARE_REG   = 0x%08x \n", data);
		gl_sis3316_adc1->register_read( SIS3316_ADC_CH9_12_FIRMWARE_REG, &data);
		printf("SIS3316_ADC_CH9_12_FIRMWARE_REG  = 0x%08x \n", data);
		gl_sis3316_adc1->register_read( SIS3316_ADC_CH13_16_FIRMWARE_REG, &data);
		printf("SIS3316_ADC_CH13_16_FIRMWARE_REG = 0x%08x \n\n", data);

		gl_sis3316_adc1->register_read( SIS3316_ADC_CH1_4_STATUS_REG, &data);
		printf("SIS3316_ADC_CH1_4_STATUS_REG     = 0x%08x \n", data);
		gl_sis3316_adc1->register_read( SIS3316_ADC_CH5_8_STATUS_REG, &data);
		printf("SIS3316_ADC_CH5_8_STATUS_REG     = 0x%08x \n", data);
		gl_sis3316_adc1->register_read( SIS3316_ADC_CH9_12_STATUS_REG, &data);
		printf("SIS3316_ADC_CH9_12_STATUS_REG    = 0x%08x \n", data);
		gl_sis3316_adc1->register_read( SIS3316_ADC_CH13_16_STATUS_REG, &data);
		printf("SIS3316_ADC_CH13_16_STATUS_REG   = 0x%08x \n\n", data);



	}
	else {
		printf("SIS3316_MODID                  = 0x%08x     return_code = 0x%08x\n", data, return_code);
	}
	this->double_clock_configure_fft_frequency = 125000000.0 ;
#endif
/************************************************************************************************************************************************************/



   fMenuBarLayout = new TGLayoutHints(kLHintsTop | kLHintsExpandX);
   fMenuBarItemLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft, 0, 4, 0, 0);
   fMenuBarHelpLayout = new TGLayoutHints(kLHintsTop | kLHintsRight);

   fMenuFile = new TGPopupMenu(fClient->GetRoot());
   fMenuFile->AddEntry("Exit/Close/Quit", TEST1_FILE_EXIT);
   fMenuFile->AddSeparator(); 

   
// add Configuration
   fMenuConfiguration = new TGPopupMenu(gClient->GetDefaultRoot());
   fMenuConfiguration->AddEntry("Load Configuration file", M_LOAD_CONFIGURATION_DLG);
   fMenuConfiguration->AddSeparator();
   fMenuConfiguration->AddEntry("Save Configuration file", M_SAVE_CONFIGURATION_DLG);
   fMenuConfiguration->AddSeparator();

  // fMenuHelp = new TGPopupMenu(fClient->GetRoot());
   fMenuHelp = new TGPopupMenu(gClient->GetDefaultRoot());
   //fMenuHelp->AddSeparator();
   fMenuHelp->AddEntry("About", TEST1_HELP_ABOUT);

   // Menu button messages are handled by the main frame (i.e. "this")
   // ProcessMessage() method.
   fMenuFile->Associate(this);
   fMenuConfiguration->Associate(this);
 
   fMenuBar = new TGMenuBar(this);
   //fMenuBar = new TGMenuBar(fMenuDock, 1, 1, kHorizontalFrame);
   fMenuBar->AddPopup("File", fMenuFile, fMenuBarItemLayout);
   fMenuBar->AddPopup("Configuration", fMenuConfiguration, fMenuBarItemLayout);
   fMenuBar->AddPopup("Help", fMenuHelp, fMenuBarHelpLayout);

   //fMenuDock->AddFrame(fMenuBar, fMenuBarLayout);
	this->AddFrame(fMenuBar, new TGLayoutHints(kLHintsTop | kLHintsExpandX));

/************************************************************************************************************************************************************/

	Pixel_t yellow;
	fClient->GetColorByName("yellow", yellow);
	Pixel_t green;
	fClient->GetColorByName("green", green);
	Pixel_t red;
	fClient->GetColorByName("red", red);
/************************************************************************************************************************************************************/

	// use hierarchical cleaning
	TGGC myGC = *fClient->GetResourcePool()->GetFrameGC();
	TGFont *myfont = fClient->GetFont("-adobe-helvetica-bold-r-*-*-12-*-*-*-*-*-iso8859-1");
	if (myfont) myGC.SetFont(myfont->GetFontHandle());

	fFrame1_main = new TGHorizontalFrame(this, 1600, 1000, kFixedWidth);
	this->AddFrame(fFrame1_main, new TGLayoutHints(kLHintsTop | kLHintsLeft, 2, 2, 2, 2));

	//fFrame1 = new TGVerticalFrame(this, 600, 20, kFixedWidth);
	fFrame1 = new TGVerticalFrame(fFrame1_main, 800, 980, kFixedWidth);
	fFrame1_main->AddFrame(fFrame1, new TGLayoutHints(kLHintsTop | kLHintsLeft, 2, 2, 2, 2));

	fFrame2 = new TGVerticalFrame(fFrame1_main, 800, 980, kFixedWidth);
	fFrame1_main->AddFrame(fFrame2, new TGLayoutHints(kLHintsTop | kLHintsLeft, 2, 2, 2, 2));

	/*******************************************************************/

	fGrp1 = new TGGroupFrame(fFrame1, "Control");
	fFrame1->AddFrame(fGrp1, new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5, 5, 0));


	fTGVer_frame1 = new TGVerticalFrame(fGrp1, 260, 220, kVerticalFrame);
	fGrp1->AddFrame(fTGVer_frame1, new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5, 5, 5));
	
	// Control buttons Frame
	fTGHor_frame1 = new TGHorizontalFrame(fTGVer_frame1, 260, 220, kHorizontalFrame);
	fTGVer_frame1->AddFrame(fTGHor_frame1, new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5, 5, 5));

	// Control Information Frame
	fTGHor_frame1a = new TGHorizontalFrame(fTGVer_frame1, 260, 220, kHorizontalFrame);
	fTGVer_frame1->AddFrame(fTGHor_frame1a, new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5, 5, 5));


	// Add control buttons
	fStartB = new TGTextButton(fTGHor_frame1, "\n      Start Sampling      \n", SIS3316TestDialog_kCM_BUTTON_IRQ_NO_4);
	fStopB  = new TGTextButton(fTGHor_frame1, "\n      Stop Sampling       \n", SIS3316TestDialog_kCM_BUTTON_IRQ_NO_5);
	fClockConfiguration  = new TGTextButton(fTGHor_frame1, "\n      Sample Clock Configuration       \n", SIS3316TestDialog_kCM_BUTTON_IRQ_NO_6);

	this->fClockConfiguration_background_color = fClockConfiguration->GetBackground();

	fStartB->ChangeBackground(red);
	fStopB->ChangeBackground(red);
	fClockConfiguration->ChangeBackground(green);

	fStartB->SetEnabled(kFALSE); // dim
	fStopB->SetEnabled(kFALSE); // dim
	fClockConfiguration->SetEnabled(kTRUE); // not dim

	fStartB->Associate(this);
	fStopB->Associate(this);
	fClockConfiguration->Associate(this);

	fTGHor_frame1->AddFrame(fStartB, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandY,  2, 5, 5, 5));// hints, left, right, top, bottom
	fTGHor_frame1->AddFrame(fStopB, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandY,  5, 5, 5, 5));
	fTGHor_frame1->AddFrame(fClockConfiguration, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandY,  5, 2, 5, 5));


	fSIS3316_Test1_Run_Cmd = kFALSE;

	//////////////////////////


	// 	// Add control Information views
	fNumericEntriesTimeSecCounterView = new TGNumberEntry(fTGHor_frame1a, 0 /* value */, 8 /* width */, 0 /* irq */, (TGNumberFormat::kNESInteger)); //kNESHex
	fTGHor_frame1a->AddFrame(fNumericEntriesTimeSecCounterView, new TGLayoutHints(kLHintsTop | kLHintsLeft, 2, 2, 5, 5));

	fLabel_TimeSecCounterView = new TGLabel(fTGHor_frame1a, "seconds ", myGC(), myfont->GetFontStruct());
	fTGHor_frame1a->AddFrame(fLabel_TimeSecCounterView, new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 5, 2, 1, 1));


	fNumericEntriesBankLoopCounterView = new TGNumberEntry(fTGHor_frame1a, 0 /* value */, 8 /* width */, 0 /* irq */, (TGNumberFormat::kNESInteger)); //kNESHex
	fTGHor_frame1a->AddFrame(fNumericEntriesBankLoopCounterView, new TGLayoutHints(kLHintsTop | kLHintsLeft, 20, 2, 5, 5));

	fLabel_BankLoopCounterView = new TGLabel(fTGHor_frame1a, "Bank loops ", myGC(), myfont->GetFontStruct());
	fTGHor_frame1a->AddFrame(fLabel_BankLoopCounterView, new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 5, 2, 1, 1));

	fNumericEntriesTimeSecCounterView->SetIntNumber(0); //  
	fNumericEntriesBankLoopCounterView->SetIntNumber(0); //  

	

	// Temperature
	fNumericEntriesTemperatureView = new TGNumberEntry(fTGHor_frame1a, 0 /* value */, 4 /* width */, 0 /* irq */, (TGNumberFormat::kNESInteger)); //kNESHex
	fTGHor_frame1a->AddFrame(fNumericEntriesTemperatureView, new TGLayoutHints(kLHintsTop | kLHintsLeft, 25, 2, 5, 5));

	fLabel_TemperatureView = new TGLabel(fTGHor_frame1a, " C", myGC(), myfont->GetFontStruct());
	fTGHor_frame1a->AddFrame(fLabel_TemperatureView, new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 5, 2, 1, 1));

	gl_sis3316_adc1->register_read(SIS3316_INTERNAL_TEMPERATURE_REG, &data);
	fNumericEntriesTemperatureView->SetIntNumber((data & 0x3FF) / 4); //  


	fNumericEntriesTimeSecCounterView->SetState(kFALSE); // dim
	fNumericEntriesBankLoopCounterView->SetState(kFALSE); // dim
	fNumericEntriesTemperatureView->SetState(kFALSE); // dim

/**********************************************************************************************************/
	 
	TGCanvas* fCanvasScroll = new TGCanvas(this);

	TGViewPort* fViewPortScroll = fCanvasScroll->GetViewPort();
	fTab = new TGTab(fViewPortScroll, 700, 800);

	tab_color_not_active = 0xFFFFFF ;
    tab_color_active     = 0xE0E0E0 ;

	// Overview
	// Tab 1:	tabel_tab[0] = fTab->GetTabTab("Sampling Control");
	// Tab 1a:	tabel_tab[1] = fTab->GetTabTab("Sampling Trigger");
	// Tab 2:	tabel_tab[2] = fTab->GetTabTab("Display Control");
	// Tab 2b:	tabel_tab[3] = fTab->GetTabTab("Polarity");
	// Tab 4:	tabel_tab[4] = fTab->GetTabTab("Gain/Offset");
	// Tab 3:	tabel_tab[5] = fTab->GetTabTab("Trigger");
	// Tab 3a:	tabel_tab[6] = fTab->GetTabTab("Energy");
	// Tab 5:	tabel_tab[7] = fTab->GetTabTab("Sample Clock");
	// Tab 6:	tabel_tab[8] = fTab->GetTabTab("NIM Outputs");

 
/**********************************************************************************************************/
/**********************************************************************************************************/

// Tab1

	fTab->SetEnabled(0, kTRUE);
	fTab->Associate(this);

	TGCompositeFrame *tf = fTab->AddTab("Sampling Control");
	tabel_tab[0] = fTab->GetTabTab("Sampling Control");
	tabel_tab[0]->ChangeBackground(tab_color_not_active);
	fTab->SetText("Sampling Control");
	this->sis3316Test1_nof_valid_tabel_tabs = 1 ;

	/*************/

	fF_tab1 = new TGCompositeFrame(tf, 60, 20, kVerticalFrame);

	/*********************************************************/

	fF_tab1_fGrp1 = new TGGroupFrame(fF_tab1, "Run Control");
	fF_tab1->AddFrame(fF_tab1_fGrp1, new TGLayoutHints(kLHintsExpandX, 5, 5, 3, 2));

	fF1 = new TGCompositeFrame(fF_tab1_fGrp1, 260, 220, kHorizontalFrame);
	fF_tab1_fGrp1->AddFrame(fF1, new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5, 5, 5));
	fF1->SetLayoutBroken(kTRUE);


	fChkStopAfterTime = new TGCheckButton(fF1, "Stop after Time (seconds, min=10, max=604800)", SIS3316TestDialog_kCM_CHECKBUTTON_IRQ_NO_9);
	fChkStopAfterTime->Associate(this);
	fF1->AddFrame(fChkStopAfterTime, new TGLayoutHints(kLHintsTop | kLHintsLeft, 2, 15, 5, 5));// hints, left, right, top, bottom

	fNumericEntriesStopAfterTime = new TGNumberEntry(fF1, 20 /* value */, 8 /* width */, 0 /* irq */, (TGNumberFormat::kNESInteger)); //kNESHex
	fF1->AddFrame(fNumericEntriesStopAfterTime, new TGLayoutHints(kLHintsTop | kLHintsRight, 21, 2, 5, 5));

	fChkStopAfterTime->SetState(kButtonUp); // is OFF !
	fNumericEntriesStopAfterTime->SetState(kFALSE); //

	fNumericEntriesStopAfterTime->SetLimits((TGNumberFormat::kNELLimitMinMax), 10, 604800); // min. 10 sec ; max. 1 week 7 x 24 * 60 * 60
	fNumericEntriesStopAfterTime->SetIntNumber(60); // 

	fChkStopAfterTime->MoveResize(2, 5, 320, 20);
	fNumericEntriesStopAfterTime->MoveResize(325, 5, 80, 20);

	fF1->MoveResize(2, 2, 500, 22);
	fF1->MapSubwindows();
	fF1->MapWindow();

/*************/

	fF1A = new TGCompositeFrame(fF_tab1_fGrp1, 260, 220, kHorizontalFrame);
	fF_tab1_fGrp1->AddFrame(fF1A, new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5, 1, 1));
	fF1A->SetLayoutBroken(kTRUE);

	fChkStopAfterBanks = new TGCheckButton(fF1A, "Stop after Bank Loops                   ", SIS3316TestDialog_kCM_CHECKBUTTON_IRQ_NO_10);
	fChkStopAfterBanks->Associate(this);
	fF1A->AddFrame(fChkStopAfterBanks, new TGLayoutHints(kLHintsTop | kLHintsLeft ,  2, 15, 3, 1));// hints, left, right, top, bottom

	fNumericEntriesStopAfterBanks = new TGNumberEntry(fF1A, 20 /* value */, 8 /* width */, 0 /* irq */ , (TGNumberFormat::kNESInteger) ) ; //kNESHex
	fF1A->AddFrame(fNumericEntriesStopAfterBanks, new TGLayoutHints(kLHintsTop | kLHintsRight, 21, 2, 2, 2));

	fChkStopAfterBanks->SetState(kButtonUp)   ; // is OFF !
	fNumericEntriesStopAfterBanks->SetState(kFALSE); //

	fNumericEntriesStopAfterBanks->SetLimits((TGNumberFormat::kNELLimitMinMax), 1, 1000000);
	fNumericEntriesStopAfterBanks->SetIntNumber(1); // 

	fChkStopAfterBanks->MoveResize(2, 5, 320, 20);
	fNumericEntriesStopAfterBanks->MoveResize(325, 5, 80, 20);

	fF1A->MoveResize(2, 2, 500, 22);
	fF1A->MapSubwindows();
	fF1A->MapWindow();




/**********************/
	fF_tab1_fGrp1B = new TGGroupFrame(fF_tab1, "Write Data to File");
	fF_tab1->AddFrame(fF_tab1_fGrp1B, new TGLayoutHints(kLHintsExpandX, 5, 5, 3, 2));

	fF1B = new TGCompositeFrame(fF_tab1_fGrp1B, 260, 220, kHorizontalFrame);
	fF_tab1_fGrp1B->AddFrame(fF1B, new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5, 1, 1));

	fChkWriteDataToFile = new TGCheckButton(fF1B, "Write Data to File enable", SIS3316TestDialog_kCM_CHECKBUTTON_IRQ_NO_12);
	fChkWriteDataToFile->Associate(this);
	fF1B->AddFrame(fChkWriteDataToFile, new TGLayoutHints(kLHintsTop | kLHintsLeft ,  2, 2, 2, 2));// hints, left, right, top, bottom

	fChkWriteMultipleFiles = new TGCheckButton(fF1B, "Write multiple Files", 0);
	//fChkWriteMultipleFiles->Associate(this);
	fF1B->AddFrame(fChkWriteMultipleFiles, new TGLayoutHints(kLHintsTop | kLHintsLeft ,  5, 2, 2, 2));// hints, left, right, top, bottom



	fF1C = new TGCompositeFrame(fF_tab1_fGrp1B, 260, 220, kHorizontalFrame);
	fF_tab1_fGrp1B->AddFrame(fF1C, new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5, 3, 2));

	fTextEntryDataFilePath = new TGTextEntry(fF1C, "sis3316_data_file.dat");
	fF1C->AddFrame(fTextEntryDataFilePath, new TGLayoutHints(kLHintsNormal, 5, 5, 5, 5));
	fTextEntryDataFilePath->SetWidth(380);
	
	fTextButtonDataFilePath = new TGTextButton(fF1C, "...", SIS3316TestDialog_kCM_BUTTON_IRQ_NO_42);
	fTextButtonDataFilePath->ChangeOptions(fTextButtonDataFilePath->GetOptions() | kFixedWidth);
	fTextButtonDataFilePath->SetWidth(50);
	fTextButtonDataFilePath->Associate(this);
	fF1C->AddFrame(fTextButtonDataFilePath, new TGLayoutHints(kLHintsNormal, 30, 5, 5, 5));

	fTextEntryDataFilePath->SetText("sis3316_test_data.dat");


// -----

	fF_tab1_fGrp1A = new TGGroupFrame(fF_tab1, "Sample Parameter");
	fF_tab1->AddFrame(fF_tab1_fGrp1A, new TGLayoutHints(kLHintsExpandX, 5, 5, 3, 2));

	fF4A = new TGCompositeFrame(fF_tab1_fGrp1A, 60, 20, kHorizontalFrame);
	//fF4A = new TGCompositeFrame(fF_tab1_fGrp1A, 60, 20, kVerticalFrame);
	fF_tab1_fGrp1A->AddFrame(fF4A, new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5, 2, 2));

	fCombo_SampleControl_BankModus = new TGComboBox(fF4A, SIS3316TestDialog_kCM_COMBOBOX_IRQ_NO_30);
	fCombo_SampleControl_BankModus->Associate(this); // Event (IRQ) anmelden
	fF4A->AddFrame(fCombo_SampleControl_BankModus, new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 3, 2));

	for (i = 0; i < 2; i++) {
		fCombo_SampleControl_BankModus->AddEntry(entrySampleControl_BankModus[i], i);
	}
	fCombo_SampleControl_BankModus->Resize(270, 20);

 

	fChkNofEvents_ProBank = new TGCheckButton(fF_tab1_fGrp1A, "limit the maximum number of Events for each Bank", SIS3316TestDialog_kCM_CHECKBUTTON_IRQ_NO_11);
	fChkNofEvents_ProBank->Associate(this);
	fF_tab1_fGrp1A->AddFrame(fChkNofEvents_ProBank, new TGLayoutHints(kLHintsTop | kLHintsLeft ,  5, 15, 2, 2));// hints, left, right, top, bottom
	
    fTGHorizontalFrame = new TGHorizontalFrame(fF_tab1_fGrp1A, 200, 30);
	fF_tab1_fGrp1A->AddFrame(fTGHorizontalFrame, new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5, 2, 2));

	fNumericEntries_SampleControl_MaxNofEvents_ProBank = new TGNumberEntry(fTGHorizontalFrame, 100 /* value */, 12 /* width */,0, (TGNumberFormat::kNESInteger) ) ; //kNESHex
    //fNumericEntries_SampleControl_NofEvents_ProBank[i]->Associate(this);
    fTGHorizontalFrame->AddFrame(fNumericEntries_SampleControl_MaxNofEvents_ProBank, new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2));
    fLabel[0] = new TGLabel(fTGHorizontalFrame, "limited number of Events for each Bank", myGC(), myfont->GetFontStruct());
    fTGHorizontalFrame->AddFrame(fLabel[0], new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 5, 2));
	fNumericEntries_SampleControl_MaxNofEvents_ProBank->SetLimits((TGNumberFormat::kNELLimitMinMax), 1, 1000000);
 
    fTGHorizontalFrame = new TGHorizontalFrame(fF_tab1_fGrp1A, 200, 30);
	fF_tab1_fGrp1A->AddFrame(fTGHorizontalFrame, new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5, 2, 2));

	fNumericEntries_SampleControl_NofEvents_ProBank = new TGNumberEntry(fTGHorizontalFrame, 1 /* value */, 12 /* width */,0, (TGNumberFormat::kNESInteger) ) ; //kNESHex
    fTGHorizontalFrame->AddFrame(fNumericEntries_SampleControl_NofEvents_ProBank, new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2));
    fLabel[0] = new TGLabel(fTGHorizontalFrame, "Info: programmed maximun number of Events for each Bank", myGC(), myfont->GetFontStruct());
    fTGHorizontalFrame->AddFrame(fLabel[0], new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2));



	fTGHorizontalFrame = new TGHorizontalFrame(fF_tab1_fGrp1A, 200, 30);
	fF_tab1_fGrp1A->AddFrame(fTGHorizontalFrame, new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5, 2, 2));

	fNumericEntries_SampleControl_PossibleMaxNofEvents_ProBank = new TGNumberEntry(fTGHorizontalFrame, 1 /* value */, 12 /* width */, 0, (TGNumberFormat::kNESInteger)); //kNESHex
	fTGHorizontalFrame->AddFrame(fNumericEntries_SampleControl_PossibleMaxNofEvents_ProBank, new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2));
	fLabel[0] = new TGLabel(fTGHorizontalFrame, "Info: possible maximun number of Events for each Bank", myGC(), myfont->GetFontStruct());
	fTGHorizontalFrame->AddFrame(fLabel[0], new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2));



	fCombo_SampleControl_BankModus->Select(0, kTRUE); //  Single
	fChkNofEvents_ProBank->SetState(kButtonUp); // is Off!
	this->root_chk_bank_event_nof_limit_on_flag = 0;

	fNumericEntries_SampleControl_MaxNofEvents_ProBank->SetIntNumber(1); // 
	fNumericEntries_SampleControl_NofEvents_ProBank->SetIntNumber(1); // 
	fNumericEntries_SampleControl_NofEvents_ProBank->SetState(kFALSE); //

	fNumericEntries_SampleControl_PossibleMaxNofEvents_ProBank->SetIntNumber(1); // 
	fNumericEntries_SampleControl_PossibleMaxNofEvents_ProBank->SetState(kFALSE); //
	

	fChkWriteDataToFile->SetState(kButtonUp)   ; // is OFF !
	fChkWriteMultipleFiles->SetState(kButtonDown)   ; // is On !

	fChkWriteMultipleFiles->SetEnabled(kFALSE);
	fTextEntryDataFilePath->SetEnabled(kFALSE);
	fTextButtonDataFilePath->SetEnabled(kFALSE);

	this->root_chk_bank_event_nof_limit_on_flag = 1;


	if (fCombo_SampleControl_BankModus->GetSelected() == 0) {
		fChkWriteDataToFile->SetEnabled(kFALSE);
		fChkNofEvents_ProBank->SetEnabled(kFALSE)   ; // dim
		fNumericEntries_SampleControl_MaxNofEvents_ProBank->SetState(kFALSE); //
		fNumericEntries_SampleControl_NofEvents_ProBank->SetIntNumber(100); // 
		fNumericEntries_SampleControl_PossibleMaxNofEvents_ProBank->SetIntNumber(1); // 
	}
	else {
		fChkWriteDataToFile->SetEnabled(kTRUE);
		fChkNofEvents_ProBank->SetEnabled(kTRUE)   ; // not dim
		fChkNofEvents_ProBank->SetState(this->root_chk_bank_event_nof_limit_on_flag ? kButtonDown : kButtonUp);
		if (fChkNofEvents_ProBank->IsOn() == kTRUE)  {
			fNumericEntries_SampleControl_MaxNofEvents_ProBank->SetState(kTRUE); //
		}
		else {
			fNumericEntries_SampleControl_MaxNofEvents_ProBank->SetState(kFALSE); //
		}
	}




/************************************************************************************************************************************/
	fF_tab1_fGrp3 = new TGGroupFrame(fF_tab1, "Event/Hit Parameter");
	fF_tab1->AddFrame(fF_tab1_fGrp3, new TGLayoutHints(kLHintsExpandX, 5, 5, 3, 2));
	fF5A = new TGCompositeFrame(fF_tab1_fGrp3, 60, 20, kVerticalFrame);
	fF_tab1_fGrp3->AddFrame(fF5A, new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5, 5, 5));


	i = 0 ; // Pretrigger Delay
    fF[i] = new TGHorizontalFrame(fF5A, 200, 30);
    fF5A->AddFrame(fF[i], new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 1, 1));
    fNumericEntries_EventHitParameter[i] = new TGNumberEntry(fF[i], numinit[i], 12, i + 20, (TGNumberFormat::kNESInteger) ) ; //kNESHex
    //fNumericEntries_EventHitParameter[i]->Associate(this);
    fF[i]->AddFrame(fNumericEntries_EventHitParameter[i], new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 1));
    fLabel[i] = new TGLabel(fF[i], numlabel[i], myGC(), myfont->GetFontStruct());
    fF[i]->AddFrame(fLabel[i], new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 1));

	i = 1 ; // Sample Start Index
    fF[i] = new TGHorizontalFrame(fF5A, 200, 30);
    fF5A->AddFrame(fF[i], new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 1, 1));// hints, left, right, top, bottom
    fNumericEntries_EventHitParameter[i] = new TGNumberEntry(fF[i], numinit[i], 12, i + 20, (TGNumberFormat::kNESInteger) ) ; //kNESHex
    //fNumericEntries_EventHitParameter[i]->Associate(this);
    fF[i]->AddFrame(fNumericEntries_EventHitParameter[i], new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 1, 1));
    fLabel[i] = new TGLabel(fF[i], numlabel[i], myGC(), myfont->GetFontStruct());
    fF[i]->AddFrame(fLabel[i],  new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 1, 1));

	i = 2 ; //  Sample Length
    fF[i] = new TGHorizontalFrame(fF5A, 200, 30);
    fF5A->AddFrame(fF[i], new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 1, 2));// hints, left, right, top, bottom
    fNumericEntries_EventHitParameter[i] = new TGNumberEntry(fF[i], numinit[i], 12, SIS3316TestDialog_kCM_ENTRY_IRQ_NO_11, (TGNumberFormat::kNESInteger) ) ; //kNESHex
    fNumericEntries_EventHitParameter[i]->Associate(this);
    fF[i]->AddFrame(fNumericEntries_EventHitParameter[i], new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2));
    fLabel[i] = new TGLabel(fF[i], numlabel[i], myGC(), myfont->GetFontStruct());
    fF[i]->AddFrame(fLabel[i],  new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 1, 1));



// MAW Test Buffer
	fCombo_SetSelectMAW_TestBuffer = new TGComboBox(fF5A, 28);
	fF5A->AddFrame(fCombo_SetSelectMAW_TestBuffer, new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 3, 2, 10, 2));// hints, left, right, top, bottom
	for (i = 0; i < 2; i++) {
		fCombo_SetSelectMAW_TestBuffer->AddEntry(entryClock_SelectMAW_TestBuffer[i], i);
	}
	fCombo_SetSelectMAW_TestBuffer->Select(0, kTRUE); //  Trigger
	fCombo_SetSelectMAW_TestBuffer->Resize(270, 22);


	i = 3 ; //  MAW TestBuffer Pretrigger Delay
    fF[i] = new TGHorizontalFrame(fF5A, 200, 30);
    fF5A->AddFrame(fF[i], new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 5, 1));// hints, left, right, top, bottom
    fNumericEntries_EventHitParameter[i] = new TGNumberEntry(fF[i], numinit[i], 12, i + 20, (TGNumberFormat::kNESInteger) ) ; //kNESHex
    //fNumericEntries_EventHitParameter[i]->Associate(this);
    fF[i]->AddFrame(fNumericEntries_EventHitParameter[i], new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2));
    fLabel[i] = new TGLabel(fF[i], numlabel[i], myGC(), myfont->GetFontStruct());
    fF[i]->AddFrame(fLabel[i],  new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2));


	i = 4 ; //  MAW TestBuffer Start Index
    fF[i] = new TGHorizontalFrame(fF5A, 200, 30);
    fF5A->AddFrame(fF[i], new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 1, 1));// hints, left, right, top, bottom
    fNumericEntries_EventHitParameter[i] = new TGNumberEntry(fF[i], numinit[i], 12, i + 20, (TGNumberFormat::kNESInteger) ) ; //kNESHex
    //fNumericEntries_EventHitParameter[i]->Associate(this);
    fF[i]->AddFrame(fNumericEntries_EventHitParameter[i], new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2));
    fLabel[i] = new TGLabel(fF[i], numlabel[i], myGC(), myfont->GetFontStruct());
    fF[i]->AddFrame(fLabel[i],  new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2));



	i = 5 ; // MAW TestBuffer Length
    fF[i] = new TGHorizontalFrame(fF5A, 200, 30);
    fF5A->AddFrame(fF[i], new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 1, 1));
    fNumericEntries_EventHitParameter[i] = new TGNumberEntry(fF[i], numinit[i], 12, SIS3316TestDialog_kCM_ENTRY_IRQ_NO_61, (TGNumberFormat::kNESInteger) ) ; //kNESHex
    fNumericEntries_EventHitParameter[i]->Associate(this);
    fF[i]->AddFrame(fNumericEntries_EventHitParameter[i], new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2));
    fLabel[i] = new TGLabel(fF[i], numlabel[i], myGC(), myfont->GetFontStruct());
    fF[i]->AddFrame(fLabel[i], new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2));

// *******
	i = 6 ; //  //Info: Event Length (32-bit words)
    fF[i] = new TGHorizontalFrame(fF5A, 200, 30);
    fF5A->AddFrame(fF[i], new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 1, 1));
    fNumericEntries_EventHitParameter[i] = new TGNumberEntry(fF[i], numinit[i], 12, i + 20, (TGNumberFormat::kNESInteger) ) ; //kNESHex
    //fNumericEntries_EventHitParameter[i]->Associate(this);
    fF[i]->AddFrame(fNumericEntries_EventHitParameter[i], new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2));
    fLabel[i] = new TGLabel(fF[i], numlabel[i], myGC(), myfont->GetFontStruct());
    fF[i]->AddFrame(fLabel[i], new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2));

// *******
	i = 7 ; //  //Info: Active Trigger Gate Length
    fF[i] = new TGHorizontalFrame(fF5A, 200, 30);
    fF5A->AddFrame(fF[i], new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 1, 1));
    fNumericEntries_EventHitParameter[i] = new TGNumberEntry(fF[i], numinit[i], 12, i + 20, (TGNumberFormat::kNESInteger) ) ; //kNESHex
    //fNumericEntries_EventHitParameter[i]->Associate(this);
    fF[i]->AddFrame(fNumericEntries_EventHitParameter[i], new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2));
    fLabel[i] = new TGLabel(fF[i], numlabel[i], myGC(), myfont->GetFontStruct());
    fF[i]->AddFrame(fLabel[i], new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2));

	// *******
	i = 8; //  //Info: Address Threshold
	fF[i] = new TGHorizontalFrame(fF5A, 200, 30);
	fF5A->AddFrame(fF[i], new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 1, 1));
	fNumericEntries_EventHitParameter[i] = new TGNumberEntry(fF[i], numinit[i], 12, i + 20, (TGNumberFormat::kNESInteger)); //kNESHex
	fF[i]->AddFrame(fNumericEntries_EventHitParameter[i], new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2));
	fLabel[i] = new TGLabel(fF[i], numlabel[i], myGC(), myfont->GetFontStruct());
	fF[i]->AddFrame(fLabel[i], new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2));

	// *******

	fChk_SuppressEventsIfAddrThresFlag = new TGCheckButton(fF5A, "Suppress saving of more Hits/Events if Memory Address Threshold Flag is valid", SIS3316TestDialog_kCM_CHECKBUTTON_IRQ_NO_61);
	fChk_SuppressEventsIfAddrThresFlag->SetState(kButtonUp); // is Off!
	fF5A->AddFrame(fChk_SuppressEventsIfAddrThresFlag, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 2, 2, 10, 10));// hints, left, right, top, bottom


	// *******

	fChk_EventHitParameter_DataFormatBit0 = new TGCheckButton(fF5A, "Data Format: Peak-Height / 6 x Accumulators Enable", SIS3316TestDialog_kCM_CHECKBUTTON_IRQ_NO_61);
	fChk_EventHitParameter_DataFormatBit0->SetState(kButtonDown); // is ON !
	fF5A->AddFrame(fChk_EventHitParameter_DataFormatBit0, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 2, 2, 10, 2));// hints, left, right, top, bottom

	fChk_EventHitParameter_DataFormatBit1 = new TGCheckButton(fF5A, "Data Format: 2 x Accumulators (Gate 7/8) Enable", SIS3316TestDialog_kCM_CHECKBUTTON_IRQ_NO_61);
	fChk_EventHitParameter_DataFormatBit1->SetState(kButtonUp); // is Off !
	fF5A->AddFrame(fChk_EventHitParameter_DataFormatBit1, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 2, 2, 5, 2));// hints, left, right, top, bottom

	fChk_EventHitParameter_DataFormatBit2 = new TGCheckButton(fF5A, "Data Format: 3xTrigger MAW values Enable", SIS3316TestDialog_kCM_CHECKBUTTON_IRQ_NO_61);
	fChk_EventHitParameter_DataFormatBit2->SetState(kButtonUp)   ; // is Off!
	fF5A->AddFrame(fChk_EventHitParameter_DataFormatBit2, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,  2, 2, 5, 2));// hints, left, right, top, bottom

	fChk_EventHitParameter_DataFormatBit3 = new TGCheckButton(fF5A, "Data Format: Energy FIR Filter values Enable", SIS3316TestDialog_kCM_CHECKBUTTON_IRQ_NO_61);
	fChk_EventHitParameter_DataFormatBit3->SetState(kButtonUp)   ; // is Off !
	fF5A->AddFrame(fChk_EventHitParameter_DataFormatBit3, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,  2, 2, 5, 10));// hints, left, right, top, bottom


	fChk_SaveRawDataFirstEventOnly = new TGCheckButton(fF5A, "Save Raw data of first Event of Bankbuffer only Enable", SIS3316TestDialog_kCM_CHECKBUTTON_IRQ_NO_61);
	fChk_SaveRawDataFirstEventOnly->SetState(kButtonUp); // is Off!
	fF5A->AddFrame(fChk_SaveRawDataFirstEventOnly, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 2, 2, 5, 10));// hints, left, right, top, bottom

	fChk_EventHitParameter_DataFormatBit0->Associate(this);
	fChk_EventHitParameter_DataFormatBit1->Associate(this);
	fChk_EventHitParameter_DataFormatBit2->Associate(this);
	fChk_EventHitParameter_DataFormatBit3->Associate(this);
	fChk_SaveRawDataFirstEventOnly->Associate(this);


   // start values

	i = 0 ; // Pretrigger Delay
	fNumericEntries_EventHitParameter[i]->SetLimits((TGNumberFormat::kNELLimitMinMax), 0, MAX_PRETRIGGER_DELAY);
	fNumericEntries_EventHitParameter[i]->SetIntNumber(64); // pretrigger delay

	i = 1 ; // Sample Start Index
	fNumericEntries_EventHitParameter[i]->SetLimits((TGNumberFormat::kNELLimitMinMax), 0, 65534);
	fNumericEntries_EventHitParameter[i]->SetIntNumber(0); // Sample Start Index


 	i = 2 ; //  Sample Length
	fNumericEntries_EventHitParameter[i]->SetLimits((TGNumberFormat::kNELLimitMinMax), 0, MAX_SAMPLE_LENGTH);
	fNumericEntries_EventHitParameter[i]->SetIntNumber(1024); // sample_length
	this->raw_sample_length = fNumericEntries_EventHitParameter[i]->GetIntNumber();



	i = 3 ; //  MAW TestBuffer Pretrigger Delay
	fNumericEntries_EventHitParameter[i]->SetLimits((TGNumberFormat::kNELLimitMinMax), 0, 1022);
	fNumericEntries_EventHitParameter[i]->SetIntNumber(100); // Test MAW Pretrigger Delay

	i = 4 ; // MAW TestBuffer Start Index
	fNumericEntries_EventHitParameter[i]->SetLimits((TGNumberFormat::kNELLimitMinMax), 0, 32767);
	fNumericEntries_EventHitParameter[i]->SetIntNumber(0); // Test MAW Length


	i = 5 ; // MAW TestBuffer Length
	fNumericEntries_EventHitParameter[i]->SetLimits((TGNumberFormat::kNELLimitMinMax), 0, 2048);
	fNumericEntries_EventHitParameter[i]->SetIntNumber(1000); // Test MAW Length

	i = 6 ; //  Info: Event Length (32-bit words)
	fNumericEntries_EventHitParameter[i]->SetIntNumber(0); //  
	fNumericEntries_EventHitParameter[i]->SetState(kFALSE); //  
 
 
	i = 7 ; //  Info: Active Trigger Gate Length
	fNumericEntries_EventHitParameter[i]->SetIntNumber(0); //  
	fNumericEntries_EventHitParameter[i]->SetState(kFALSE); //  

	i = 8; //  Info: Address Threshold
	fNumericEntries_EventHitParameter[i]->SetIntNumber(0); //  
	fNumericEntries_EventHitParameter[i]->SetState(kFALSE); //  


	tf->AddFrame(fF_tab1, new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5, 5, 5));

	

 /**********************************************************************************************************/
 /**********************************************************************************************************/
 /**********************************************************************************************************/

 // Tab 1A  : Sampling Trigger 
	tf = fTab->AddTab("Sampling Trigger");
	tabel_tab[1] = fTab->GetTabTab("Sampling Trigger");
	tabel_tab[1]->ChangeBackground(tab_color_not_active);
	this->sis3316Test1_nof_valid_tabel_tabs++;

	fF_tab1a = new TGCompositeFrame(tf, 600, 500, kVerticalFrame);
	fF_tab1a->SetLayoutBroken(kTRUE);



	fTGHor_tab1a = new TGHorizontalFrame(fF_tab1a, 900, 900, kHorizontalFrame);
	fF_tab1a->AddFrame(fTGHor_tab1a, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 5, 5, 3, 5));
	//fTGHor_tab1a->SetLayoutBroken(kTRUE);
	fF_tab1a->Resize(900, 900);

	
	// ********

	// "VME FPGA: External Trigger Condition"
	fF_tab1a_fGrp1a = new TGGroupFrame(fTGHor_tab1a, "VME FPGA: External Trigger Condition");
	fTGHor_tab1a->AddFrame(fF_tab1a_fGrp1a, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 5, 5, 2, 3));
	//fF_tab1a_fGrp1a->SetLayoutBroken(kTRUE);



	fF5B = new TGCompositeFrame(fF_tab1a_fGrp1a, 60, 20, kVerticalFrame);
	fF_tab1a_fGrp1a->AddFrame(fF5B, new TGLayoutHints(kLHintsTop | kLHintsLeft, 0, 0, 5, 5));


	fChkExternalTriggerFunc = new TGCheckButton(fF5B, "VME FPGA: External Trigger function as External Trigger En", 0);
	fChkExternalTriggerFunc->SetState(kButtonDown); // is ON !
	fF5B->AddFrame(fChkExternalTriggerFunc, new TGLayoutHints(kLHintsTop | kLHintsLeft , 0, 0, 5, 2));
	//fChkExternalTriggerFunc->MoveResize(10, 60, 70, 15);

	fChkKeyTrigger = new TGCheckButton(fF5B, "Software: Key-Trigger Enable", 0);
	fChkKeyTrigger->SetState(kButtonDown); // is ON !
	fF5B->AddFrame(fChkKeyTrigger, new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 2, 2, 2));
	//fChkKeyTrigger->MoveResize(20, 80, 70, 15);

	fChkLemoInTiEnable = new TGCheckButton(fF5B, "VME FPGA: Lemo Input TI use as External Trigger Enable", 0);
	fChkLemoInTiEnable->SetState(kButtonUp); // is OFF !
	fF5B->AddFrame(fChkLemoInTiEnable, new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 2, 2, 2));
	//fChkLemoInTiEnable->MoveResize(20, 100, 70, 15);





	fChkExternalTriggerDisableWithBusyEnable = new TGCheckButton(fF5B, "VME FPGA: External Trigger Disable with Internal Busy", 0);
	fChkExternalTriggerDisableWithBusyEnable->SetState(kButtonUp); // is OFF !
	//fChkExternalTriggerDisableWithBusyEnable->MoveResize(20, 140, 70, 15);
	fF5B->AddFrame(fChkExternalTriggerDisableWithBusyEnable, new TGLayoutHints(kLHintsTop | kLHintsLeft, 0, 0, 5, 5));


	fChkFeedbackInternalTriggerEnable = new TGCheckButton(fF5B, "VME FPGA: Feedback Internal Trigger(s) as External Trigger", 0);
	fChkFeedbackInternalTriggerEnable->SetState(kButtonUp); // is OFF !
	//fChkFeedbackInternalTriggerEnable->MoveResize(20, 120, 70, 15);
	fF5B->AddFrame(fChkFeedbackInternalTriggerEnable, new TGLayoutHints(kLHintsTop | kLHintsLeft, 0, 0, 10, 2));


	fChkFeedbackCoincidence1TriggerEnable = new TGCheckButton(fF5B, "VME FPGA: Coincidence 1 Trigger Feedback Select", 0);
	fChkFeedbackCoincidence1TriggerEnable->SetState(kButtonUp); // is OFF !
	//fChkFeedbackCoincidence1TriggerEnable->MoveResize(20, 120, 70, 15);
	fF5B->AddFrame(fChkFeedbackCoincidence1TriggerEnable, new TGLayoutHints(kLHintsTop | kLHintsLeft, 15, 0, 2, 2));



	// "VME FPGA: VME FPGA: Feedback Internal Trigger Enable
	fF_tab1a_fGrp1f = new TGGroupFrame(fF_tab1a_fGrp1a, "VME FPGA: Internal Trigger Feedback Select");
	fF_tab1a_fGrp1a->AddFrame(fF_tab1a_fGrp1f, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 5, 5, 2, 3));
	fF_tab1a_fGrp1f->SetLayoutBroken(kTRUE);


	fIntFeedbackTriggerEnableCh_Set = new TGTextButton(fF_tab1a_fGrp1f, "Enable all Channels", SIS3316TestDialog_kCM_BUTTON_INTFEEDBACK_IRQ_NO_18);
	fIntFeedbackTriggerEnableCh_Clr = new TGTextButton(fF_tab1a_fGrp1f, "Disable all Channels", SIS3316TestDialog_kCM_BUTTON_INTFEEDBACK_IRQ_NO_19);
	fIntFeedbackTriggerEnableCh_Set->ChangeBackground(yellow);
	fIntFeedbackTriggerEnableCh_Clr->ChangeBackground(yellow);
	fIntFeedbackTriggerEnableCh_Set->Associate(this);
	fIntFeedbackTriggerEnableCh_Clr->Associate(this);
	fIntFeedbackTriggerEnableCh_Set->MoveResize(20, 25, 120, 25);
	fIntFeedbackTriggerEnableCh_Clr->MoveResize(180, 25, 120, 25);


	for (i = 0; i < 4; i++) {
		fTGHor_tab1a_grp1f_sub[i] = new TGHorizontalFrame(fF_tab1a_fGrp1f, 360, 220, kHorizontalFrame);
		fF_tab1a_fGrp1f->AddFrame(fTGHor_tab1a_grp1f_sub[i], new TGLayoutHints(kLHintsExpandX, 5, 5, 15, 5));
		fTGHor_tab1a_grp1f_sub[i]->MoveResize(20, 65 + ((i) * 20), 310, 17);

		fChkIntFeedbackTriggerEnableCh[15 - i] = new TGCheckButton(fTGHor_tab1a_grp1f_sub[i], chkChLabel[15 - i], 0);
		fChkIntFeedbackTriggerEnableCh[15 - i]->MoveResize(0, 0, 60, 15);
		fChkIntFeedbackTriggerEnableCh[15 - i]->SetState(kButtonUp); // is OFF !

		fChkIntFeedbackTriggerEnableCh[11 - i] = new TGCheckButton(fTGHor_tab1a_grp1f_sub[i], chkChLabel[11 - i], 0);
		fChkIntFeedbackTriggerEnableCh[11 - i]->MoveResize(80, 0, 60, 15);
		fChkIntFeedbackTriggerEnableCh[11 - i]->SetState(kButtonUp); // is OFF !

		fChkIntFeedbackTriggerEnableCh[7 - i] = new TGCheckButton(fTGHor_tab1a_grp1f_sub[i], chkChLabel[7 - i], 0);
		fChkIntFeedbackTriggerEnableCh[7 - i]->MoveResize(160, 0, 60, 15);
		fChkIntFeedbackTriggerEnableCh[7 - i]->SetState(kButtonUp); // is OFF !

		fChkIntFeedbackTriggerEnableCh[3 - i] = new TGCheckButton(fTGHor_tab1a_grp1f_sub[i], chkChLabel[3 - i], 0);
		fChkIntFeedbackTriggerEnableCh[3 - i]->MoveResize(240, 0, 60, 15);
		fChkIntFeedbackTriggerEnableCh[3 - i]->SetState(kButtonUp); // is OFF !
	}

	fF_tab1a_fGrp1f->MapSubwindows();
	fF_tab1a_fGrp1f->MapWindow();
	fF_tab1a_fGrp1f->MoveResize(2, 20, 310, 165);

	fF_tab1a_fGrp1a->MapSubwindows();
	fF_tab1a_fGrp1a->MapWindow();
	fF_tab1a_fGrp1a->MoveResize(2, 20, 380, 350);

	//******************************************************************

	// "VME FPGA: External Gate/Veto Condition"
	fF_tab1a_fGrp1ar = new TGGroupFrame(fTGHor_tab1a, "VME FPGA: External Veto/Gate Condition");
	//fTGHor_tab1a->AddFrame(fF_tab1a_fGrp1ar, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 5, 5, 2, 3));
	fTGHor_tab1a->AddFrame(fF_tab1a_fGrp1ar, new TGLayoutHints(kLHintsExpandX, 0, 0, 2, 3));
	//fF_tab1a_fGrp1ar->SetLayoutBroken(kTRUE);

	fF5Br = new TGCompositeFrame(fF_tab1a_fGrp1ar, 60, 20, kVerticalFrame);
	fF_tab1a_fGrp1ar->AddFrame(fF5Br, new TGLayoutHints(kLHintsTop | kLHintsLeft, 0, 0, 5, 5));



	fChkExternalTriggerFuncAsVeto = new TGCheckButton(fF5Br, "VME FPGA: External Trigger function as Veto/Gate Enable", 0);
	fF5Br->AddFrame(fChkExternalTriggerFuncAsVeto, new TGLayoutHints(kLHintsTop | kLHintsLeft, 0, 2, 15, 2));
	fChkExternalTriggerFuncAsVeto->SetState(kButtonUp); // is OFF !

	fChkLocalVetoFuncAsVeto = new TGCheckButton(fF5Br, "VME FPGA: Local Veto function as Veto/Gate Enable", 0);
	fF5Br->AddFrame(fChkLocalVetoFuncAsVeto, new TGLayoutHints(kLHintsTop | kLHintsLeft, 0, 2, 10, 2));
	fChkLocalVetoFuncAsVeto->SetState(kButtonUp); // is OFF !

	fChkLemoInUiAsVetoEnable = new TGCheckButton(fF5Br, "VME FPGA: Lemo Input UI use as Local Veto function Enable", 0);
	fF5Br->AddFrame(fChkLemoInUiAsVetoEnable, new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 2, 5, 2));
	fChkLemoInUiAsVetoEnable->SetState(kButtonUp); // is OFF !



	fF_tab1a_fGrp1ar_Veto_Delay = new TGHorizontalFrame(fF5Br, 200, 30);
	fF5Br->AddFrame(fF_tab1a_fGrp1ar_Veto_Delay, new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 0, 2, 10, 2));// hints, left, right, top, bottom

	fNumericEntries_VetoDelay = new TGNumberEntry(fF_tab1a_fGrp1ar_Veto_Delay, 0, 12, 0, (TGNumberFormat::kNESInteger)); //kNESHex
	fF_tab1a_fGrp1ar_Veto_Delay->AddFrame(fNumericEntries_VetoDelay, new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2));
	fLabel_VetoDelay = new TGLabel(fF_tab1a_fGrp1ar_Veto_Delay, "External Veto/Gate Delay", myGC(), myfont->GetFontStruct());
	fF_tab1a_fGrp1ar_Veto_Delay->AddFrame(fLabel_VetoDelay, new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 5, 2, 5, 2));

	fNumericEntries_VetoDelay->SetLimits((TGNumberFormat::kNELLimitMinMax), 0, 2044);
	fNumericEntries_VetoDelay->SetIntNumber(0); //  

	fF_tab1a_fGrp1ar->MapSubwindows();
	fF_tab1a_fGrp1ar->MapWindow();
	fF_tab1a_fGrp1ar->MoveResize(390, 20, 380, 170);


	//******************************************************************

	// ADC FPGAs: Internal Trigger Delay
	fF_tab1a_fGrp1ar1 = new TGGroupFrame(fTGHor_tab1a, "ADC FPGAs: Internal Trigger Delay");
	fTGHor_tab1a->AddFrame(fF_tab1a_fGrp1ar1, new TGLayoutHints(kLHintsExpandX, 0, 0, 5, 3));
	//fF_tab1a_fGrp1ar1->SetLayoutBroken(kTRUE);

	fF5Br1 = new TGCompositeFrame(fF_tab1a_fGrp1ar1, 60, 20, kVerticalFrame);
	fF_tab1a_fGrp1ar1->AddFrame(fF5Br1, new TGLayoutHints(kLHintsTop | kLHintsLeft, 0, 0, 10, 5));


	fF_tab1a_fGrp1ar_InternalDelay = new TGHorizontalFrame(fF5Br1, 200, 30);
	fF5Br1->AddFrame(fF_tab1a_fGrp1ar_InternalDelay, new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 1, 2));// hints, left, right, top, bottom

	fNumericEntries_InternalDelay = new TGNumberEntry(fF_tab1a_fGrp1ar_InternalDelay, 0, 12, 0, (TGNumberFormat::kNESInteger)); //kNESHex
	fF_tab1a_fGrp1ar_InternalDelay->AddFrame(fNumericEntries_InternalDelay, new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2));
	fLabel_InternalDelay = new TGLabel(fF_tab1a_fGrp1ar_InternalDelay, "Internal Trigger Delay", myGC(), myfont->GetFontStruct());
	fF_tab1a_fGrp1ar_InternalDelay->AddFrame(fLabel_InternalDelay, new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 5, 2, 5, 2));

	fNumericEntries_InternalDelay->SetLimits((TGNumberFormat::kNELLimitMinMax), 0, 510);
	fNumericEntries_InternalDelay->SetIntNumber(0); //  


	fF_tab1a_fGrp1ar1->MapSubwindows();
	fF_tab1a_fGrp1ar1->MapWindow();
	fF_tab1a_fGrp1ar1->MoveResize(390, 190, 380, 180);



	//******************************************************************

	// "ADC FPGAs: Individual External Trigger"
	fF_tab1a_fGrp1b = new TGGroupFrame(fTGHor_tab1a, "ADC FPGAs: External Trigger Enable");
	fTGHor_tab1a->AddFrame(fF_tab1a_fGrp1b, new TGLayoutHints( kLHintsExpandX, 5, 5, 2, 3));
	//fTGHor_tab1a->AddFrame(fF_tab1a_fGrp1b, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 5, 5, 2, 3));
	fF_tab1a_fGrp1b->SetLayoutBroken(kTRUE);

	fExtTriggerEnableCh_Set = new TGTextButton(fF_tab1a_fGrp1b, "Enable all Channels", SIS3316TestDialog_kCM_BUTTON_EXT_IRQ_NO_10);
	fExtTriggerEnableCh_Clr = new TGTextButton(fF_tab1a_fGrp1b, "Disable all Channels", SIS3316TestDialog_kCM_BUTTON_EXT_IRQ_NO_11);
	fExtTriggerEnableCh_Set->ChangeBackground(yellow);
	fExtTriggerEnableCh_Clr->ChangeBackground(yellow);
	fExtTriggerEnableCh_Set->Associate(this);
	fExtTriggerEnableCh_Clr->Associate(this);
	fExtTriggerEnableCh_Set->MoveResize(20, 25, 140, 25);
	fExtTriggerEnableCh_Clr->MoveResize(180, 25, 140, 25);

	for (i = 0; i < 4; i++) {
		fTGHor_tab1a_grp1b_sub[i] = new TGHorizontalFrame(fF_tab1a_fGrp1b, 360, 220, kHorizontalFrame);
		fF_tab1a_fGrp1b->AddFrame(fTGHor_tab1a_grp1b_sub[i], new TGLayoutHints(kLHintsExpandX, 5, 5, 15, 5));
		fTGHor_tab1a_grp1b_sub[i]->MoveResize(20, 65 + ((i) * 20), 320, 17);

		fChkExtTriggerEnableCh[15 - i] = new TGCheckButton(fTGHor_tab1a_grp1b_sub[i], chkChLabel[15 - i], 0);
		fChkExtTriggerEnableCh[15 - i]->MoveResize(0, 0, 70, 15);
		fChkExtTriggerEnableCh[15 - i]->SetState(kButtonDown); // is ON !

		fChkExtTriggerEnableCh[11 - i] = new TGCheckButton(fTGHor_tab1a_grp1b_sub[i], chkChLabel[11 - i], 0);
		fChkExtTriggerEnableCh[11 - i]->MoveResize(80, 0, 70, 15);
		fChkExtTriggerEnableCh[11 - i]->SetState(kButtonDown); // is ON !

		fChkExtTriggerEnableCh[7 - i] = new TGCheckButton(fTGHor_tab1a_grp1b_sub[i], chkChLabel[7 - i], 0);
		fChkExtTriggerEnableCh[7 - i]->MoveResize(160, 0, 70, 15);
		fChkExtTriggerEnableCh[7 - i]->SetState(kButtonDown); // is ON !

		fChkExtTriggerEnableCh[3 - i] = new TGCheckButton(fTGHor_tab1a_grp1b_sub[i], chkChLabel[3 - i], 0);
		fChkExtTriggerEnableCh[3 - i]->MoveResize(240, 0, 70, 15);
		fChkExtTriggerEnableCh[3 - i]->SetState(kButtonDown); // is ON !
	}


	fF_tab1a_fGrp1b->MoveResize(2, 380, 380, 165);
	fF_tab1a_fGrp1b->MapSubwindows();
	fF_tab1a_fGrp1b->MapWindow();



	//******************** 
	
	// "ADC FPGAs:  Internal Trigger Enable"
	fF_tab1a_fGrp1c = new TGGroupFrame(fF_tab1a, "ADC FPGAs: Internal Trigger Enable");
	fF_tab1a->AddFrame(fF_tab1a_fGrp1c, new TGLayoutHints(kLHintsExpandX, 5, 5, 2, 3));

	fF_tab1a_fGrp1c->SetLayoutBroken(kTRUE);

	fIntTriggerEnableCh_Set = new TGTextButton(fF_tab1a_fGrp1c, "Enable all Channels", SIS3316TestDialog_kCM_BUTTON_INT_IRQ_NO_12);
	fIntTriggerEnableCh_Clr = new TGTextButton(fF_tab1a_fGrp1c, "Disable all Channels", SIS3316TestDialog_kCM_BUTTON_INT_IRQ_NO_13);
	fIntTriggerEnableCh_Set->ChangeBackground(yellow);
	fIntTriggerEnableCh_Clr->ChangeBackground(yellow);
	fIntTriggerEnableCh_Set->Associate(this);
	fIntTriggerEnableCh_Clr->Associate(this);
	fIntTriggerEnableCh_Set->MoveResize(20, 25, 140, 25);
	fIntTriggerEnableCh_Clr->MoveResize(180, 25, 140, 25);
	for (i = 0; i < 4; i++) {
		fTGHor_tab1a_grp1c_sub[i] = new TGHorizontalFrame(fF_tab1a_fGrp1c, 360, 220, kHorizontalFrame);
		fF_tab1a_fGrp1c->AddFrame(fTGHor_tab1a_grp1c_sub[i], new TGLayoutHints(kLHintsExpandX, 5, 5, 15, 5));
		fTGHor_tab1a_grp1c_sub[i]->MoveResize(20, 65 + ((i) * 20), 320, 17);

		fChkIntTriggerEnableCh[15 - i] = new TGCheckButton(fTGHor_tab1a_grp1c_sub[i], chkChLabel[15 - i], 0);
		fChkIntTriggerEnableCh[15 - i]->MoveResize(0, 0, 70, 15);
		fChkIntTriggerEnableCh[15 - i]->SetState(kButtonUp); // is OFF !

		fChkIntTriggerEnableCh[11 - i] = new TGCheckButton(fTGHor_tab1a_grp1c_sub[i], chkChLabel[11 - i], 0);
		fChkIntTriggerEnableCh[11 - i]->MoveResize(80, 0, 70, 15);
		fChkIntTriggerEnableCh[11 - i]->SetState(kButtonUp); // is OFF !

		fChkIntTriggerEnableCh[7 - i] = new TGCheckButton(fTGHor_tab1a_grp1c_sub[i], chkChLabel[7 - i], 0);
		fChkIntTriggerEnableCh[7 - i]->MoveResize(160, 0, 70, 15);
		fChkIntTriggerEnableCh[7 - i]->SetState(kButtonUp); // is OFF !

		fChkIntTriggerEnableCh[3 - i] = new TGCheckButton(fTGHor_tab1a_grp1c_sub[i], chkChLabel[3 - i], 0);
		fChkIntTriggerEnableCh[3 - i]->MoveResize(240, 0, 70, 15);
		fChkIntTriggerEnableCh[3 - i]->SetState(kButtonUp); // is OFF !
	}

	fF_tab1a_fGrp1c->MapSubwindows();
	fF_tab1a_fGrp1c->MapWindow();
	fF_tab1a_fGrp1c->MoveResize(390, 380, 380, 165);

	//******************************************************************




	 

	//******************************************************************

	// "ADC FPGAs: Internal SUM-Trigger"
	fF_tab1a_fGrp1d = new TGGroupFrame(fTGHor_tab1a, "ADC FPGAs: Internal SUM-Trigger Enable");
	fTGHor_tab1a->AddFrame(fF_tab1a_fGrp1d, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 5, 5, 2, 3));
	fF_tab1a_fGrp1d->SetLayoutBroken(kTRUE);

	fIntSumTriggerEnableCh_Set = new TGTextButton(fF_tab1a_fGrp1d, "Enable all Channels", SIS3316TestDialog_kCM_BUTTON_INTSUM_IRQ_NO_14);
	fIntSumTriggerEnableCh_Clr = new TGTextButton(fF_tab1a_fGrp1d, "Disable all Channels", SIS3316TestDialog_kCM_BUTTON_INTSUM_IRQ_NO_15);
	fIntSumTriggerEnableCh_Set->ChangeBackground(yellow);
	fIntSumTriggerEnableCh_Clr->ChangeBackground(yellow);
	fIntSumTriggerEnableCh_Set->Associate(this);
	fIntSumTriggerEnableCh_Clr->Associate(this);
	fIntSumTriggerEnableCh_Set->MoveResize(20, 25, 140, 25);
	fIntSumTriggerEnableCh_Clr->MoveResize(180, 25, 140, 25);

	for (i = 0; i < 4; i++) {
		fTGHor_tab1a_grp1d_sub[i] = new TGHorizontalFrame(fF_tab1a_fGrp1d, 360, 220, kHorizontalFrame);
		fF_tab1a_fGrp1d->AddFrame(fTGHor_tab1a_grp1d_sub[i], new TGLayoutHints(kLHintsExpandX, 5, 5, 15, 5));
		fTGHor_tab1a_grp1d_sub[i]->MoveResize(20, 65 + ((i) * 20), 320, 17);

		fChkIntSumTriggerEnableCh[15 - i] = new TGCheckButton(fTGHor_tab1a_grp1d_sub[i], chkChLabel[15 - i], 0);
		fChkIntSumTriggerEnableCh[15 - i]->MoveResize(0, 0, 70, 15);
		fChkIntSumTriggerEnableCh[15 - i]->SetState(kButtonUp); // is OFF !

		fChkIntSumTriggerEnableCh[11 - i] = new TGCheckButton(fTGHor_tab1a_grp1d_sub[i], chkChLabel[11 - i], 0);
		fChkIntSumTriggerEnableCh[11 - i]->MoveResize(80, 0, 70, 15);
		fChkIntSumTriggerEnableCh[11 - i]->SetState(kButtonUp); // is OFF !

		fChkIntSumTriggerEnableCh[7 - i] = new TGCheckButton(fTGHor_tab1a_grp1d_sub[i], chkChLabel[7 - i], 0);
		fChkIntSumTriggerEnableCh[7 - i]->MoveResize(160, 0, 70, 15);
		fChkIntSumTriggerEnableCh[7 - i]->SetState(kButtonUp); // is OFF !

		fChkIntSumTriggerEnableCh[3 - i] = new TGCheckButton(fTGHor_tab1a_grp1d_sub[i], chkChLabel[3 - i], 0);
		fChkIntSumTriggerEnableCh[3 - i]->MoveResize(240, 0, 70, 15);
		fChkIntSumTriggerEnableCh[3 - i]->SetState(kButtonUp); // is OFF !
	}

	fF_tab1a_fGrp1d->MapSubwindows();
	fF_tab1a_fGrp1d->MapWindow();
	fF_tab1a_fGrp1d->MoveResize(2, 550, 380, 165);


	

	//******************** 

	// "ADC FPGAs:  Internal Pileup-Trigger Enable"
	fF_tab1a_fGrp1e = new TGGroupFrame(fF_tab1a, "ADC FPGAs: Internal Pileup-Trigger Enable");
	fF_tab1a->AddFrame(fF_tab1a_fGrp1e, new TGLayoutHints(kLHintsExpandX, 5, 5, 2, 3));
	fF_tab1a_fGrp1e->SetLayoutBroken(kTRUE);

	fIntPileupTriggerEnableCh_Set = new TGTextButton(fF_tab1a_fGrp1e, "Enable all Channels", SIS3316TestDialog_kCM_BUTTON_INTPILE_IRQ_NO_16);
	fIntPileupTriggerEnableCh_Clr = new TGTextButton(fF_tab1a_fGrp1e, "Disable all Channels", SIS3316TestDialog_kCM_BUTTON_INTPILE_IRQ_NO_17);
	fIntPileupTriggerEnableCh_Set->ChangeBackground(yellow);
	fIntPileupTriggerEnableCh_Clr->ChangeBackground(yellow);
	fIntPileupTriggerEnableCh_Set->Associate(this);
	fIntPileupTriggerEnableCh_Clr->Associate(this);
	fIntPileupTriggerEnableCh_Set->MoveResize(20, 25, 140, 25);
	fIntPileupTriggerEnableCh_Clr->MoveResize(180, 25, 140, 25);
	for (i = 0; i < 4; i++) {
		fTGHor_tab1a_grp1e_sub[i] = new TGHorizontalFrame(fF_tab1a_fGrp1e, 360, 220, kHorizontalFrame);
		fF_tab1a_fGrp1e->AddFrame(fTGHor_tab1a_grp1e_sub[i], new TGLayoutHints(kLHintsExpandX, 5, 5, 15, 5));
		fTGHor_tab1a_grp1e_sub[i]->MoveResize(20, 65 + ((i) * 20), 320, 17);

		fChkIntPileupTriggerEnableCh[15 - i] = new TGCheckButton(fTGHor_tab1a_grp1e_sub[i], chkChLabel[15 - i], 0);
		fChkIntPileupTriggerEnableCh[15 - i]->MoveResize(0, 0, 70, 15);
		fChkIntPileupTriggerEnableCh[15 - i]->SetState(kButtonUp); // is OFF !

		fChkIntPileupTriggerEnableCh[11 - i] = new TGCheckButton(fTGHor_tab1a_grp1e_sub[i], chkChLabel[11 - i], 0);
		fChkIntPileupTriggerEnableCh[11 - i]->MoveResize(80, 0, 70, 15);
		fChkIntPileupTriggerEnableCh[11 - i]->SetState(kButtonUp); // is OFF !

		fChkIntPileupTriggerEnableCh[7 - i] = new TGCheckButton(fTGHor_tab1a_grp1e_sub[i], chkChLabel[7 - i], 0);
		fChkIntPileupTriggerEnableCh[7 - i]->MoveResize(160, 0, 70, 15);
		fChkIntPileupTriggerEnableCh[7 - i]->SetState(kButtonUp); // is OFF !

		fChkIntPileupTriggerEnableCh[3 - i] = new TGCheckButton(fTGHor_tab1a_grp1e_sub[i], chkChLabel[3 - i], 0);
		fChkIntPileupTriggerEnableCh[3 - i]->MoveResize(240, 0, 70, 15);
		fChkIntPileupTriggerEnableCh[3 - i]->SetState(kButtonUp); // is OFF !
	}

	fF_tab1a_fGrp1e->MapSubwindows();
	fF_tab1a_fGrp1e->MapWindow();
	fF_tab1a_fGrp1e->MoveResize(390, 550, 380, 165);

	//******************************************************************






	//******************************************************************

	// "ADC FPGAs: External Gate"
	fF_tab1a_fGrp1g = new TGGroupFrame(fTGHor_tab1a, "ADC FPGAs: External Gate Enable");
	fTGHor_tab1a->AddFrame(fF_tab1a_fGrp1g, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 5, 5, 2, 3));
	fF_tab1a_fGrp1g->SetLayoutBroken(kTRUE);

	fExtGateEnableCh_Set = new TGTextButton(fF_tab1a_fGrp1g, "Enable all Channels", SIS3316TestDialog_kCM_BUTTON_EXTGATE_IRQ_NO_20);
	fExtGateEnableCh_Clr = new TGTextButton(fF_tab1a_fGrp1g, "Disable all Channels", SIS3316TestDialog_kCM_BUTTON_EXTGATE_IRQ_NO_21);
	fExtGateEnableCh_Set->ChangeBackground(yellow);
	fExtGateEnableCh_Clr->ChangeBackground(yellow);
	fExtGateEnableCh_Set->Associate(this);
	fExtGateEnableCh_Clr->Associate(this);
	fExtGateEnableCh_Set->MoveResize(20, 25, 140, 25);
	fExtGateEnableCh_Clr->MoveResize(180, 25, 140, 25);

 


	for (i = 0; i < 4; i++) {
		fTGHor_tab1a_grp1g_sub[i] = new TGHorizontalFrame(fF_tab1a_fGrp1g, 360, 220, kHorizontalFrame);
		fF_tab1a_fGrp1g->AddFrame(fTGHor_tab1a_grp1g_sub[i], new TGLayoutHints(kLHintsExpandX, 5, 5, 15, 5));
		fTGHor_tab1a_grp1g_sub[i]->MoveResize(20, 65 + ((i) * 20), 320, 17);

		fChkExtGateEnableCh[15 - i] = new TGCheckButton(fTGHor_tab1a_grp1g_sub[i], chkChLabel[15 - i], 0);
		fChkExtGateEnableCh[15 - i]->MoveResize(0, 0, 70, 15);
		fChkExtGateEnableCh[15 - i]->SetState(kButtonUp); // is OFF !

		fChkExtGateEnableCh[11 - i] = new TGCheckButton(fTGHor_tab1a_grp1g_sub[i], chkChLabel[11 - i], 0);
		fChkExtGateEnableCh[11 - i]->MoveResize(80, 0, 70, 15);
		fChkExtGateEnableCh[11 - i]->SetState(kButtonUp); // is OFF !

		fChkExtGateEnableCh[7 - i] = new TGCheckButton(fTGHor_tab1a_grp1g_sub[i], chkChLabel[7 - i], 0);
		fChkExtGateEnableCh[7 - i]->MoveResize(160, 0, 70, 15);
		fChkExtGateEnableCh[7 - i]->SetState(kButtonUp); // is OFF !

		fChkExtGateEnableCh[3 - i] = new TGCheckButton(fTGHor_tab1a_grp1g_sub[i], chkChLabel[3 - i], 0);
		fChkExtGateEnableCh[3 - i]->MoveResize(240, 0, 70, 15);
		fChkExtGateEnableCh[3 - i]->SetState(kButtonUp); // is OFF !
	}

	fF_tab1a_fGrp1g->MapSubwindows();
	fF_tab1a_fGrp1g->MapWindow();
	fF_tab1a_fGrp1g->MoveResize(2, 720, 380, 165);

	

	//******************** 
	

	// "ADC FPGAs: External Veto"
	fF_tab1a_fGrp1h = new TGGroupFrame(fF_tab1a, "ADC FPGAs: External Veto Enable");
	fF_tab1a->AddFrame(fF_tab1a_fGrp1h, new TGLayoutHints(kLHintsExpandX, 5, 5, 2, 3));
	fF_tab1a_fGrp1h->SetLayoutBroken(kTRUE);

	fExtVetoEnableCh_Set = new TGTextButton(fF_tab1a_fGrp1h, "Enable all Channels", SIS3316TestDialog_kCM_BUTTON_EXTVETO_IRQ_NO_22);
	fExtVetoEnableCh_Clr = new TGTextButton(fF_tab1a_fGrp1h, "Disable all Channels", SIS3316TestDialog_kCM_BUTTON_EXTVETO_IRQ_NO_23);
	fExtVetoEnableCh_Set->ChangeBackground(yellow);
	fExtVetoEnableCh_Clr->ChangeBackground(yellow);
	fExtVetoEnableCh_Set->Associate(this);
	fExtVetoEnableCh_Clr->Associate(this);
	fExtVetoEnableCh_Set->MoveResize(20, 25, 140, 25);
	fExtVetoEnableCh_Clr->MoveResize(180, 25, 140, 25);

 
	for (i = 0; i < 4; i++) {
		fTGHor_tab1a_grp1h_sub[i] = new TGHorizontalFrame(fF_tab1a_fGrp1h, 360, 220, kHorizontalFrame);
		fF_tab1a_fGrp1h->AddFrame(fTGHor_tab1a_grp1h_sub[i], new TGLayoutHints(kLHintsExpandX, 5, 5, 15, 5));
		fTGHor_tab1a_grp1h_sub[i]->MoveResize(20, 65 + ((i) * 20), 320, 17);

		fChkExtVetoEnableCh[15 - i] = new TGCheckButton(fTGHor_tab1a_grp1h_sub[i], chkChLabel[15 - i], 0);
		fChkExtVetoEnableCh[15 - i]->MoveResize(0, 0, 70, 15);
		fChkExtVetoEnableCh[15 - i]->SetState(kButtonUp); // is OFF !

		fChkExtVetoEnableCh[11 - i] = new TGCheckButton(fTGHor_tab1a_grp1h_sub[i], chkChLabel[11 - i], 0);
		fChkExtVetoEnableCh[11 - i]->MoveResize(80, 0, 70, 15);
		fChkExtVetoEnableCh[11 - i]->SetState(kButtonUp); // is OFF !

		fChkExtVetoEnableCh[7 - i] = new TGCheckButton(fTGHor_tab1a_grp1h_sub[i], chkChLabel[7 - i], 0);
		fChkExtVetoEnableCh[7 - i]->MoveResize(160, 0, 70, 15);
		fChkExtVetoEnableCh[7 - i]->SetState(kButtonUp); // is OFF !
		
		fChkExtVetoEnableCh[3 - i] = new TGCheckButton(fTGHor_tab1a_grp1h_sub[i], chkChLabel[3 - i], 0);
		fChkExtVetoEnableCh[3 - i]->MoveResize(240, 0, 70, 15);
		fChkExtVetoEnableCh[3 - i]->SetState(kButtonUp); // is OFF !
	}

	fF_tab1a_fGrp1h->MapSubwindows();
	fF_tab1a_fGrp1h->MapWindow();
	fF_tab1a_fGrp1h->MoveResize(390, 720, 380, 165);

	//******************************************************************


	tf->AddFrame(fF_tab1a, new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5, 5, 5));

 /**********************************************************************************************************/
 /**********************************************************************************************************/
 
 
 
 /**********************************************************************************************************/

 // Tab 2  : Display Control
	tf = fTab->AddTab("Display Control");
 	tabel_tab[2] = fTab->GetTabTab("Display Control");
	tabel_tab[2]->ChangeBackground(tab_color_not_active);
	this->sis3316Test1_nof_valid_tabel_tabs++ ;	

	fF_tab2 = new TGCompositeFrame(tf, 500, 20, kVerticalFrame);
	fF_tab2_fGrp1 = new TGGroupFrame(fF_tab2, "Display Raw data");
	fF_tab2->AddFrame(fF_tab2_fGrp1, new TGLayoutHints(kLHintsNormal | kLHintsExpandX, 5, 5, 5, 5));

	fF_tab2_fGrp1->SetLayoutBroken(kTRUE);

//   kLHintsNoHints = 0,
//   kLHintsLeft    = BIT(0),
//   kLHintsCenterX = BIT(1),
//   kLHintsRight   = BIT(2),
//   kLHintsTop     = BIT(3),
//   kLHintsCenterY = BIT(4),
//   kLHintsBottom  = BIT(5),
//   kLHintsExpandX = BIT(6),
//   kLHintsExpandY = BIT(7),
//   kLHintsNormal  = (kLHintsLeft | kLHintsTop)


	fF[0] = new TGHorizontalFrame(fF_tab2_fGrp1, 350, 30);
	fF_tab2_fGrp1->AddFrame(fF[0], new TGLayoutHints(kLHintsExpandX, 2, 2, 12, 2));

	fChkDisplayAutoZoom = new TGCheckButton(fF[0], "Y-Auto-Zoom", SIS3316TestDialog_kCM_CHECKBUTTON_IRQ_NO_21);
	fChkDisplayAutoZoom->SetState(kButtonDown); // is ON !
	fChkDisplayAutoZoom->Associate(this); // Event (IRQ) anmelden
	fF[0]->AddFrame(fChkDisplayAutoZoom, new TGLayoutHints(kLHintsLeft , 2, 2, 2, 2)); //hints, left, right, top, bottom

	fChkDisplayDisableDeleteGraph = new TGCheckButton(fF[0], "Disable Delete Graph (Root Zoom)", SIS3316TestDialog_kCM_CHECKBUTTON_IRQ_NO_21);
	fChkDisplayDisableDeleteGraph->Associate(this); // Event (IRQ) anmelden
	fChkDisplayDisableDeleteGraph->SetState(kButtonUp); // is Off !
	fF[0]->AddFrame(fChkDisplayDisableDeleteGraph, new TGLayoutHints(kLHintsNormal | kLHintsExpandX, 22, 2, 2, 2)); //hints, left, right, top, bottom
	fChkDisplayDisableDeleteGraph->SetEnabled(kFALSE); // dim

	fF[0]->MoveResize(20, 25, 340, 25);


	fF[1] = new TGHorizontalFrame(fF_tab2_fGrp1, 200, 30);
	fF_tab2_fGrp1->AddFrame(fF[1], new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,  2, 2, 2, 2));

	fNumericEntriesGraph_Yaxis[0] = new TGNumberEntry(fF[1], 0 /* value */, 8 /* width */, SIS3316TestDialog_kCM_ENTRY_IRQ_NO_21 /* irq */ , (TGNumberFormat::kNESInteger) ) ; //kNESHex
	fNumericEntriesGraph_Yaxis[0]->Associate(this); // Event (IRQ) anmelden
	fF[1]->AddFrame(fNumericEntriesGraph_Yaxis[0], new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2));
	fLabel[0] = new TGLabel(fF[1], "Y-max", myGC(), myfont->GetFontStruct());
	fF[1]->AddFrame(fLabel[0], new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2));

	fNumericEntriesGraph_Xaxis[0] = new TGNumberEntry(fF[1], 0 /* value */, 8 /* width */, SIS3316TestDialog_kCM_ENTRY_IRQ_NO_23 /* irq */, (TGNumberFormat::kNESInteger)); //kNESHex
	fNumericEntriesGraph_Xaxis[0]->Associate(this); // Event (IRQ) anmelden
	fF[1]->AddFrame(fNumericEntriesGraph_Xaxis[0], new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 76, 2, 2, 2));
	fLabel[0] = new TGLabel(fF[1], "X-max", myGC(), myfont->GetFontStruct());
	fF[1]->AddFrame(fLabel[0], new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2));

	fF[1]->MoveResize(20, 50, 330, 25);

	fF[2] = new TGHorizontalFrame(fF_tab2_fGrp1, 200, 30);
	fF_tab2_fGrp1->AddFrame(fF[2], new TGLayoutHints(kLHintsTop | kLHintsLeft ,  2, 2, 2, 2));
	fNumericEntriesGraph_Yaxis[1] = new TGNumberEntry(fF[2], 0 /* value */, 8 /* width */, SIS3316TestDialog_kCM_ENTRY_IRQ_NO_20 /* irq */ , (TGNumberFormat::kNESInteger) ) ; //kNESHex
	fNumericEntriesGraph_Yaxis[1]->Associate(this); // Event (IRQ) anmelden
	fF[2]->AddFrame(fNumericEntriesGraph_Yaxis[1],  new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2));
	fLabel[1] = new TGLabel(fF[2], "Y-min", myGC(), myfont->GetFontStruct());
	fF[2]->AddFrame(fLabel[1],  new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2));

	fNumericEntriesGraph_Xaxis[1] = new TGNumberEntry(fF[2], 0 /* value */, 8 /* width */, SIS3316TestDialog_kCM_ENTRY_IRQ_NO_22 /* irq */, (TGNumberFormat::kNESInteger)); //kNESHex
	fNumericEntriesGraph_Xaxis[1]->Associate(this);
	fF[2]->AddFrame(fNumericEntriesGraph_Xaxis[1], new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 80, 2, 2, 2));
	fLabel[1] = new TGLabel(fF[2], "X-min", myGC(), myfont->GetFontStruct());
	fF[2]->AddFrame(fLabel[1], new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2));

	fF[2]->MoveResize(20, 80, 330, 25);

#ifdef RUN_WITHOUT_HARDWARE
		fNumericEntriesGraph_Yaxis[0]->SetIntNumber(65536); // Y-max
#else
	if (gl_sis3316_adc1->adc_125MHz_flag == 1) {
		fNumericEntriesGraph_Yaxis[0]->SetIntNumber(65536); // Y-max	
		this->raw_graph_ymax = 65536;
//		this->raw_graph_ymax_old = 65536;
		this->raw_graph_ymax_absolute = 65536;
	}
	else {
		fNumericEntriesGraph_Yaxis[0]->SetIntNumber(16384); // Y-max
		this->raw_graph_ymax = 16384;
//		this->raw_graph_ymax_old = 16384;
		this->raw_graph_ymax_absolute = 16384;
	}
#endif
	//fNumericEntriesGraph_Yaxis[0]->SetIntNumber(16384); // Y-max
	fNumericEntriesGraph_Yaxis[1]->SetIntNumber(0); // Y-min

	fNumericEntriesGraph_Yaxis[0]->SetState(kFALSE); // dim
	fNumericEntriesGraph_Yaxis[1]->SetState(kFALSE); // dim

	this->raw_graph_ymin     = 0;
//	this->raw_graph_ymin_old = 0;

	
 
	fNumericEntriesGraph_Xaxis[0]->SetIntNumber(this->raw_sample_length); // X-max
	fNumericEntriesGraph_Xaxis[0]->SetLimits((TGNumberFormat::kNELLimitMinMax), 10, this->raw_sample_length); //X - max

	fNumericEntriesGraph_Xaxis[1]->SetIntNumber(0); // X-min
	if (this->raw_sample_length > 10) { // Sample Length
		fNumericEntriesGraph_Xaxis[1]->SetLimits((TGNumberFormat::kNELLimitMinMax), 0, this->raw_sample_length); //X - min
	}
	else {
		fNumericEntriesGraph_Xaxis[1]->SetLimits((TGNumberFormat::kNELLimitMinMax), 0, 0); //X - min
	}

	this->raw_graph_xmin = 0;

	this->raw_graph_xmax = this->raw_sample_length;
	this->raw_graph_xmax_absolute = this->raw_sample_length;
	
//************


 	fF[3] = new TGHorizontalFrame(fF_tab2_fGrp1, 200, 30);
 	fF_tab2_fGrp1->AddFrame(fF[3], new TGLayoutHints(kLHintsTop | kLHintsLeft ,  2, 2, 2, 5));

 	fVF[3] = new TGVerticalFrame(fF[3], 200, 25);
 	fF[3]->AddFrame(fVF[3], new TGLayoutHints(kLHintsTop | kLHintsLeft ,  2, 2, 2, 5));

 	fVF[4] = new TGVerticalFrame(fF[3], 200, 25);
 	fF[3]->AddFrame(fVF[4], new TGLayoutHints(kLHintsTop | kLHintsLeft ,  2, 2, 2, 5));

   fDisplayEnableCh_Set = new TGTextButton(fVF[3], "&Enable all Channels ", 50);
   fDisplayEnableCh_Clr = new TGTextButton(fVF[4], "&Disable all Channels", 51);
   fDisplayEnableCh_Set->ChangeBackground(yellow);
   fDisplayEnableCh_Clr->ChangeBackground(yellow);

   fDisplayEnableCh_Set->Associate(this);
   fDisplayEnableCh_Clr->Associate(this);

   fVF[3]->AddFrame(fDisplayEnableCh_Set, new TGLayoutHints(kLHintsTop | kLHintsLeft ,  2, 2, 2, 5));
   fVF[4]->AddFrame(fDisplayEnableCh_Clr, new TGLayoutHints(kLHintsTop | kLHintsLeft ,  2, 2, 2, 5));

	for (i = 0; i < 8; i++) {
		fChkDisplayAdc[15-i] = new TGCheckButton(fVF[3], chkDisAdcLabel[15-i], 16-i);
		fChkDisplayAdc[15-i]->SetState(kButtonDown)   ; // is ON !
		fVF[3]->AddFrame(fChkDisplayAdc[15-i], new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandY , 2, 2, 1, 0)); // hints, left, right, top, bottom
    }

	for (i = 0; i < 8; i++) {
		fChkDisplayAdc[7-i] = new TGCheckButton(fVF[4], chkDisAdcLabel[7-i], 8-i);
		fChkDisplayAdc[7-i]->SetState(kButtonDown)   ; // is ON !
		fVF[4]->AddFrame(fChkDisplayAdc[7-i], new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandY , 2, 2, 1, 0)); // hints, left, right, top, bottom
    }
	fF[3]->MoveResize(20, 110, 330, 190);


	fF_tab2_fGrp1->MapSubwindows();
	fF_tab2_fGrp1->MapWindow();
	fF_tab2_fGrp1->MoveResize(2, 20, 410, 310);


	// ************************************************************************************************************************************************************
	// Statistic Counters
	fF_tab2_fGrp5 = new TGGroupFrame(fF_tab2, "Display Statistic Counters at end of multievent acquisition");
	fF_tab2->AddFrame(fF_tab2_fGrp5, new TGLayoutHints(kLHintsExpandX, 5, 5, 5, 5));

	fChkDisplayStatisticCounters = new TGCheckButton(fF_tab2_fGrp5, "Display Statistic Counters", 0);
	fChkDisplayStatisticCounters->SetState(kButtonDown); // is On !
	fF_tab2_fGrp5->AddFrame(fChkDisplayStatisticCounters, new TGLayoutHints(kLHintsLeft, 2, 2, 2, 2));



	// ************************************************************************************************************************************************************
	// Maw
	fF_tab2_fGrp4 = new TGGroupFrame(fF_tab2, "Display Moving Average Window (Test)");
	fF_tab2->AddFrame(fF_tab2_fGrp4, new TGLayoutHints(kLHintsExpandX, 5, 5, 5, 5));

	fCombo_Display_MAW = new TGComboBox(fF_tab2_fGrp4, 90);
	//fCombo_Display_MAW->Associate(this); // Event (IRQ) anmelden
	fF_tab2_fGrp4->AddFrame(fCombo_Display_MAW, new TGLayoutHints(kLHintsLeft, 5, 2, 2, 2));
	for (i = 0; i < 17; i++) {
		fCombo_Display_MAW->AddEntry(entryMawLabel[i], i);
	}
	fCombo_Display_MAW->Select(1, kTRUE); // display ch1
	fCombo_Display_MAW->Resize(250, 22);



// ************************************************************************************************************************************************************


// histograms
	fF_tab2_fGrp2 = new TGGroupFrame(fF_tab2, "Build and Display Histograms");
	fF_tab2->AddFrame(fF_tab2_fGrp2, new TGLayoutHints(kLHintsExpandX, 5, 5, 5, 5));

/******************/	

	fCombo_Display_Histos_Build = new TGComboBox(fF_tab2_fGrp2, 88);
	//fCombo_Display_Histos_Ch->Associate(this); // Event (IRQ) anmelden
	fF_tab2_fGrp2->AddFrame(fCombo_Display_Histos_Build, new TGLayoutHints(kLHintsLeft ,  2, 2, 15, 2));

	for (i = 0; i < 3; i++) {
		fCombo_Display_Histos_Build->AddEntry(entryHistoDisplayOption[i], i);
	}
	fCombo_Display_Histos_Build->Select(0, kTRUE); // build ADC output code
	fCombo_Display_Histos_Build->SetEnabled(kFALSE); // dim

	fCombo_Display_Histos_Build->Resize(250, 22);
/******************/
	fCombo_Display_Histos_Ch = new TGComboBox(fF_tab2_fGrp2, 88);
	//fCombo_Display_Histos_Ch->Associate(this); // Event (IRQ) anmelden
	fF_tab2_fGrp2->AddFrame(fCombo_Display_Histos_Ch, new TGLayoutHints(kLHintsLeft ,  2, 2, 2, 2));

	for (i = 0; i < 19; i++) {
		fCombo_Display_Histos_Ch->AddEntry(entryHistoLabel[i], i);
	}
	fCombo_Display_Histos_Ch->Select(2, kTRUE); // display ch1
	fCombo_Display_Histos_Ch->Resize(250, 22);

	fChkHistoSum = new TGCheckButton(fF_tab2_fGrp2, "Histogram Sum", 0);
	fChkHistoSum->SetState(kButtonUp)   ; // is Off !
	fF_tab2_fGrp2->AddFrame(fChkHistoSum, new TGLayoutHints(kLHintsLeft, 2, 2, 2, 2));

	fChkHistoZoomMean = new TGCheckButton(fF_tab2_fGrp2, "Histogram Zoom to Mean", 0);
	fChkHistoZoomMean->SetState(kButtonUp)   ; // is Off !
	fF_tab2_fGrp2->AddFrame(fChkHistoZoomMean, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 2, 2, 2, 2));

	fChkHistoGaussFit = new TGCheckButton(fF_tab2_fGrp2, "Histogram Gauss Fit", 0);
	fChkHistoGaussFit->SetState(kButtonUp)   ; // is Off !
	fF_tab2_fGrp2->AddFrame(fChkHistoGaussFit, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandY, 2, 2, 2, 2));



	fF[10] = new TGHorizontalFrame(fF_tab2_fGrp2, 200, 30);
	fF_tab2_fGrp2->AddFrame(fF[10], new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,  2, 2, 2, 2));
	fNumericEntriesHistogramXaxisOffset = new TGNumberEntry(fF[10], 0 /* value */, 8 /* width */, 120 /* irq */ , (TGNumberFormat::kNESInteger) ) ; //kNESHex
	//fNumericEntries[i]->Associate(this); // Event (IRQ) anmelden
	fF[10]->AddFrame(fNumericEntriesHistogramXaxisOffset,  new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2));
	fLabel[0] = new TGLabel(fF[10], "X-axis Offset", myGC(), myfont->GetFontStruct());
	fF[10]->AddFrame(fLabel[0],  new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2));

	fNumericEntriesHisto_Xaxis[0] = new TGNumberEntry(fF[10], 0 /* value */, 8 /* width */, SIS3316TestDialog_kCM_ENTRY_IRQ_NO_25 /* irq */, (TGNumberFormat::kNESInteger)); //kNESHex
	fNumericEntriesHisto_Xaxis[0]->Associate(this); // Event (IRQ) anmelden
	fF[10]->AddFrame(fNumericEntriesHisto_Xaxis[0], new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 90, 2, 2, 2));
	fLabel[0] = new TGLabel(fF[10], "X-max", myGC(), myfont->GetFontStruct());
	fF[10]->AddFrame(fLabel[0], new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2));




	fF[11] = new TGHorizontalFrame(fF_tab2_fGrp2, 200, 30);
	fF_tab2_fGrp2->AddFrame(fF[11], new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,  2, 2, 2, 2));
	fNumericEntriesHistogramXaxisDivider = new TGNumberEntry(fF[11], 0 /* value */, 8 /* width */, 120 /* irq */ , (TGNumberFormat::kNESInteger) ) ; //kNESHex
	//fNumericEntries[i]->Associate(this); // Event (IRQ) anmelden
	fF[11]->AddFrame(fNumericEntriesHistogramXaxisDivider,  new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2));
	fLabel[0] = new TGLabel(fF[11], "X-axis Divider (1/Gain)", myGC(), myfont->GetFontStruct());
	fF[11]->AddFrame(fLabel[0],  new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2));


	fNumericEntriesHisto_Xaxis[1] = new TGNumberEntry(fF[11], 0 /* value */, 8 /* width */, SIS3316TestDialog_kCM_ENTRY_IRQ_NO_24 /* irq */, (TGNumberFormat::kNESInteger)); //kNESHex
	fNumericEntriesHisto_Xaxis[1]->Associate(this);
	fF[11]->AddFrame(fNumericEntriesHisto_Xaxis[1], new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 40, 2, 2, 2));
	fLabel[0] = new TGLabel(fF[11], "X-min", myGC(), myfont->GetFontStruct());
	fF[11]->AddFrame(fLabel[0], new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2));


	fNumericEntriesHisto_Xaxis[0]->SetIntNumber(this->root_histo_xmax_absolute); // X-max
	fNumericEntriesHisto_Xaxis[0]->SetLimits((TGNumberFormat::kNELLimitMinMax), 10, this->root_histo_xmax_absolute); //X - max

	fNumericEntriesHisto_Xaxis[1]->SetIntNumber(this->root_histo_xmin); // X-min
	fNumericEntriesHisto_Xaxis[1]->SetLimits((TGNumberFormat::kNELLimitMinMax), 0, this->root_histo_xmax_absolute - 10); //X - min

	fNumericEntriesHistogramXaxisOffset->SetIntNumber(0);
	fNumericEntriesHistogramXaxisDivider->SetNumber(1);
	

/******************************************************************/




// FFT
	fF_tab2_fGrp3 = new TGGroupFrame(fF_tab2, "Display FFT ");
	fF_tab2->AddFrame(fF_tab2_fGrp3, new TGLayoutHints(kLHintsExpandX, 5, 5, 5, 5));

	fCombo_Display_FFT_Ch = new TGComboBox(fF_tab2_fGrp3, 89);
	//fCombo_Display_FFT_Ch->Associate(this); // Event (IRQ) anmelden

    fF_tab2_fGrp3->AddFrame(fCombo_Display_FFT_Ch, new TGLayoutHints(kLHintsLeft, 2, 2, 15, 2));

   for (i = 0; i < 17; i++) {
      fCombo_Display_FFT_Ch->AddEntry(entryDisplayFFTLabel[i], i);
   }
   fCombo_Display_FFT_Ch->Select(0, kTRUE); // no display ch
   fCombo_Display_FFT_Ch->Resize(250, 22);


   fCombo_Display_FFT_Window = new TGComboBox(fF_tab2_fGrp3, 90);
   //fCombo_Display_FFT_Window->Associate(this); // Event (IRQ) anmelden

   fF_tab2_fGrp3->AddFrame(fCombo_Display_FFT_Window, new TGLayoutHints(kLHintsLeft, 2, 2, 2, 2));
   for (i = 0; i < 6; i++) {
      fCombo_Display_FFT_Window->AddEntry(entryDisplayFFTWindowLabel[i], i);
   }
   fCombo_Display_FFT_Window->Select(4, kTRUE); // d
   fCombo_Display_FFT_Window->Resize(250, 22);


   fChkFFT_Sum = new TGCheckButton(fF_tab2_fGrp3, "FFT Spectrum Sum", 100);
   fChkFFT_Sum->Associate(this); // Event (IRQ) anmelden
   fChkFFT_Sum->SetState(kButtonUp)   ; // is Off !
   fChkFFT_Sum->SetEnabled(kFALSE)   ; //
   fF_tab2_fGrp3->AddFrame(fChkFFT_Sum, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 2, 2, 2, 2));

   fChkFFTLogY = new TGCheckButton(fF_tab2_fGrp3, "FFT Display LogY", 101);
   fChkFFTLogY->Associate(this); // Event (IRQ) anmelden
   fChkFFTLogY->SetState(kButtonUp)   ; // is Off !
   fChkFFTLogY->SetEnabled(kFALSE)   ; //
   fF_tab2_fGrp3->AddFrame(fChkFFTLogY, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 2, 2, 2, 2));

   fChkFFT_Db = new TGCheckButton(fF_tab2_fGrp3, "FFT in dB", 102);
   fChkFFT_Db->Associate(this); // Event (IRQ) anmelden
   fChkFFT_Db->SetEnabled(kTRUE)   ; //
   fChkFFT_Db->SetState(kButtonDown)   ; // is On !
   fF_tab2_fGrp3->AddFrame(fChkFFT_Db, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 2, 2, 2, 2));

   fChkFFT_AutoScale = new TGCheckButton(fF_tab2_fGrp3, "FFT Autoscale", 0);
   fChkFFT_AutoScale->SetState(kButtonUp)   ; // is Off !
   fF_tab2_fGrp3->AddFrame(fChkFFT_AutoScale, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 2, 2, 2, 2));


/**********/

   //tf->AddFrame(fF_tab2, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 5, 5, 5, 5));
   tf->AddFrame(fF_tab2, new TGLayoutHints(kLHintsTop | kLHintsLeft , 5, 5, 5, 5));




/*****************************************************************************************************************************************************/

/**********************************************************************************************************/


	// Tab 2b  (Polarity)

   tf = fTab->AddTab("Polarity");
 	tabel_tab[3] = fTab->GetTabTab("Polarity");
	tabel_tab[3]->ChangeBackground(tab_color_not_active);
	this->sis3316Test1_nof_valid_tabel_tabs++ ;	
	fF_tab2b = new TGCompositeFrame(tf, 60, 20, kVerticalFrame);
 

   fF_tab2b_fGrp1 = new TGGroupFrame(fF_tab2b, "Channel Input Invert ");
   fF_tab2b->AddFrame(fF_tab2b_fGrp1, new TGLayoutHints(kLHintsExpandX, 5, 5, 5, 5));

   fF_tab2b_fGrp1->SetLayoutBroken(kTRUE);


   fInvertChannel_Set = new TGTextButton(fF_tab2b_fGrp1, "Set Invert all Channels", SIS3316TestDialog_kCM_BUTTON_IRQ_NO_70);
   fInvertChannel_Clr = new TGTextButton(fF_tab2b_fGrp1, "Clear Invert all Channels", SIS3316TestDialog_kCM_BUTTON_IRQ_NO_71);
   fInvertChannel_Set->ChangeBackground(yellow);
   fInvertChannel_Clr->ChangeBackground(yellow);
   fInvertChannel_Set->Resize(150, 25);
   fInvertChannel_Clr->Resize(150, 25);
   fInvertChannel_Set->Associate(this);
   fInvertChannel_Clr->Associate(this);

   fF_tab2b_fGrp1->AddFrame(fInvertChannel_Set, new TGLayoutHints(kLHintsTop | kLHintsLeft ,  2, 2, 12, 5));
   fF_tab2b_fGrp1->AddFrame(fInvertChannel_Clr, new TGLayoutHints(kLHintsTop | kLHintsLeft ,  2, 2, 5, 12));
   fInvertChannel_Set->MoveResize(20, 30, 150, 25);
   fInvertChannel_Clr->MoveResize(20, 65, 150, 25);


   for (i = 0; i < 16; i++) {
      fChkInvertChannel[15-i] = new TGCheckButton(fF_tab2b_fGrp1, chkTriggerEnableChLabel[15-i], SIS3316TestDialog_kCM_CHECKBUTTON_IRQ_NO_74);
      fChkInvertChannel[15-i]->SetState(kButtonUp)   ; // is OFF !
	  fChkInvertChannel[15-i]->MoveResize(20, 110 + ((i) * 20), 60, 15);
	  fChkInvertChannel[15-i]->Associate(this);
      fF_tab2b_fGrp1->AddFrame(fChkInvertChannel[15-i], new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 20, 2, 2, 2));
   }
   invert_parameter_has_changed_flag = 1 ;


   fF_tab2b_fGrp1->MapSubwindows();
   fF_tab2b_fGrp1->Resize(fF_tab2b_fGrp1->GetDefaultSize());
   fF_tab2b_fGrp1->MapWindow();
   fF_tab2b_fGrp1->Resize(200,460);

	tf->AddFrame(fF_tab2b, new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5, 5, 5));


 


/*****************************************************************************************************************************************************/



// Tab 4  (Gain / Offset)
 
   tf = fTab->AddTab("Gain/Offset");
 	tabel_tab[4] = fTab->GetTabTab("Gain/Offset");
	tabel_tab[4]->ChangeBackground(tab_color_not_active);
	this->sis3316Test1_nof_valid_tabel_tabs++ ;	
   //fL1 = new TGLayoutHints(kLHintsTop | kLHintsLeft ,  20 ,2, 2, 12); //hints, left, right, top, bottom

   fF_tab4 = new TGCompositeFrame(tf, 360, 20, kVerticalFrame);

   fTGHor_tab4a = new TGHorizontalFrame(fF_tab4, 360, 250, kHorizontalFrame);
   fF_tab4->AddFrame(fTGHor_tab4a, new TGLayoutHints(kLHintsExpandX, 5, 5, 15, 5));
 
   fF_tab4_fGrp1A = new TGGroupFrame(fTGHor_tab4a, "Channel Input 50 Ohm Termination");
   fTGHor_tab4a->AddFrame(fF_tab4_fGrp1A, new TGLayoutHints(kLHintsExpandX, 5, 5, 5, 5));
   fF_tab4_fGrp1A->SetLayoutBroken(kTRUE);


   fTerminationChannel_Set = new TGTextButton(fF_tab4_fGrp1A, "Set 50 Ohm Termination all Channels", SIS3316TestDialog_kCM_BUTTON_IRQ_NO_80);
   fTerminationChannel_Clr = new TGTextButton(fF_tab4_fGrp1A, "Clear 50 Ohm Termination all Channels", SIS3316TestDialog_kCM_BUTTON_IRQ_NO_81);
   fTerminationChannel_Set->ChangeBackground(yellow);
   fTerminationChannel_Clr->ChangeBackground(yellow);
   fTerminationChannel_Set->Resize(225, 25);
   fTerminationChannel_Clr->Resize(225, 25);
   fTerminationChannel_Set->Associate(this);
   fTerminationChannel_Clr->Associate(this);

   fF_tab4_fGrp1A->AddFrame(fTerminationChannel_Set, new TGLayoutHints(kLHintsTop | kLHintsLeft ,  2, 2, 12, 5));
   fF_tab4_fGrp1A->AddFrame(fTerminationChannel_Clr, new TGLayoutHints(kLHintsTop | kLHintsLeft ,  2, 2, 5, 12));
   fTerminationChannel_Set->MoveResize(20, 30, 225, 25);
   fTerminationChannel_Clr->MoveResize(20, 65, 225, 25);


   for (i = 0; i < 8; i++) {
		fTGHor_tab4a_1a_sub[i] = new TGHorizontalFrame(fF_tab4_fGrp1A, 360, 250, kHorizontalFrame);
		fF_tab4_fGrp1A->AddFrame(fTGHor_tab4a_1a_sub[i], new TGLayoutHints(kLHintsExpandX, 5, 5, 15, 5));
		fTGHor_tab4a_1a_sub[i]->MoveResize(20, 110 + ((i) * 20), 225, 17);

		fChkTerminationChannel[15-i] = new TGCheckButton(fTGHor_tab4a_1a_sub[i], chkChLabel[15-i], SIS3316TestDialog_kCM_CHECKBUTTON_IRQ_NO_84);
		fChkTerminationChannel[15-i]->SetState(kButtonDown)   ; // is On !
		fChkTerminationChannel[15-i]->Associate(this);
		fChkTerminationChannel[15-i]->MoveResize(0, 0, 70, 15);

		fChkTerminationChannel[7-i] = new TGCheckButton(fTGHor_tab4a_1a_sub[i], chkChLabel[7-i], SIS3316TestDialog_kCM_CHECKBUTTON_IRQ_NO_84);
		fChkTerminationChannel[7-i]->SetState(kButtonDown)   ; // is On !
		fChkTerminationChannel[7-i]->Associate(this);
		fChkTerminationChannel[7-i]->MoveResize(110, 0, 70, 15); 
		//fTGHor_tab4a_1a_sub[i]->AddFrame(fChkTerminationChannel[7-i], new TGLayoutHints(kLHintsExpandX, 5, 5, 15, 5));

   }
	gain_termination_parameter_has_changed_flag = 1 ;

	fLabel_TerminationChannel = new TGLabel(fF_tab4_fGrp1A,"checked 50 Ohm else 1K Ohm");
	fLabel_TerminationChannel->SetTextJustify(kTextLeft + kTextCenterX );
	fLabel_TerminationChannel->SetMargins(0,0,0,0);
	fLabel_TerminationChannel->SetWrapLength(-1);
    fLabel_TerminationChannel->MoveResize(20, 280, 200, 25);
	fF_tab4_fGrp1A->AddFrame(fLabel_TerminationChannel, new TGLayoutHints(kLHintsExpandX,2,2,15,2));

	fF_tab4_fGrp1A->MapSubwindows();
	fF_tab4_fGrp1A->Resize(fF_tab4_fGrp1A->GetDefaultSize());
	fF_tab4_fGrp1A->MapWindow();
	fF_tab4_fGrp1A->Resize(260,320);

/********************/
   fF_tab4_fGrp1B = new TGGroupFrame(fTGHor_tab4a, "Channel Input Range");
   fTGHor_tab4a->AddFrame(fF_tab4_fGrp1B, new TGLayoutHints(kLHintsExpandX, 5, 5, 5, 5));

   fF_tab4_fGrp1B->SetLayoutBroken(kTRUE);


   fInputRange0Channel_Set = new TGTextButton(fF_tab4_fGrp1B, "Set Input Range 0 all Channels", SIS3316TestDialog_kCM_BUTTON_IRQ_NO_82);
   fInputRange0Channel_Clr = new TGTextButton(fF_tab4_fGrp1B, "Clear Input Range 0 all Channels", SIS3316TestDialog_kCM_BUTTON_IRQ_NO_83);
   fInputRange0Channel_Set->ChangeBackground(yellow);
   fInputRange0Channel_Clr->ChangeBackground(yellow);
   fInputRange0Channel_Set->Resize(225, 25);
   fInputRange0Channel_Clr->Resize(225, 25);
   fInputRange0Channel_Set->Associate(this);
   fInputRange0Channel_Clr->Associate(this);

	fF_tab4_fGrp1B->AddFrame(fInputRange0Channel_Set, new TGLayoutHints(kLHintsTop | kLHintsLeft ,  2, 2, 12, 5));
	fF_tab4_fGrp1B->AddFrame(fInputRange0Channel_Clr, new TGLayoutHints(kLHintsTop | kLHintsLeft ,  2, 2, 5, 12));
	fInputRange0Channel_Set->MoveResize(20, 30, 225, 25);
	fInputRange0Channel_Clr->MoveResize(20, 65, 225, 25);


     for (i = 0; i < 8; i++) {
		fTGHor_tab4a_1b_sub[i] = new TGHorizontalFrame(fF_tab4_fGrp1B, 360, 250, kHorizontalFrame);
		fF_tab4_fGrp1B->AddFrame(fTGHor_tab4a_1b_sub[i], new TGLayoutHints(kLHintsExpandX, 5, 5, 15, 5));
		fTGHor_tab4a_1b_sub[i]->MoveResize(20, 110 + ((i) * 20), 225, 17);

		fChkInputRange0Channel[15-i] = new TGCheckButton(fTGHor_tab4a_1b_sub[i], chkChLabel[15-i], SIS3316TestDialog_kCM_CHECKBUTTON_IRQ_NO_84);
		fChkInputRange0Channel[15-i]->SetState(kButtonDown)   ; // is On !
		fChkInputRange0Channel[15-i]->Associate(this);
		fChkInputRange0Channel[15-i]->MoveResize(0, 0, 70, 15);

		fChkInputRange0Channel[7-i] = new TGCheckButton(fTGHor_tab4a_1b_sub[i], chkChLabel[7-i], SIS3316TestDialog_kCM_CHECKBUTTON_IRQ_NO_84);
		fChkInputRange0Channel[7-i]->SetState(kButtonDown)   ; // is On !
		fChkInputRange0Channel[7-i]->Associate(this);
		fChkInputRange0Channel[7-i]->MoveResize(110, 0, 70, 15); 
	//	fTGHor_tab4a_1b_sub[i]->AddFrame(fChkInputRange0Channel[7-i], new TGLayoutHints(kLHintsExpandX, 5, 5, 15, 5));

   }
 
	fLabel_InputRange0Channel = new TGLabel(fF_tab4_fGrp1B,"checked 5V else 2V ");
	fLabel_InputRange0Channel->SetTextJustify(kTextLeft + kTextCenterX );
	fLabel_InputRange0Channel->SetMargins(0,0,0,0);
	fLabel_InputRange0Channel->SetWrapLength(-1);
    fLabel_InputRange0Channel->MoveResize(20, 280, 200, 25);
	fF_tab4_fGrp1B->AddFrame(fLabel_InputRange0Channel, new TGLayoutHints(kLHintsExpandX,2,2,15,2));

	fF_tab4_fGrp1B->MapSubwindows();
	fF_tab4_fGrp1B->Resize(fF_tab4_fGrp1B->GetDefaultSize());
	fF_tab4_fGrp1B->MapWindow();
	fF_tab4_fGrp1B->Resize(260,320);



 
   fF_tab4_fGrp2 = new TGGroupFrame(fF_tab4, "Channel DAC Offset");
   fF_tab4->AddFrame(fF_tab4_fGrp2, new TGLayoutHints(kLHintsExpandX, 5, 5, 15, 5));
   fF_tab4_fGrp2->SetLayoutBroken(kTRUE);

   for (i = 0; i < 8; i++) {
	  fF[i] = new TGHorizontalFrame(fF_tab4_fGrp2, 200, 25);
	  fF_tab4_fGrp2->AddFrame(fF[i], new TGLayoutHints(kLHintsExpandX, 5, 5, 15, 5));
	  fF[i]->MoveResize(20  , 30 + ((i) * 25), 440, 25);

	  fNumericEntriesAnalogOffset[15-i] = new TGNumberEntry(fF[i], 0 /* value */, 8 /* width */, SIS3316TestDialog_kCM_ENTRY_IRQ_NO_40 , (TGNumberFormat::kNESInteger) );
      fNumericEntriesAnalogOffset[15-i]->SetState(kButtonDown)   ; // is ON !
	  fNumericEntriesAnalogOffset[15-i]->SetIntNumber(0x8000); //
      fNumericEntriesAnalogOffset[15-i]->SetLimits((TGNumberFormat::kNELLimitMinMax), 0, 0xffff);
	  fNumericEntriesAnalogOffset[15-i]->Associate(this);
	  fNumericEntriesAnalogOffset[15-i]->MoveResize(0, 0, 70, 20);
	  fLabel[15-i] = new TGLabel(fF[i], chkChLabel[15-i], myGC(), myfont->GetFontStruct());
 	  fLabel[15-i]->MoveResize(85, 2, 40, 15);

	  fNumericEntriesAnalogOffset[7-i] = new TGNumberEntry(fF[i], 0 /* value */, 8 /* width */, SIS3316TestDialog_kCM_ENTRY_IRQ_NO_40 , (TGNumberFormat::kNESInteger) );
      fNumericEntriesAnalogOffset[7-i]->SetState(kButtonDown)   ; // is ON !
	  fNumericEntriesAnalogOffset[7-i]->SetIntNumber(0x8000); //
      fNumericEntriesAnalogOffset[7-i]->SetLimits((TGNumberFormat::kNELLimitMinMax), 0, 0xffff);
	  fNumericEntriesAnalogOffset[7-i]->Associate(this);
	  fNumericEntriesAnalogOffset[7-i]->MoveResize(260, 0, 70, 20);
	  fLabel[7-i] = new TGLabel(fF[i], chkChLabel[7-i], myGC(), myfont->GetFontStruct());
 	  fLabel[7-i]->MoveResize(260+85, 2, 40, 15);

   }
   offset_parameter_has_changed_flag = 1 ;

// CheckButton to enable DAC-test (automatical DAC value increment)
   fChkDacInrementTest = new TGCheckButton(fF_tab4_fGrp2, "automatical Increment Dac value Test enable", 0);
   fF_tab4_fGrp2->AddFrame(fChkDacInrementTest, new TGLayoutHints(kLHintsExpandX, 5, 5, 5, 20));
   fChkDacInrementTest->SetState(kButtonUp)   ; // is Off !
   fChkDacInrementTest->MoveResize(20, 250, 300, 15);

 
   fF_tab4_fGrp2->MapSubwindows();
//   fF_tab4_fGrp2->Resize(fF[0]->GetDefaultSize());
   fF_tab4_fGrp2->MapWindow();
   fF_tab4_fGrp2->Resize(480,300);


/*******************************************************/

   fF_tab4_fGrp3 = new TGGroupFrame(fF_tab4, "ADC SPI Settings for all channels");
   fF_tab4->AddFrame(fF_tab4_fGrp3, new TGLayoutHints(kLHintsExpandX, 5, 5, 15, 5));

   fF[0] = new TGHorizontalFrame(fF_tab4_fGrp3, 400, 30);
   fF_tab4_fGrp3->AddFrame(fF[0],  new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 0, 0, 15, 10));


   fCombo_Set_ADC_SPI_Input_Voltage = new TGComboBox(fF[0], 90);
	fCombo_Set_ADC_SPI_Input_Voltage->Resize(400, 20);
   fCombo_Set_ADC_SPI_Input_Voltage->Associate(this); // Event (IRQ) anmelden
   fF[0]->AddFrame(fCombo_Set_ADC_SPI_Input_Voltage, new TGLayoutHints(kLHintsCenterY | kLHintsLeft , 5, 5, 5, 5));
   //fF[0]->AddFrame(fCombo_Set_ADC_SPI_Input_Voltage, new TGLayoutHints(kLHintsExpandX ,  5, 2, 15, 15));

	for (i = 0; i < 3; i++) {
		fCombo_Set_ADC_SPI_Input_Voltage->AddEntry(entryADC_SPI_InputVoltage[i], i);
	}

	fCombo_Set_ADC_SPI_Input_Voltage->Select(2, kTRUE); // ADC SPI 2.0V

	for (i = 0; i < 3; i++) {
		fLabel_fCombo_Set_ADC_SPI_text[i] = new TGLabel(fF[0]," ");
		fLabel_fCombo_Set_ADC_SPI_text[i]->SetTextJustify(kTextLeft + kTextCenterX );
		fLabel_fCombo_Set_ADC_SPI_text[i]->SetMargins(0,0,0,0);
		fLabel_fCombo_Set_ADC_SPI_text[i]->SetWrapLength(-1);
		fF[0]->AddFrame(fLabel_fCombo_Set_ADC_SPI_text[i], new TGLayoutHints(kLHintsExpandX,2,2,10,2));
	}
#ifdef TEST
	if (gl_sis3316_adc1->adc_125MHz_flag == 1) {
		sprintf(s,"chip full scale 2.00V: input range 5V->5V / 2V->2V (default)");
		fLabel_fCombo_Set_ADC_SPI_text[0]->SetText(s);
		sprintf(s,"chip full scale 1.75V: input range 5V->4.375V / 2V->1.75V  ");
		fLabel_fCombo_Set_ADC_SPI_text[1]->SetText(s);
		sprintf(s,"chip full scale 1.50V:  input range 5V->3.75V / 2V->1.5V");
		fLabel_fCombo_Set_ADC_SPI_text[2]->SetText(s);
	}
	else {
		sprintf(s,"chip full scale 1.75V: input range 5V->5V / 2V->2V (default)");
		fLabel_fCombo_Set_ADC_SPI_text[0]->SetText(s);
		sprintf(s,"chip full scale 2.00V: input range 5V->5.7V / 2V->2.3V  ");
		fLabel_fCombo_Set_ADC_SPI_text[1]->SetText(s);
		sprintf(s,"chip full scale 1.50V: input range 5V->4.3V / 2V->1.7V");
		fLabel_fCombo_Set_ADC_SPI_text[2]->SetText(s);
	}
#endif


   fF_tab4_fGrp4 = new TGGroupFrame(fF_tab4, "SIS3316 Tap Delay Settings for all channels");
   fF_tab4->AddFrame(fF_tab4_fGrp4, new TGLayoutHints(kLHintsExpandX, 5, 5, 15, 5));

   fF[0] = new TGHorizontalFrame(fF_tab4_fGrp4, 200, 30);
   fF_tab4_fGrp4->AddFrame(fF[0],  new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 0, 0, 15, 10));

   fNumericEntriesTapDelay = new TGNumberEntry(fF[0], 0 /* value */, 8 /* width */, 120 /* irq */ , (TGNumberFormat::kNESHex) ) ; //kNESHex
   fNumericEntriesTapDelay->Associate(this); // Event (IRQ) anmelden
    fF[0]->AddFrame(fNumericEntriesTapDelay, new TGLayoutHints(kLHintsCenterY | kLHintsLeft , 5, 5, 5, 5 ));   // left, right, top, bottom
   fLabel[0] = new TGLabel(fF[0], "Tap Delay (hex)", myGC(), myfont->GetFontStruct());
   fF[0]->AddFrame(fLabel[0], new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 5, 5, 5, 5));
   fNumericEntriesTapDelay->SetIntNumber(0x0000); //
   fNumericEntriesTapDelay->SetLimits((TGNumberFormat::kNELLimitMinMax), 0, 0xffff);


   tf->AddFrame(fF_tab4, new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5, 5, 5));
 	

	
	
/**********************************************************************************************************/
// Tab 3  (FIR Trigger)

   tf = fTab->AddTab("Trigger");
 	tabel_tab[5] = fTab->GetTabTab("Trigger");
	tabel_tab[5]->ChangeBackground(tab_color_not_active);
	this->sis3316Test1_nof_valid_tabel_tabs++ ;	
   //fL1 = new TGLayoutHints(kLHintsTop | kLHintsLeft ,  20 ,2, 2, 12); //hints, left, right, top, bottom
   fF_tab3 = new TGCompositeFrame(tf, 60, 20, kVerticalFrame);


/*********************************************************************/
   fF_tab3_fGrp1 = new TGGroupFrame(fF_tab3, "FIR Filter Trigger Settings for all channels (internal Trigger generation)");
   fF_tab3->AddFrame(fF_tab3_fGrp1, new TGLayoutHints(kLHintsExpandX, 5, 5, 5, 5));
   fF_tab3_fGrp1->SetLayoutBroken(kTRUE);



   fNumericEntriesTriggerPulse_length = new TGNumberEntry(fF_tab3_fGrp1, 0 /* value */, 8 /* width */, 80 /* irq */ , (TGNumberFormat::kNESInteger) ) ; //kNESHex
   fNumericEntriesTriggerPulse_length->SetIntNumber(4); //
   fNumericEntriesTriggerPulse_length->SetLimits((TGNumberFormat::kNELLimitMinMax), 0, 0xfffe);
   fNumericEntriesTriggerPulse_length->MoveResize(20, 30, 80, 20);
   fLabel[0] = new TGLabel(fF_tab3_fGrp1, "Trigger Out Pulse Length", myGC(), myfont->GetFontStruct());
   fLabel[0]->SetTextJustify(kTextLeft + kTextCenterX );
   fLabel[0]->MoveResize(105, 32, 180, 15);

   fNumericEntriesTriggerGap = new TGNumberEntry(fF_tab3_fGrp1, 0 /* value */, 8 /* width */, 80 /* irq */ , (TGNumberFormat::kNESInteger) ) ; //kNESHex
   fNumericEntriesTriggerGap->SetIntNumber(8); //
   fNumericEntriesTriggerGap->SetLimits((TGNumberFormat::kNELLimitMinMax), 0, 0xfffe);
   fNumericEntriesTriggerGap->MoveResize(20, 55, 80, 20);
   fLabel[1] = new TGLabel(fF_tab3_fGrp1, "Trigger Gap", myGC(), myfont->GetFontStruct());
   fLabel[1]->SetTextJustify(kTextLeft + kTextCenterX );
   fLabel[1]->MoveResize(105, 57, 180, 15);

   fNumericEntriesTriggerPeaking = new TGNumberEntry(fF_tab3_fGrp1, 0 /* value */, 8 /* width */, 80 /* irq */ , (TGNumberFormat::kNESInteger) ) ; //kNESHex
   fNumericEntriesTriggerPeaking->SetIntNumber(4); //
   fNumericEntriesTriggerPeaking->SetLimits((TGNumberFormat::kNELLimitMinMax), 0, 0xfffe);
   fNumericEntriesTriggerPeaking->MoveResize(20, 80, 80, 20);
   fLabel[2] = new TGLabel(fF_tab3_fGrp1, "Trigger Peaking", myGC(), myfont->GetFontStruct());
   fLabel[2]->SetTextJustify(kTextLeft + kTextCenterX );
   fLabel[2]->MoveResize(105, 82, 180, 15);

   fNumericEntriesTriggerThreshold = new TGNumberEntry(fF_tab3_fGrp1, 0 /* value */, 8 /* width */, 80 /* irq */ , (TGNumberFormat::kNESInteger) ) ; //kNESHex
   fNumericEntriesTriggerThreshold->SetIntNumber(100); //
   fNumericEntriesTriggerThreshold->SetLimits((TGNumberFormat::kNELLimitMinMax), 0, 0xfffe);
   fNumericEntriesTriggerThreshold->MoveResize(20, 105, 80, 20);
   fLabel[3] = new TGLabel(fF_tab3_fGrp1, "Trigger Threshold (adc value !)", myGC(), myfont->GetFontStruct());
   fLabel[3]->SetTextJustify(kTextLeft + kTextCenterX );
   fLabel[3]->MoveResize(105, 107, 180, 15);

   fNumericEntriesHeTriggerThreshold = new TGNumberEntry(fF_tab3_fGrp1, 0 /* value */, 8 /* width */, 80 /* irq */ , (TGNumberFormat::kNESInteger) ) ; //kNESHex
   fNumericEntriesHeTriggerThreshold->SetIntNumber(0); //
   fNumericEntriesHeTriggerThreshold->SetLimits((TGNumberFormat::kNELLimitMinMax), 0, 0xfffe);
   fNumericEntriesHeTriggerThreshold->MoveResize(20, 130, 80, 20);
   fLabel[4] = new TGLabel(fF_tab3_fGrp1, "HE-Trigger Threshold (adc value !)", myGC(), myfont->GetFontStruct());
   fLabel[4]->SetTextJustify(kTextLeft + kTextCenterX );
   fLabel[4]->MoveResize(105, 132, 210, 15);

   fChkTriggerHeSuppressMode = new TGCheckButton(fF_tab3_fGrp1, "High Energy Suppress Trigger mode", 80);
   fChkTriggerHeSuppressMode->SetState(kButtonUp)   ; // is Off !
   fChkTriggerHeSuppressMode->MoveResize(20, 165, 280, 15);
   fF_tab3_fGrp1->AddFrame(fChkTriggerHeSuppressMode, new TGLayoutHints(kLHintsExpandX, 5, 5, 5, 20));

   fCombo_InternalTriggerCfdSelection = new TGComboBox(fF_tab3_fGrp1, 1);
   for (i = 0; i < 3; i++) {
		fCombo_InternalTriggerCfdSelection->AddEntry(entryInternalTriggerCfdSelection[i], i);
   }
   fCombo_InternalTriggerCfdSelection->MoveResize(20, 190, 430, 20);
   fCombo_InternalTriggerCfdSelection->Select(2, kTRUE); //  
   fF_tab3_fGrp1->AddFrame(fCombo_InternalTriggerCfdSelection, new TGLayoutHints(kLHintsCenterY | kLHintsLeft , 5, 5, 5, 5));

 
   fF_tab3_fGrp1->MapSubwindows();
   fF_tab3_fGrp1->MapWindow();
   fF_tab3_fGrp1->Resize(480,240);



/*********************************************************************/


   fF_tab3_fGrp2 = new TGGroupFrame(fF_tab3, "individual internal Trigger generation Enable ");
	//this->AddFrame(fF_tab3_fGrp1, new TGLayoutHints(kLHintsExpandX, 5, 5, 5, 5));
   fF_tab3->AddFrame(fF_tab3_fGrp2, new TGLayoutHints(kLHintsExpandX, 5, 5, 5, 5));
   fF_tab3_fGrp2->SetLayoutBroken(kTRUE);


   fTriggerEnableCh_Set = new TGTextButton(fF_tab3_fGrp2, "Enable all Channels", SIS3316TestDialog_kCM_BUTTON_IRQ_NO_30);
   fTriggerEnableCh_Clr = new TGTextButton(fF_tab3_fGrp2, "Disable all Channels", SIS3316TestDialog_kCM_BUTTON_IRQ_NO_31);
   fTriggerEnableCh_Set->ChangeBackground(yellow);
   fTriggerEnableCh_Clr->ChangeBackground(yellow);
   //fTriggerEnableCh_Set->Resize(225, 25);
   //fTriggerEnableCh_Clr->Resize(225, 25);
   fTriggerEnableCh_Set->Associate(this);
   fTriggerEnableCh_Clr->Associate(this);
   fTriggerEnableCh_Set->MoveResize(20, 30, 225, 25);
   fTriggerEnableCh_Clr->MoveResize(20, 65, 225, 25);

   for (i = 0; i < 8; i++) {
		fTGHor_tab3_2_sub[i] = new TGHorizontalFrame(fF_tab3_fGrp2, 360, 250, kHorizontalFrame);
		fF_tab3_fGrp2->AddFrame(fTGHor_tab3_2_sub[i], new TGLayoutHints(kLHintsExpandX, 5, 5, 15, 5));
		fTGHor_tab3_2_sub[i]->MoveResize(20, 110 + ((i) * 20), 225, 17);

		fChkTriggerEnableCh[15-i] = new TGCheckButton(fTGHor_tab3_2_sub[i], chkTriggerEnableChLabel[15-i], 16-i);
		fChkTriggerEnableCh[15-i]->MoveResize(0, 0, 70, 15);
		fChkTriggerEnableCh[15-i]->SetState(kButtonUp)   ; // is OFF !

		fChkTriggerEnableCh[7-i] = new TGCheckButton(fTGHor_tab3_2_sub[i], chkTriggerEnableChLabel[7-i], 16-i);
		fChkTriggerEnableCh[7-i]->MoveResize(110, 0, 70, 15); 
		fChkTriggerEnableCh[7-i]->SetState(kButtonUp)   ; // is OFF !

   }

   for (i = 16; i < 20; i++) {
		fTGHor_tab3_2_sub[i] = new TGHorizontalFrame(fF_tab3_fGrp2, 360, 250, kHorizontalFrame);
		fF_tab3_fGrp2->AddFrame(fTGHor_tab3_2_sub[i], new TGLayoutHints(kLHintsExpandX, 5, 5, 15, 5));
		fTGHor_tab3_2_sub[i]->MoveResize(20, 300 + ((19-i) * 20), 225, 17);

		fChkTriggerEnableCh[16+(19-i)] = new TGCheckButton(fTGHor_tab3_2_sub[i], chkTriggerEnableChLabel[16+(19-i)], 16+(19-i));
		fChkTriggerEnableCh[16+(19-i)]->MoveResize(0, 0, 170, 15);
		fChkTriggerEnableCh[16+(19-i)]->SetState(kButtonUp)   ; // is OFF !
   }


   fF_tab3_fGrp2->MapSubwindows();
   fF_tab3_fGrp2->MapWindow();
   fF_tab3_fGrp2->Resize(480,420);



/*********************************************************************/

   fF_tab3_fGrp3 = new TGGroupFrame(fF_tab3, "Pileup Settings for all channels");
   fF_tab3->AddFrame(fF_tab3_fGrp3, new TGLayoutHints(kLHintsExpandX, 5, 5, 5, 5));
   fF_tab3_fGrp3->SetLayoutBroken(kTRUE);

   fNumericEntriesPileup_length = new TGNumberEntry(fF_tab3_fGrp3, 0 /* value */, 8 /* width */, 80 /* irq */ , (TGNumberFormat::kNESInteger) ) ; //kNESHex
   fNumericEntriesPileup_length->SetIntNumber(0); //
   fNumericEntriesPileup_length->SetLimits((TGNumberFormat::kNELLimitMinMax), 0, 0xfffe);
   fNumericEntriesPileup_length->MoveResize(20, 30, 70, 20);

   fLabel[8] = new TGLabel(fF_tab3_fGrp3, "Pileup Length", myGC(), myfont->GetFontStruct());
   fLabel[8]->SetTextJustify(kTextLeft + kTextCenterX );
   fLabel[8]->MoveResize(100, 32, 90, 15);


   fNumericEntriesRepileup_length = new TGNumberEntry(fF_tab3_fGrp3, 0 /* value */, 8 /* width */, 80 /* irq */ , (TGNumberFormat::kNESInteger) ) ; //kNESHex
   fNumericEntriesRepileup_length->SetIntNumber(0); //
   fNumericEntriesRepileup_length->SetLimits((TGNumberFormat::kNELLimitMinMax), 0, 0xfffe);
   fNumericEntriesRepileup_length->MoveResize(20, 55, 70, 20);

   fLabel[9] = new TGLabel(fF_tab3_fGrp3, "Re-Pileup Length", myGC(), myfont->GetFontStruct());
   fLabel[9]->SetTextJustify(kTextLeft + kTextCenterX );
   fLabel[9]->MoveResize(100, 57, 90, 15);

   fF_tab3_fGrp3->MapSubwindows();
   fF_tab3_fGrp3->MapWindow();
   fF_tab3_fGrp3->Resize(480,100);


 
/*********************************************************************/

   fF_tab3_fGrp4 = new TGGroupFrame(fF_tab3, "Internal Trigger and High-Energy Trigger routing selection to the VME FPGA");
   fF_tab3->AddFrame(fF_tab3_fGrp4, new TGLayoutHints(kLHintsExpandX, 5, 5, 5, 5));
   fF_tab3_fGrp4->SetLayoutBroken(kTRUE);

   fCombo_InternalTriggerToVMESelection = new TGComboBox(fF_tab3_fGrp4, 1);
   for (i = 0; i < 3; i++) {
		fCombo_InternalTriggerToVMESelection->AddEntry(entryInternalTriggerToVMESelection[i], i);
   }
   fCombo_InternalTriggerToVMESelection->MoveResize(20, 30, 430, 20);
   fCombo_InternalTriggerToVMESelection->Select(0, kTRUE); //  
   fF_tab3_fGrp4->AddFrame(fCombo_InternalTriggerToVMESelection, new TGLayoutHints(kLHintsCenterY | kLHintsLeft , 5, 5, 5, 5));

   fCombo_InternalHeTriggerToVMESelection = new TGComboBox(fF_tab3_fGrp4, 1);
   for (i = 0; i < 2; i++) {
		fCombo_InternalHeTriggerToVMESelection->AddEntry(entryInternalHeTriggerToVMESelection[i], i);
   }
   fCombo_InternalHeTriggerToVMESelection->MoveResize(20, 60, 430, 20);
   fCombo_InternalHeTriggerToVMESelection->Select(0, kTRUE); //  
   fF_tab3_fGrp4->AddFrame(fCombo_InternalHeTriggerToVMESelection, new TGLayoutHints(kLHintsCenterY | kLHintsLeft , 5, 5, 5, 5));



 
   fF_tab3_fGrp4->MapSubwindows();
   fF_tab3_fGrp4->MapWindow();
   fF_tab3_fGrp4->Resize(480,120);

/*********************************************************************/


   tf->AddFrame(fF_tab3, new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5, 5, 5));

   
   
 /**********************************************************************************************************/



   
   // Tab 3a  (FIR Energy)
   tf = fTab->AddTab("Energy");
  	tabel_tab[6] = fTab->GetTabTab("Energy");
	tabel_tab[6]->ChangeBackground(tab_color_not_active);
	this->sis3316Test1_nof_valid_tabel_tabs++ ;	
	fF_tab3a = new TGCompositeFrame(tf, 60, 20, kVerticalFrame);

	fF_tab3a_fGrp1 = new TGGroupFrame(fF_tab3a, "FIR Filter Energy Settings for all channels");
	fF_tab3a->AddFrame(fF_tab3a_fGrp1, new TGLayoutHints(kLHintsExpandX, 5, 5, 10, 5));

	fL5 = new TGLayoutHints(kLHintsCenterY | kLHintsRight, 2, 2, 2, 2);

   fF[0] = new TGHorizontalFrame(fF_tab3a_fGrp1, 200, 30);

   fF_tab3a_fGrp1->AddFrame(fF[0], new TGLayoutHints(kLHintsExpandX, 2, 2, 12, 2));
   fNumericEntriesEnergyPeaking = new TGNumberEntry(fF[0], 50 /* value */, 8 /* width */, SIS3316TestDialog_kCM_ENTRY_IRQ_NO_90 /* irq */ , (TGNumberFormat::kNESInteger) ) ; //kNESHex
   fNumericEntriesEnergyPeaking->Associate(this);
   fF[0]->AddFrame(fNumericEntriesEnergyPeaking, fL5);
   fLabel[0] = new TGLabel(fF[0], "Energy Peaking", myGC(), myfont->GetFontStruct());
   fF[0]->AddFrame(fLabel[0], fL5);

   fF[1] = new TGHorizontalFrame(fF_tab3a_fGrp1, 200, 30);
   fF_tab3a_fGrp1->AddFrame(fF[1], fL5);
   fNumericEntriesEnergyGap = new TGNumberEntry(fF[1], 20 /* value */, 8 /* width */, SIS3316TestDialog_kCM_ENTRY_IRQ_NO_90 /* irq */ , (TGNumberFormat::kNESInteger) ) ; //kNESHex
   fNumericEntriesEnergyGap->Associate(this);
   fF[1]->AddFrame(fNumericEntriesEnergyGap, fL5);
   fLabel[1] = new TGLabel(fF[1], "Energy Gap", myGC(), myfont->GetFontStruct());
   fF[1]->AddFrame(fLabel[1], fL5);

   fF[2] = new TGHorizontalFrame(fF_tab3a_fGrp1, 200, 30);
   fF_tab3a_fGrp1->AddFrame(fF[2], fL5);
   fNumericEntriesEnergyTauTable = new TGNumberEntry(fF[2], 0 /* value */, 8 /* width */, SIS3316TestDialog_kCM_ENTRY_IRQ_NO_90 /* irq */ , (TGNumberFormat::kNESInteger) ) ; //kNESHex
   fNumericEntriesEnergyTauTable->Associate(this);
   fF[2]->AddFrame(fNumericEntriesEnergyTauTable, fL5);
   fLabel[2] = new TGLabel(fF[2], "Energy Dekay Tau table", myGC(), myfont->GetFontStruct());
   fF[2]->AddFrame(fLabel[2], fL5);

   fF[3] = new TGHorizontalFrame(fF_tab3a_fGrp1, 200, 30);
   fF_tab3a_fGrp1->AddFrame(fF[3], fL5);
   fNumericEntriesEnergyTauFactor = new TGNumberEntry(fF[3], 0 /* value */, 8 /* width */, SIS3316TestDialog_kCM_ENTRY_IRQ_NO_90 /* irq */ , (TGNumberFormat::kNESInteger) ) ; //kNESHex
   fNumericEntriesEnergyTauFactor->Associate(this);
   fF[3]->AddFrame(fNumericEntriesEnergyTauFactor, fL5);
   fLabel[3] = new TGLabel(fF[3], "Energy Dekay Tau factor", myGC(), myfont->GetFontStruct());
   fF[3]->AddFrame(fLabel[3], fL5);


   fF[4] = new TGHorizontalFrame(fF_tab3a_fGrp1, 200, 30);
   fF_tab3a_fGrp1->AddFrame(fF[4], fL5);
   fNumericEntriesEnergyAdditionalAverage = new TGNumberEntry(fF[4], 0 /* value */, 8 /* width */, SIS3316TestDialog_kCM_ENTRY_IRQ_NO_90 /* irq */ , (TGNumberFormat::kNESInteger) ) ; //kNESHex
   fNumericEntriesEnergyAdditionalAverage->Associate(this);
   fF[4]->AddFrame(fNumericEntriesEnergyAdditionalAverage, fL5);
   fLabel[4] = new TGLabel(fF[4], "Energy Additional Average factor", myGC(), myfont->GetFontStruct());
   fF[4]->AddFrame(fLabel[4], fL5);

 
   fF[5] = new TGHorizontalFrame(fF_tab3a_fGrp1, 200, 30);
   fF_tab3a_fGrp1->AddFrame(fF[5], fL5);
   fNumericEntriesEnergyPickupValueIndex = new TGNumberEntry(fF[5], 0 /* value */, 8 /* width */, SIS3316TestDialog_kCM_ENTRY_IRQ_NO_90 /* irq */ , (TGNumberFormat::kNESInteger) ) ; //kNESHex
   fNumericEntriesEnergyAdditionalAverage->Associate(this);
   fF[5]->AddFrame(fNumericEntriesEnergyPickupValueIndex, fL5);
   fLabel[5] = new TGLabel(fF[5], "Energy Pickup Index", myGC(), myfont->GetFontStruct());
   fF[5]->AddFrame(fLabel[5], fL5);

 
//*********************************

   fF_tab3a_fGrp2 = new TGGroupFrame(fF_tab3a, "Accumulator Settings for all channels");
   fF_tab3a->AddFrame(fF_tab3a_fGrp2, new TGLayoutHints(kLHintsExpandX, 5, 5, 10, 5)); // hints, left, right, top, bottom


   fL5 = new TGLayoutHints(kLHintsCenterY | kLHintsRight, 2, 2, 2, 2);
   
	for (i=0;i<8;i++) {
		fTGHorizontalFrame = new TGHorizontalFrame(fF_tab3a_fGrp2, 200, 30);
		fF_tab3a_fGrp2->AddFrame(fTGHorizontalFrame, new TGLayoutHints(kLHintsExpandX, 2, 2, 12, 2));
		fNumericEntriesAccumulatorStartIndex[i] = new TGNumberEntry(fTGHorizontalFrame, 0 /* value */, 8 /* width */, 0 /* irq */ , (TGNumberFormat::kNESInteger) ) ; //kNESHex
		//fNumericEntriesAccumulatorStartIndex[i]->Associate(this);
		fTGHorizontalFrame->AddFrame(fNumericEntriesAccumulatorStartIndex[i], fL5);
		fLabel_AccumulatorStartIndex_text[i] = new TGLabel(fTGHorizontalFrame, accuStartIndexlabel[i], myGC(), myfont->GetFontStruct());
		fTGHorizontalFrame->AddFrame(fLabel_AccumulatorStartIndex_text[i], fL5);
	 	fNumericEntriesAccumulatorStartIndex[i]->SetLimits((TGNumberFormat::kNELLimitMinMax), 0, 65535);


		fTGHorizontalFrame = new TGHorizontalFrame(fF_tab3a_fGrp2, 200, 30);
		fF_tab3a_fGrp2->AddFrame(fTGHorizontalFrame, new TGLayoutHints(kLHintsExpandX, 2, 2, 2, 2));
		fNumericEntriesAccumulatorLength[i] = new TGNumberEntry(fTGHorizontalFrame, 10 /* value */, 8 /* width */, 0 /* irq */ , (TGNumberFormat::kNESInteger) ) ; //kNESHex
		//fNumericEntriesAccumulatorLength[i]->Associate(this);
		fTGHorizontalFrame->AddFrame(fNumericEntriesAccumulatorLength[i], fL5);
		fLabel_AccumulatorLength_text[i] = new TGLabel(fTGHorizontalFrame, accuLengthlabel[i], myGC(), myfont->GetFontStruct());
		fTGHorizontalFrame->AddFrame(fLabel_AccumulatorLength_text[i], fL5);
	 	fNumericEntriesAccumulatorLength[i]->SetLimits((TGNumberFormat::kNELLimitMinMax), 1, 512);
	}



   tf->AddFrame(fF_tab3a, new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5, 5, 5));


/**********************************************************************************************************/



   // Tab 5  (Clock)
 
   tf = fTab->AddTab("Sample Clock");
 	tabel_tab[7] = fTab->GetTabTab("Sample Clock");
	tabel_tab[7]->ChangeBackground(tab_color_active);
	fTab->SetTab(7) ; // set active

	this->sis3316Test1_nof_valid_tabel_tabs++ ;	
    //fL1 = new TGLayoutHints(kLHintsTop | kLHintsLeft ,  20 ,2, 2, 12); //hints, left, right, top, bottom

	fF_tab5 = new TGCompositeFrame(tf, 60, 20, kVerticalFrame);


/*******************************/
	fF_tab5_fGrp1 = new TGGroupFrame(fF_tab5, "Internal Programmable Clock Oscillator");
	fF_tab5->AddFrame(fF_tab5_fGrp1, new TGLayoutHints(kLHintsExpandX, 5, 5, 5, 5));

	fF5E = new TGCompositeFrame(fF_tab5_fGrp1, 60, 20, kVerticalFrame);
	fF_tab5_fGrp1->AddFrame(fF5E, new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5, 5, 5));


	fCombo_SetInternalClockFreq = new TGComboBox(fF5E, SIS3316TestDialog_kCM_COMBOBOX_IRQ_NO_40);
	fCombo_SetInternalClockFreq->Associate(this); // Event (IRQ) anmelden
	fL5 = new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 5, 5);
	fF5E->AddFrame(fCombo_SetInternalClockFreq, fL5);

	for (i = 0; i < 16; i++) {
		fCombo_SetInternalClockFreq->AddEntry(entryClock_freq[i], i);
	}

	//#ifdef TEST
	#ifdef RUN_WITHOUT_HARDWARE
		fCombo_SetInternalClockFreq->Select(0, kTRUE); //
	#else
	if (gl_sis3316_adc1->adc_125MHz_flag == 1) {
			fCombo_SetInternalClockFreq->Select(6, kTRUE); //
		}
		else {
			fCombo_SetInternalClockFreq->Select(0, kTRUE); //
	}
	#endif
	fCombo_SetInternalClockFreq->Resize(270, 22);

 //***


 
/***************************************************/
   	fF_tab5_fGrp1A = new TGGroupFrame(fF_tab5, "FP-Bus Control");
	fF_tab5->AddFrame(fF_tab5_fGrp1A, new TGLayoutHints(kLHintsExpandX, 5, 5, 5, 5));

	fF4A = new TGCompositeFrame(fF_tab5_fGrp1A, 60, 20, kVerticalFrame);
	fF_tab5_fGrp1A->AddFrame(fF4A, new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5, 5, 5));

	fChkFP_BUS_ClockMaster = new TGCheckButton(fF4A, "Enable Internal Sample Clock to FP-BUS", SIS3316TestDialog_kCM_CHECKBUTTON_IRQ_NO_40);
	fChkFP_BUS_ClockMaster->Associate(this); // Event (IRQ) anmelden
	fChkFP_BUS_ClockMaster->SetState(kButtonUp)   ; // is Off !
	fF4A->AddFrame(fChkFP_BUS_ClockMaster,  new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,  2, 2, 5, 2));// hints, left, right, top, bottom


//***

	fCombo_FP_BUS_ClockOutMux = new TGComboBox(fF4A, SIS3316TestDialog_kCM_COMBOBOX_IRQ_NO_40);
	fCombo_FP_BUS_ClockOutMux->Associate(this); // Event (IRQ) anmelden
	fL5 = new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 5, 5);
	fF4A->AddFrame(fCombo_FP_BUS_ClockOutMux, fL5);

	for (i = 0; i < 2; i++) {
		fCombo_FP_BUS_ClockOutMux->AddEntry(entryFP_BUS_ClockOutMux[i], i);
	}
	fCombo_FP_BUS_ClockOutMux->Select(0, kTRUE); //  internal
	fCombo_FP_BUS_ClockOutMux->Resize(270, 22);

/**********************/

	fF_tab5_fGrp2 = new TGGroupFrame(fF_tab5, "Sample Clock");
	fF_tab5->AddFrame(fF_tab5_fGrp2, new TGLayoutHints(kLHintsExpandX, 5, 5, 5, 5));

	fF5 = new TGCompositeFrame(fF_tab5_fGrp2, 60, 20, kVerticalFrame);
	fF_tab5_fGrp2->AddFrame(fF5, new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5, 5, 5));

//***
	fCombo_SampleClock_source = new TGComboBox(fF5, SIS3316TestDialog_kCM_COMBOBOX_IRQ_NO_40);
	fCombo_SampleClock_source->Associate(this); // Event (IRQ) anmelden
	fL5 = new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 5, 15); // hints, left, right, top, bottom
	fF5->AddFrame(fCombo_SampleClock_source, fL5);

	for (i = 0; i < 4; i++) {
		fCombo_SampleClock_source->AddEntry(entryClock_source[i], i);
	}
	fCombo_SampleClock_source->Select(0, kTRUE); //
	fCombo_SampleClock_source->Resize(270, 22);

//***


	fCombo_SetClockMultiplierMode = new TGComboBox(fF5, SIS3316TestDialog_kCM_COMBOBOX_IRQ_NO_40);
	fCombo_SetClockMultiplierMode->Associate(this); // Event (IRQ) anmelden
	fL5 = new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 5, 5);
	fF5->AddFrame(fCombo_SetClockMultiplierMode, fL5);

	for (i = 0; i < 9; i++) {
		fCombo_SetClockMultiplierMode->AddEntry(entryClock_multiplier_modes[i], i);
	}
	fCombo_SetClockMultiplierMode->Select(0, kTRUE); //  Bypass
	fCombo_SetClockMultiplierMode->Resize(270, 22);

	sample_clock_configuration_valid_flag = 0 ;

	TGLabel* fLabel_ClockNotes[4];
	for (i = 0; i < 4; i++) {
		fLabel_ClockNotes[i] = new TGLabel(fF5, " ");
		fLabel_ClockNotes[i]->SetTextJustify(kTextLeft + kTextCenterX);
		fLabel_ClockNotes[i]->SetMargins(0, 0, 0, 0);
		fLabel_ClockNotes[i]->SetWrapLength(-1);
	}


	fF5->AddFrame(fLabel_ClockNotes[0], new TGLayoutHints(kLHintsExpandX, 2, 2, 8, 2));
	sprintf(s, "Note if you are not using the Internal Programmable Clock Oscillator:");
	fLabel_ClockNotes[0]->SetText(s);
	fF5->AddFrame(fLabel_ClockNotes[1], new TGLayoutHints(kLHintsExpandX, 2, 2, 2, 1));
	sprintf(s, "   Nevertheless, you must set the Programmable Clock Oscillator");
	fLabel_ClockNotes[1]->SetText(s);
	fF5->AddFrame(fLabel_ClockNotes[2], new TGLayoutHints(kLHintsExpandX, 2, 2, 1, 1));
	sprintf(s, "   to a value that is as close as possible to the Sample Clock");
	fLabel_ClockNotes[2]->SetText(s);
	fF5->AddFrame(fLabel_ClockNotes[3], new TGLayoutHints(kLHintsExpandX, 2, 2, 1, 5));
	sprintf(s, "   for the correct tap delay setting");
	fLabel_ClockNotes[3]->SetText(s);

		
		/**********************/
	// Coinicidence Lookup Tables

	fF_tab5_fGrp3 = new TGGroupFrame(fF_tab5, "Coincidence Lookup Tables");
	fF_tab5->AddFrame(fF_tab5_fGrp3, new TGLayoutHints(kLHintsExpandX, 5, 5, 25, 5));

	fF5AA = new TGCompositeFrame(fF_tab5_fGrp3, 60, 20, kVerticalFrame);
	fF_tab5_fGrp3->AddFrame(fF5AA, new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5, 5, 5));

	//***
	fCombo_CoincidenceLookupTableMode = new TGComboBox(fF5AA, SIS3316TestDialog_kCM_COMBOBOX_IRQ_NO_40);
	fCombo_CoincidenceLookupTableMode->Associate(this); // Event (IRQ) anmelden
	fL5 = new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 10, 15); // hints, left, right, top, bottom
	fF5AA->AddFrame(fCombo_CoincidenceLookupTableMode, fL5);

	for (i = 0; i < 3; i++) {
		fCombo_CoincidenceLookupTableMode->AddEntry(entryCoincidenceLookupTableMode[i], i);
	}
	fCombo_CoincidenceLookupTableMode->Select(0, kTRUE); //
	fCombo_CoincidenceLookupTableMode->Resize(270, 22);


	TGLabel* fLabel_LookupTable[14];

	for (i = 0; i < 14; i++) {
		fLabel_LookupTable[i] = new TGLabel(fF5AA, " ");
		fLabel_LookupTable[i]->SetTextJustify(kTextLeft + kTextCenterX);
		fLabel_LookupTable[i]->SetMargins(0, 0, 0, 0);
		fLabel_LookupTable[i]->SetWrapLength(-1);
	}
	fF5AA->AddFrame(fLabel_LookupTable[0], new TGLayoutHints(kLHintsExpandX, 2, 2, 5, 2));
	sprintf(s, "Example 1: use Ch 1 to Ch 10 internal triggers");
	fLabel_LookupTable[0]->SetText(s);

	fF5AA->AddFrame(fLabel_LookupTable[1], new TGLayoutHints(kLHintsExpandX, 2, 2, 2, 2));
	sprintf(s, "Coincidence Lookup Table 1 validation Pulse =");
	fLabel_LookupTable[1]->SetText(s);

	fF5AA->AddFrame(fLabel_LookupTable[2], new TGLayoutHints(kLHintsExpandX, 2, 2, 2, 2));
	sprintf(s, "   (ch 10 or ch 9) and ");
	fLabel_LookupTable[2]->SetText(s);

	fF5AA->AddFrame(fLabel_LookupTable[3], new TGLayoutHints(kLHintsExpandX, 2, 2, 2, 2));
	sprintf(s, "   (ch 8 or ch 7 or ch 6 or ch 5 or ch 4 or ch 3 or ch 2 or ch 1)");
	fLabel_LookupTable[3]->SetText(s);

	fF5AA->AddFrame(fLabel_LookupTable[4], new TGLayoutHints(kLHintsExpandX, 2, 2, 2, 2));
	sprintf(s, " ");
	fLabel_LookupTable[4]->SetText(s);

	fF5AA->AddFrame(fLabel_LookupTable[5], new TGLayoutHints(kLHintsExpandX, 2, 2, 5, 2));
	sprintf(s, "Example 2: use Ch 1 to Ch 16 internal triggers");
	fLabel_LookupTable[5]->SetText(s);

	fF5AA->AddFrame(fLabel_LookupTable[6], new TGLayoutHints(kLHintsExpandX, 2, 2, 2, 2));
	sprintf(s, "Coincidence Lookup Table 1 validation Pulse = ");
	fLabel_LookupTable[6]->SetText(s);

	fF5AA->AddFrame(fLabel_LookupTable[7], new TGLayoutHints(kLHintsExpandX, 2, 2, 2, 2));
	sprintf(s, "   Nof ch x triggers >= 2  ");
	fLabel_LookupTable[7]->SetText(s);

	fF5AA->AddFrame(fLabel_LookupTable[8], new TGLayoutHints(kLHintsExpandX, 2, 2, 2, 2));
	sprintf(s, "Coincidence Lookup Table 2 validation Pulse = ");
	fLabel_LookupTable[8]->SetText(s);

	fF5AA->AddFrame(fLabel_LookupTable[9], new TGLayoutHints(kLHintsExpandX, 2, 2, 2, 2));
	sprintf(s, "   Nof ch x triggers >= 3  ");
	fLabel_LookupTable[9]->SetText(s);


	fF5AA->AddFrame(fLabel_LookupTable[10], new TGLayoutHints(kLHintsExpandX, 2, 2, 2, 2));
	sprintf(s, " ");
	fLabel_LookupTable[10]->SetText(s);

	fF5AA->AddFrame(fLabel_LookupTable[11], new TGLayoutHints(kLHintsExpandX, 2, 2, 5, 2));
	sprintf(s, "Notes:");
	fLabel_LookupTable[11]->SetText(s);

	fF5AA->AddFrame(fLabel_LookupTable[12], new TGLayoutHints(kLHintsExpandX, 2, 2, 2, 2));
	sprintf(s, "Enable internal Trigger generaiton ");
	fLabel_LookupTable[12]->SetText(s);

	fF5AA->AddFrame(fLabel_LookupTable[13], new TGLayoutHints(kLHintsExpandX, 2, 2, 2, 2));
	sprintf(s, "Coincidence window corresponds to 'Trigger Out Pulse Length'");
	fLabel_LookupTable[13]->SetText(s);

	//******

	tf->AddFrame(fF_tab5, new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5, 5, 5));

 
/**********************************************************************************************************/

   // Tab 6  (outputs )
     tf = fTab->AddTab("NIM Outputs");
 	tabel_tab[8] = fTab->GetTabTab("NIM Outputs");
	tabel_tab[8]->ChangeBackground(tab_color_not_active);
	this->sis3316Test1_nof_valid_tabel_tabs++ ;	

	fF_tab6 = new TGCompositeFrame(tf, 60, 20, kVerticalFrame);

/**********************************************************************************/

 
	fF_tab6_fGrp1 = new TGGroupFrame(fF_tab6, "Lemo Out CO Select  (OR of selected)");
	fF_tab6->AddFrame(fF_tab6_fGrp1, new TGLayoutHints(kLHintsExpandX, 5, 5, 5, 5));
	fF_tab6_fGrp1->SetLayoutBroken(kTRUE);

	for (i = 0; i < 16; i++) {
		fTGHor_tab6_1_sub[i] = new TGHorizontalFrame(fF_tab6_fGrp1, 360, 250, kHorizontalFrame);
		fF_tab6_fGrp1->AddFrame(fTGHor_tab6_1_sub[i], new TGLayoutHints(kLHintsExpandX, 3, 3, 15, 5));
		fTGHor_tab6_1_sub[i]->MoveResize(20, 30 + ((i) * 18), 640, 17);

		fChkLemoOutCoEnableCh[31 - i] = new TGCheckButton(fTGHor_tab6_1_sub[i], chkLemoOutCoLabel[31 - i], SIS3316TestDialog_kCM_CHECKBUTTON_IRQ_NO_60);
		fChkLemoOutCoEnableCh[31 - i]->MoveResize(0, 0, 260, 15);
		fChkLemoOutCoEnableCh[31 - i]->SetState(kButtonUp); // is OFF !
		fChkLemoOutCoEnableCh[31 - i]->Associate(this);

		fChkLemoOutCoEnableCh[15 - i] = new TGCheckButton(fTGHor_tab6_1_sub[i], chkLemoOutCoLabel[15 - i], SIS3316TestDialog_kCM_CHECKBUTTON_IRQ_NO_60);
		fChkLemoOutCoEnableCh[15 - i]->MoveResize(330, 0, 260, 15);
		fChkLemoOutCoEnableCh[15 - i]->SetState(kButtonUp); // is OFF !
		fChkLemoOutCoEnableCh[15 - i]->Associate(this);
	}
	
	i = 16;
	fTGHor_tab6_1_sub[i] = new TGHorizontalFrame(fF_tab6_fGrp1, 360, 250, kHorizontalFrame);
	fF_tab6_fGrp1->AddFrame(fTGHor_tab6_1_sub[i], new TGLayoutHints(kLHintsExpandX, 5, 5, 15, 5));
	fTGHor_tab6_1_sub[i]->MoveResize(20, 30 + 10 + ((i) * 18), 640, 22);

	fNumericEntriesNimOutput[0] = new TGNumberEntry(fTGHor_tab6_1_sub[i], 0 /* value */, 8 /* width */, SIS3316TestDialog_kCM_ENTRY_IRQ_NO_60 /* irq */, (TGNumberFormat::kNESHex)); //kNESHex
	fNumericEntriesNimOutput[0]->Associate(this);
	fNumericEntriesNimOutput[0]->MoveResize(0, 0, 90, 20);
	//fNumericEntriesNimOutput[0]->SetState(kFALSE); //
																																													 
	fLabel[0] = new TGLabel(fTGHor_tab6_1_sub[i], "Lemo Out CO Select register (hex)", myGC(), myfont->GetFontStruct());
	fLabel[0]->MoveResize(85, 0, 250, 20);

	/*******/

	fF_tab6_fGrp2 = new TGGroupFrame(fF_tab6, "Lemo Out TO Select  (OR of selected)");
	fF_tab6->AddFrame(fF_tab6_fGrp2, new TGLayoutHints(kLHintsExpandX, 5, 5, 5, 5));
	fF_tab6_fGrp2->SetLayoutBroken(kTRUE);

	for (i = 0; i < 16; i++) {
		fTGHor_tab6_2_sub[i] = new TGHorizontalFrame(fF_tab6_fGrp2, 360, 250, kHorizontalFrame);
		fF_tab6_fGrp2->AddFrame(fTGHor_tab6_2_sub[i], new TGLayoutHints(kLHintsExpandX, 3, 3, 15, 5));
		fTGHor_tab6_2_sub[i]->MoveResize(20, 30 + ((i) * 18), 640, 17);

		fChkLemoOutToEnableCh[31 - i] = new TGCheckButton(fTGHor_tab6_2_sub[i], chkLemoOutToLabel[31 - i], SIS3316TestDialog_kCM_CHECKBUTTON_IRQ_NO_60);
		fChkLemoOutToEnableCh[31 - i]->MoveResize(0, 0, 300, 15);
		fChkLemoOutToEnableCh[31 - i]->SetState(kButtonUp); // is OFF !
		fChkLemoOutToEnableCh[31 - i]->Associate(this);

		fChkLemoOutToEnableCh[15 - i] = new TGCheckButton(fTGHor_tab6_2_sub[i], chkLemoOutToLabel[15 - i], SIS3316TestDialog_kCM_CHECKBUTTON_IRQ_NO_60);
		fChkLemoOutToEnableCh[15 - i]->MoveResize(330, 0, 290, 15);
		fChkLemoOutToEnableCh[15 - i]->SetState(kButtonUp); // is OFF !
		fChkLemoOutToEnableCh[15 - i]->Associate(this);
	}

	i = 16;
	fTGHor_tab6_2_sub[i] = new TGHorizontalFrame(fF_tab6_fGrp2, 360, 250, kHorizontalFrame);
	fF_tab6_fGrp2->AddFrame(fTGHor_tab6_2_sub[i], new TGLayoutHints(kLHintsExpandX, 5, 5, 15, 5));
	fTGHor_tab6_2_sub[i]->MoveResize(20, 30 + 10 + ((i) * 18), 640, 22);

	fNumericEntriesNimOutput[1] = new TGNumberEntry(fTGHor_tab6_2_sub[i], 0 /* value */, 8 /* width */, SIS3316TestDialog_kCM_ENTRY_IRQ_NO_60 /* irq */, (TGNumberFormat::kNESHex)); //kNESHex
	fNumericEntriesNimOutput[1]->Associate(this);
	fNumericEntriesNimOutput[1]->MoveResize(0, 0, 90, 20);
	//fNumericEntriesNimOutput[1]->SetState(kFALSE); //

	fLabel[1] = new TGLabel(fTGHor_tab6_2_sub[i], "Lemo Out TO Select register (hex)", myGC(), myfont->GetFontStruct());
	fLabel[1]->MoveResize(85, 0, 250, 20);

	/*******/
	/*******/

	fF_tab6_fGrp3 = new TGGroupFrame(fF_tab6, "Lemo Out UO Select  (OR of selected)");
	fF_tab6->AddFrame(fF_tab6_fGrp3, new TGLayoutHints(kLHintsExpandX, 5, 5, 5, 5));
	fF_tab6_fGrp3->SetLayoutBroken(kTRUE);

	for (i = 0; i < 16; i++) {
		fTGHor_tab6_3_sub[i] = new TGHorizontalFrame(fF_tab6_fGrp3, 360, 250, kHorizontalFrame);
		fF_tab6_fGrp3->AddFrame(fTGHor_tab6_3_sub[i], new TGLayoutHints(kLHintsExpandX, 3, 3, 15, 5));
		fTGHor_tab6_3_sub[i]->MoveResize(20, 30 + ((i) * 18), 640, 17);

		fChkLemoOutUoEnableCh[31 - i] = new TGCheckButton(fTGHor_tab6_3_sub[i], chkLemoOutUoLabel[31 - i], SIS3316TestDialog_kCM_CHECKBUTTON_IRQ_NO_60);
		fChkLemoOutUoEnableCh[31 - i]->MoveResize(0, 0, 300, 15);
		fChkLemoOutUoEnableCh[31 - i]->SetState(kButtonUp); // is OFF !
		fChkLemoOutUoEnableCh[31 - i]->Associate(this);

		fChkLemoOutUoEnableCh[15 - i] = new TGCheckButton(fTGHor_tab6_3_sub[i], chkLemoOutUoLabel[15 - i], SIS3316TestDialog_kCM_CHECKBUTTON_IRQ_NO_60);
		fChkLemoOutUoEnableCh[15 - i]->MoveResize(330, 0, 290, 15);
		fChkLemoOutUoEnableCh[15 - i]->SetState(kButtonUp); // is OFF !
		fChkLemoOutUoEnableCh[15 - i]->Associate(this);
	}

	i = 16;
	fTGHor_tab6_3_sub[i] = new TGHorizontalFrame(fF_tab6_fGrp3, 360, 250, kHorizontalFrame);
	fF_tab6_fGrp3->AddFrame(fTGHor_tab6_3_sub[i], new TGLayoutHints(kLHintsExpandX, 5, 5, 15, 5));
	fTGHor_tab6_3_sub[i]->MoveResize(20, 30 + 10 + ((i) * 18), 640, 22);

	fNumericEntriesNimOutput[2] = new TGNumberEntry(fTGHor_tab6_3_sub[i], 0 /* value */, 8 /* width */, SIS3316TestDialog_kCM_ENTRY_IRQ_NO_60 /* irq */, (TGNumberFormat::kNESHex)); //kNESHex
	fNumericEntriesNimOutput[2]->Associate(this);
	fNumericEntriesNimOutput[2]->MoveResize(0, 0, 90, 20);
	//fNumericEntriesNimOutput[2]->SetState(kFALSE); //

	fLabel[2] = new TGLabel(fTGHor_tab6_3_sub[i], "Lemo Out UO Select register (hex)", myGC(), myfont->GetFontStruct());
	fLabel[2]->MoveResize(85, 0, 250, 20);


	/*******/

	fF_tab6_fGrp1->MapSubwindows();
	fF_tab6_fGrp1->MapWindow();
	fF_tab6_fGrp1->Resize(670, 370);

	fF_tab6_fGrp2->MapSubwindows();
	fF_tab6_fGrp2->MapWindow();
	fF_tab6_fGrp2->Resize(670, 370);

	fF_tab6_fGrp3->MapSubwindows();
	fF_tab6_fGrp3->MapWindow();
	fF_tab6_fGrp3->Resize(670, 370);


	tf->AddFrame(fF_tab6, new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5, 5, 5));

/**********************************************************************************************************/



// Tab 7  (Statistic)

	tf = fTab->AddTab("Statistic");
	tabel_tab[9] = fTab->GetTabTab("Statistic");
	tabel_tab[9]->ChangeBackground(tab_color_not_active);
	this->sis3316Test1_nof_valid_tabel_tabs++;
	fF_tab7 = new TGCompositeFrame(tf, 60, 20, kVerticalFrame);


	fTextView = new TGTextView(fF_tab7, 720, 340, -1, kFixedWidth | kFixedHeight);
	fF_tab7->AddFrame(fTextView, new TGLayoutHints(kLHintsTop | kLHintsLeft, 2, 15, 5, 5));// hints, left, right, top, bottom
	fTextView->Clear();
	fTextView->ShowBottom();

	tf->AddFrame(fF_tab7, new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5, 5, 5));



/**********************************************************************************************************/

	this->SIS3316_Test_Calculate_MaxNofEventsEachBank();
	
/**********************************************************************************************************/

	fViewPortScroll->AddFrame(fTab, new TGLayoutHints(kLHintsNormal | kLHintsExpandX | kLHintsExpandY));
 
	fTab->MapSubwindows();
	fCanvasScroll->SetContainer(fTab);
	fCanvasScroll->MapSubwindows();
	this->AddFrame(fCanvasScroll, new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX | kLHintsExpandY, 2, 2, 2, 2));
	//fFrame1->AddFrame(fCanvasScroll, new TGLayoutHints(kLHintsLeft | kLHintsTop |  kLHintsExpandX | kLHintsExpandY,  2, 2, 2, 2));


	this->SetWindowName("SIS3316 Test Dialog (08-March-2024)");
	this->Move(SIS3316_TEST_WINDOW_POSTION_X, SIS3316_TEST_WINDOW_POSTION_Y);
	this->MapSubwindows();
	this->SetWMPosition(SIS3316_TEST_WINDOW_POSTION_X, SIS3316_TEST_WINDOW_POSTION_Y);
	this->Resize(SIS3316_TEST_WINDOW_WIDTH, SIS3316_TEST_WINDOW_HIGH);   // resize to default size
	this->MapWindow();

	//printf ("SIS3316_TEST_WINDOW_POSTION_X = %d \n",SIS3316_TEST_WINDOW_POSTION_X) ;

	// valid configuration file ?
	if (char_test_config_file[0] != '\0') {

		if (!params->read_parameter_file(char_test_config_file)) {
			setGuiParameters(params);
			setAdcParameters(params);
		}
	}
	this->SIS3316_Test_Update_Gui_Entries();
	this->SIS3316_Test_Calculate_MaxNofEventsEachBank();

 }

/********************************************************************************************************************************/








SIS3316TestDialog::~SIS3316TestDialog()
{
	unsigned int i;
	fSIS3316_Test1_Run_Cmd = kFALSE;
//	CloseWindow();   // this also terminates theApp


	*fBSetup = kFALSE;
	// Delete test dialog widgets.

	//delete gl_sis3316_adc1;
	if (fB_openfCanvas1WindowFlag == kTRUE) {
		for (i = 0; i < 17; i++) {
			delete fGraph_ch[i];
			//delete fGraph_Text_ch[i];
		}
		delete fCanvas1 ; //
	}

	if (fB_openfCanvas2WindowFlag == kTRUE) { // Histogram
		delete fCanvas2 ; //
	}

	for (i = 0; i < 16; i++) {
		delete iHistoAdc[i];
	}



	if (fB_openfCanvas3WindowFlag == kTRUE) { // FFT
		delete fCanvas3 ; //
	}
	if (fB_openfCanvas4WindowFlag == kTRUE) { // MAW
		delete fCanvas4 ; //
	}

	//if (fB_openfCanvas5WindowFlag == kTRUE) {
	//	delete fCanvas5; //
	//}

	fB_openfCanvas1WindowFlag = kFALSE; // Setup
	fB_openfCanvas2WindowFlag = kFALSE; // Setup
	fB_openfCanvas3WindowFlag = kFALSE; // Setup
	fB_openfCanvas4WindowFlag = kFALSE; // Setup
	//fB_openfCanvas5WindowFlag = kFALSE; // Setup

	free(dma_data_buffer);
	free(root_gr_x);
	free(root_gr_y);
	free(root_gr_maw_x);
	free(root_gr_maw_y);

	free(root_float_fft_x);
	free(root_float_fft_y);
	free(root_float_fft_y1);
	free(root_double_window_weight);
	free(root_double_fft_spectrum);
	free(root_int_save_adc_buffer);


	CloseWindow();   // this also terminates theApp


}


/**********************************************************************************************************************************/

void SIS3316TestDialog::SIS3316_Test_running_dim_widgets(bool dim_state)
{
	unsigned int i ;
	unsigned int uint_SampleControl_BankModus ;

	uint_SampleControl_BankModus = fCombo_SampleControl_BankModus->GetSelected();
	fCombo_SampleControl_BankModus->SetEnabled(dim_state); //


	fChkFP_BUS_ClockMaster->SetEnabled(dim_state); //
	fCombo_FP_BUS_ClockOutMux->SetEnabled(dim_state); //

	fCombo_SampleClock_source->SetEnabled(dim_state); //
	fCombo_SetInternalClockFreq->SetEnabled(dim_state); //
	fCombo_SetClockMultiplierMode->SetEnabled(dim_state); //
	fCombo_CoincidenceLookupTableMode->SetEnabled(dim_state); //
	
	fCombo_SetSelectMAW_TestBuffer->SetEnabled(dim_state); //

	fNumericEntries_EventHitParameter[1]->SetState(dim_state); //
	fNumericEntries_EventHitParameter[2]->SetState(dim_state); //
	fNumericEntries_EventHitParameter[4]->SetState(dim_state); //
	fNumericEntries_EventHitParameter[5]->SetState(dim_state); //
	//fNumericEntries_EventHitParameter[6]->SetState(dim_state); //
	//fNumericEntries_EventHitParameter[7]->SetState(dim_state); //

	fChk_SuppressEventsIfAddrThresFlag->SetEnabled(dim_state); //

	fChk_EventHitParameter_DataFormatBit0->SetEnabled(dim_state); //
	fChk_EventHitParameter_DataFormatBit1->SetEnabled(dim_state); //
	fChk_EventHitParameter_DataFormatBit2->SetEnabled(dim_state); //
	fChk_EventHitParameter_DataFormatBit3->SetEnabled(dim_state); //
	fChk_SaveRawDataFirstEventOnly->SetEnabled(dim_state); //

	

	fChkKeyTrigger->SetEnabled(dim_state); //
	fChkLemoInTiEnable->SetEnabled(dim_state); //
	fChkFeedbackInternalTriggerEnable->SetEnabled(dim_state); //
	fChkFeedbackCoincidence1TriggerEnable->SetEnabled(dim_state); //
	
	fChkExternalTriggerFunc->SetEnabled(dim_state); //

//************************************************************************************************

	
	// Sampling Trigger		

	
	fChkExternalTriggerDisableWithBusyEnable->SetEnabled(dim_state); //
	fChkExternalTriggerFuncAsVeto->SetEnabled(dim_state); //
	fChkLocalVetoFuncAsVeto->SetEnabled(dim_state); //
	fChkLemoInUiAsVetoEnable->SetEnabled(dim_state); //

	for (i = 0; i < 16; i++) {
		fChkIntFeedbackTriggerEnableCh[i]->SetEnabled(dim_state); //
		fChkExtTriggerEnableCh[i]->SetEnabled(dim_state); //
		fChkIntTriggerEnableCh[i]->SetEnabled(dim_state); //
		fChkIntSumTriggerEnableCh[i]->SetEnabled(dim_state); //
		fChkIntPileupTriggerEnableCh[i]->SetEnabled(dim_state); //
		fChkExtGateEnableCh[i]->SetEnabled(dim_state); //
		fChkExtVetoEnableCh[i]->SetEnabled(dim_state); //
	}
		


	fNumericEntries_VetoDelay->SetState(dim_state); //
	fNumericEntries_InternalDelay->SetState(dim_state); //

	fIntFeedbackTriggerEnableCh_Set->SetEnabled(dim_state); //
	fIntFeedbackTriggerEnableCh_Clr->SetEnabled(dim_state); //

	fExtTriggerEnableCh_Set->SetEnabled(dim_state); //
	fExtTriggerEnableCh_Clr->SetEnabled(dim_state); //

	fIntTriggerEnableCh_Set->SetEnabled(dim_state); //
	fIntTriggerEnableCh_Clr->SetEnabled(dim_state); //

	fIntSumTriggerEnableCh_Set->SetEnabled(dim_state); //
	fIntSumTriggerEnableCh_Clr->SetEnabled(dim_state); //

	fIntPileupTriggerEnableCh_Set->SetEnabled(dim_state); //
	fIntPileupTriggerEnableCh_Clr->SetEnabled(dim_state); //

	fExtGateEnableCh_Set->SetEnabled(dim_state); //
	fExtGateEnableCh_Clr->SetEnabled(dim_state); //

	fExtVetoEnableCh_Set->SetEnabled(dim_state); //
	fExtVetoEnableCh_Clr->SetEnabled(dim_state); //


	// Polarity 
	fInvertChannel_Set->SetEnabled(dim_state); //
	fInvertChannel_Clr->SetEnabled(dim_state); //
	for (i=0;i<16;i++) {
		fChkInvertChannel[i]->SetEnabled(dim_state); //
	}

	
	// Trigger FIR Filter 
	fNumericEntriesTriggerPulse_length->SetState(dim_state); //
	fNumericEntriesTriggerGap->SetState(dim_state); //
	fNumericEntriesTriggerPeaking->SetState(dim_state); //
	fNumericEntriesTriggerThreshold->SetState(dim_state); //
	fNumericEntriesHeTriggerThreshold->SetState(dim_state); //
	fChkTriggerHeSuppressMode->SetEnabled(dim_state); //
	fTriggerEnableCh_Set->SetEnabled(dim_state); //
	fTriggerEnableCh_Clr->SetEnabled(dim_state); //
	for (i=0;i<20;i++) {
		fChkTriggerEnableCh[i]->SetEnabled(dim_state); //
	}
	fCombo_InternalTriggerCfdSelection->SetEnabled(dim_state); //

	fNumericEntriesPileup_length->SetState(dim_state); //
	fNumericEntriesRepileup_length->SetState(dim_state); //
	fCombo_InternalTriggerToVMESelection->SetEnabled(dim_state); //
	fCombo_InternalHeTriggerToVMESelection->SetEnabled(dim_state); //

 

	// Energy FIR Filter 
	fNumericEntriesEnergyGap->SetState(dim_state); //
	fNumericEntriesEnergyPeaking->SetState(dim_state); //
	fNumericEntriesEnergyTauTable->SetState(dim_state); //
	fNumericEntriesEnergyTauFactor->SetState(dim_state); //
	fNumericEntriesEnergyAdditionalAverage->SetState(dim_state); //
	fNumericEntriesEnergyPickupValueIndex->SetState(dim_state); //


	gSystem->ProcessEvents();  // handle GUI events
	if (uint_SampleControl_BankModus == 0) {
		fChkNofEvents_ProBank->SetEnabled(kFALSE)   ; // dim
		fNumericEntries_SampleControl_MaxNofEvents_ProBank->SetState(kFALSE); //

		fChkWriteDataToFile->SetEnabled(kFALSE); //
		fChkWriteMultipleFiles->SetEnabled(kFALSE); //
		fTextEntryDataFilePath->SetEnabled(kFALSE); //
		fTextButtonDataFilePath->SetEnabled(kFALSE); //
	}
	else {
		fChkNofEvents_ProBank->SetEnabled(dim_state)   ; //
		fChkNofEvents_ProBank->SetState(this->root_chk_bank_event_nof_limit_on_flag ? kButtonDown : kButtonUp);

		fNumericEntries_SampleControl_MaxNofEvents_ProBank->SetState(dim_state); //
		fChkDisplayAutoZoom->SetState(kButtonUp); // is off !
		fChkDisplayAutoZoom->SetEnabled(dim_state); // 
		fChkDisplayDisableDeleteGraph->SetState(kButtonUp); // is off !
		fChkDisplayDisableDeleteGraph->SetEnabled(dim_state); // 
	}
	

	if (uint_SampleControl_BankModus != 0) {

		if (dim_state == kFALSE) {
			fChkWriteDataToFile->SetEnabled(kFALSE); //
			fChkWriteMultipleFiles->SetEnabled(kFALSE); //
			fTextEntryDataFilePath->SetEnabled(kFALSE); //
			fTextButtonDataFilePath->SetEnabled(kFALSE); //
		}
		else {
			fChkWriteDataToFile->SetEnabled(kTRUE); //
			gSystem->ProcessEvents();  // handle GUI events
			if (fChkWriteDataToFile->IsOn() == kTRUE)  {
				fChkWriteMultipleFiles->SetEnabled(kTRUE); //
				fTextEntryDataFilePath->SetEnabled(kTRUE); //
				fTextButtonDataFilePath->SetEnabled(kTRUE); //
			}
			else {
				fChkWriteMultipleFiles->SetEnabled(kFALSE); //
				fTextEntryDataFilePath->SetEnabled(kFALSE); //
				fTextButtonDataFilePath->SetEnabled(kFALSE); //
			}
		}

		fNumericEntries_EventHitParameter[0]->SetState(dim_state); //
		fNumericEntries_EventHitParameter[3]->SetState(dim_state); //

		
		fCombo_Display_MAW->Select(0, kTRUE); // no display ch
		fCombo_Display_MAW->SetEnabled(dim_state);

		fCombo_Display_FFT_Ch->Select(0, kTRUE); // no display ch
		fCombo_Display_FFT_Ch->SetEnabled(dim_state);


		// Termination 
		fTerminationChannel_Set->SetEnabled(dim_state); //
		fTerminationChannel_Clr->SetEnabled(dim_state); //
		for (i = 0; i < 16; i++) {
			fChkTerminationChannel[i]->SetEnabled(dim_state); //
		}

		// Gain/Range
		fInputRange0Channel_Set->SetEnabled(dim_state); //
		fInputRange0Channel_Clr->SetEnabled(dim_state); //
		for (i = 0; i < 16; i++) {
			fChkInputRange0Channel[i]->SetEnabled(dim_state); //
		}

 
  		// Offset
		for (i = 0; i < 16; i++) {
			fNumericEntriesAnalogOffset[i]->SetState(dim_state); //
		}
		fChkDacInrementTest->SetEnabled(dim_state); //
		fCombo_Set_ADC_SPI_Input_Voltage->SetEnabled(dim_state);
		fNumericEntriesTapDelay->SetState(dim_state); //

	}

}


  

/**********************************************************************************************************************************/
#define MAW_GRAPH
#define FFT_GRAPH

#define HISTOGRAM

void SIS3316TestDialog::SIS3316_Test1()
{
	volatile unsigned int return_code ;
	unsigned int i_ch;

	unsigned int req_nof_32bit_words, got_nof_32bit_words;
	unsigned int  ch_data_valid[16];

	unsigned int ui, plot_length;
	int ymin, ymax;
	int xmin, xmax;
	int ywidth, y_delta ;
	unsigned int i, i_adc_fpga;
	unsigned int i_accu;

	unsigned int uint_soft_trigger_flag ;
	unsigned int sample_length, sample_start_index ;
	unsigned int pre_trigger_delay = 1, old_pre_trigger_delay = 1  ;
	unsigned int trigger_gate_window_length ;


	unsigned int uint_save_raw_data_first_event_only_flag;
	unsigned int nof_events_pro_bank;
	unsigned int event_length;
	unsigned int short_event_length;
	unsigned int header_length;

	unsigned int maw_test_buffer_length ;
	unsigned int maw_test_buffer_start_index ;
	unsigned int maw_test_buffer_delay, old_maw_test_buffer_delay ;

	unsigned int header_v0100_accu_4_values_enable_flag ;
	unsigned int header_v0200_accu_5_values_enable_flag ;
	unsigned int header_accu_6_values_enable_flag ;
	unsigned int header_accu_2_values_enable_flag ;
	unsigned int header_maw_3_values_enable_flag ;
	unsigned int header_energy_filter_values_enable_flag ;

	unsigned int maw_test_buffer_enable_flag ;
	unsigned int maw_test_buffer_energy_mux_flag ;
	unsigned int uint_trigger_maw_offset ;
	unsigned int* uint_maw_ptr;

	unsigned int header_maw_3_values_offset ;
	unsigned int header_accu_6_values_offset ;
	unsigned int header_accu_2_values_offset ;
	unsigned int header_energy_values_offset ;

	unsigned int header_offset_ushort_ptr ;
	unsigned int address_threshold ;

	unsigned int trigger_pulse_length_val ;
	unsigned int trigger_gap_val ;
	unsigned int trigger_peaking_val ;
	unsigned int trigger_threshold_adc_val , trigger_HE_threshold_adc_val;
	unsigned int trigger_threshold_val, trigger_HE_threshold_val ;

	unsigned int trigger_HE_suppress_mode_flag ;
	unsigned int trigger_CFD_enable_flag ;
	unsigned int trigger_enable_flags[20] ;

	unsigned int internalTriggerToVmeFPGAselection ;
	unsigned int internalHeTriggerToVmeFPGAselection ;

	unsigned int pileup_length_val ;
	unsigned int re_pileup_length_val ;

	unsigned int energy_peaking_val ;
	unsigned int energy_gap_val ;
	unsigned int energy_decay_tau_table_val ;
	unsigned int energy_decay_tau_factor_val ;
	unsigned int energy_additional_average_val ;
	unsigned int energy_pickup_index ;

	unsigned int accumulator_start_index[8] ;
	unsigned int accumulator_length[8] ;

	unsigned int display_histo_counter, display_histo_ch_no ;
	unsigned int data ;
	unsigned int fpga_data[4] ;
	unsigned short* ushort_adc_buffer_ptr; //

	unsigned int display_histogram_choice, new_display_histogram_choice;
	unsigned int histogram_gausfit_enable_flag, histogram_gausfit_clear_flag;

	unsigned int display_MAW_choice;
	unsigned int display_FFT_choice;
	unsigned int display_FFT_Window_choice;

	unsigned int fft_plot_length  ;
	double double_fft_frequency  ;

	double double_histo_min_x ;
	double double_histo_max_x;
	double double_histo_mean ;
	double double_histo_min_mean[16] ;
	double double_histo_max_mean[16] ;
	char char_temp[80] ;


	unsigned int plot_counter;
	unsigned int bank_buffer_counter;
	unsigned int bank1_armed_flag;
	unsigned int bank2_read_flag;
	unsigned int poll_counter;
	
	unsigned int uint_WritenData_to_File_32bit_words = 0 ;
	unsigned int uint_WriteData_to_File_EnableFlag = 0 ;
	unsigned int uint_WriteData_to_MultipleFiles_Flag = 0 ;
	unsigned int uint_WriteData_to_File_OpenFlag = 0 ;
	unsigned int uint_WriteData_to_File_counter = 0 ;
	unsigned file_header_indentifier;
	unsigned int file_header_short_and_maw_length = 0;
	//unsigned int file_header_reserved = 0;
	unsigned int file_header_reserved_EventBufferLen = 0;



	char char_WriteData_to_File_initialize_filename[512]  ;
	char char_WriteData_to_File_filename[512]  ;

	FILE *file_WriteData_to_File_Pointer           ;

  	unsigned int uint_SampleControl_BankModus ;
	unsigned int possibe_nof_events_pro_bank;

//	unsigned int half_possibe_events_pro_bank;
	unsigned int gui_max_events_pro_bank ;
	file_WriteData_to_File_Pointer = NULL          ;
	
	struct timeval time_start, time_actual;
	double double_run_time_sec;
	unsigned int uint_run_time_sec;

	unsigned int uint_statistic_buffer[24];
	unsigned int i_fpga;

	uint_SampleControl_BankModus = fCombo_SampleControl_BankModus->GetSelected();

	// Parameter Setup
	pre_trigger_delay = fNumericEntries_EventHitParameter[0]->GetIntNumber();
	if (pre_trigger_delay > MAX_PRETRIGGER_DELAY) { pre_trigger_delay = MAX_PRETRIGGER_DELAY ; }
	fNumericEntries_EventHitParameter[0]->SetIntNumber(pre_trigger_delay );
	old_pre_trigger_delay = pre_trigger_delay ;

	sample_start_index = fNumericEntries_EventHitParameter[1]->GetIntNumber();
	if (sample_start_index > MAX_SAMPLE_LENGTH) { sample_start_index = MAX_SAMPLE_LENGTH - 2; }
	fNumericEntries_EventHitParameter[1]->SetIntNumber(sample_start_index );

	sample_length = fNumericEntries_EventHitParameter[2]->GetIntNumber();
	if (sample_length > MAX_SAMPLE_LENGTH) { sample_length = MAX_SAMPLE_LENGTH - 2; }
	fNumericEntries_EventHitParameter[2]->SetIntNumber(sample_length );
	this->raw_sample_length = fNumericEntries_EventHitParameter[2]->GetIntNumber();


	pileup_length_val = fNumericEntriesPileup_length->GetIntNumber();
	if (pileup_length_val > 0xffff) { pileup_length_val = 0xffff; }
	fNumericEntriesPileup_length->SetIntNumber(pileup_length_val );

	re_pileup_length_val = fNumericEntriesRepileup_length->GetIntNumber();
	if (re_pileup_length_val > 0xffff) { re_pileup_length_val = 0xffff; }
	fNumericEntriesRepileup_length->SetIntNumber(re_pileup_length_val );


	//maw_test_buffer_delay
	maw_test_buffer_delay = fNumericEntries_EventHitParameter[3]->GetIntNumber();
	maw_test_buffer_delay = maw_test_buffer_delay & 0xfffe; // only even number
	if (maw_test_buffer_delay > 1022) { maw_test_buffer_delay = 1024 - 2; }
	fNumericEntries_EventHitParameter[3]->SetIntNumber(maw_test_buffer_delay );
	old_maw_test_buffer_delay = maw_test_buffer_delay ;

	//maw_test_buffer_start_index
	maw_test_buffer_start_index = fNumericEntries_EventHitParameter[4]->GetIntNumber();
	maw_test_buffer_start_index = maw_test_buffer_start_index & 0xfffe; // only even number
	if ((gl_sis3316_adc1->adc_fpga_version & 0xff) < 0x0A) { // up to version 0x0009
		 maw_test_buffer_start_index = 0;  
	}
	else {
		if (maw_test_buffer_start_index > 32768) { maw_test_buffer_start_index = 32768; }
	}
	fNumericEntries_EventHitParameter[4]->SetIntNumber(maw_test_buffer_start_index );
	
	
	//maw_test_buffer_length
	maw_test_buffer_length = fNumericEntries_EventHitParameter[5]->GetIntNumber();
	maw_test_buffer_length = maw_test_buffer_length & 0xfffe; // only even number
	if ((gl_sis3316_adc1->adc_fpga_version & 0xff) < 0x0A) { // up to version 0x0009
		if (maw_test_buffer_length > 1022) { maw_test_buffer_length = 1024 - 2; }
	}
	else {
		if (maw_test_buffer_length > 2048) { maw_test_buffer_length = 2048 ; }
	}
	fNumericEntries_EventHitParameter[5]->SetIntNumber(maw_test_buffer_length );

	maw_test_buffer_energy_mux_flag = 0 ;
	uint_trigger_maw_offset         = 0x08000000 ;
	if (fCombo_SetSelectMAW_TestBuffer->GetSelected() == 1) {
		maw_test_buffer_energy_mux_flag = 1  ;
		uint_trigger_maw_offset         = 0x0 ;
	}

	trigger_gap_val =  fNumericEntriesTriggerGap->GetIntNumber(); ;
	if (trigger_gap_val > 510) { trigger_gap_val = 510; }
	if (trigger_gap_val < 2)   { trigger_gap_val = 2; }   // lt 2 ?
	trigger_gap_val = trigger_gap_val & 0x3fe ;
	fNumericEntriesTriggerGap->SetIntNumber(trigger_gap_val );


	trigger_pulse_length_val =  fNumericEntriesTriggerPulse_length->GetIntNumber(); ;
	if (trigger_pulse_length_val > 255) { trigger_pulse_length_val = 255; }
	fNumericEntriesTriggerPulse_length->SetIntNumber(trigger_pulse_length_val );

	trigger_gap_val =  fNumericEntriesTriggerGap->GetIntNumber(); ;
	if (trigger_gap_val > 510) { trigger_gap_val = 510; }
	if (trigger_gap_val < 2)   { trigger_gap_val = 2; }   // lt 2 ?
	trigger_gap_val = trigger_gap_val & 0x3fe ;
	fNumericEntriesTriggerGap->SetIntNumber(trigger_gap_val );

	trigger_peaking_val =  fNumericEntriesTriggerPeaking->GetIntNumber(); ;
	if (trigger_peaking_val > 510) { trigger_peaking_val = 510; } // gt 510 ?
	if (trigger_peaking_val < 2)   { trigger_peaking_val = 2; }   // lt 2 ?
	trigger_peaking_val = trigger_peaking_val & 0x3fe ;
	fNumericEntriesTriggerPeaking->SetIntNumber(trigger_peaking_val );


	trigger_threshold_adc_val =  fNumericEntriesTriggerThreshold->GetIntNumber(); ;
	if (trigger_threshold_adc_val > 0xffff) { trigger_threshold_adc_val = 0xffff; }
	fNumericEntriesTriggerThreshold->SetIntNumber(trigger_threshold_adc_val );

	
	trigger_HE_threshold_adc_val =  fNumericEntriesHeTriggerThreshold->GetIntNumber(); ;
	if (trigger_HE_threshold_adc_val > 0xffff) { trigger_HE_threshold_adc_val = 0xffff; }
	fNumericEntriesHeTriggerThreshold->SetIntNumber(trigger_HE_threshold_adc_val );

	internalTriggerToVmeFPGAselection = fCombo_InternalTriggerToVMESelection->GetSelected();
	if(internalTriggerToVmeFPGAselection > 2) { internalTriggerToVmeFPGAselection = 2; }
	fCombo_InternalTriggerToVMESelection->Select(internalTriggerToVmeFPGAselection, kTRUE); //  

	internalHeTriggerToVmeFPGAselection = fCombo_InternalHeTriggerToVMESelection->GetSelected();
	if(internalHeTriggerToVmeFPGAselection > 1) { internalHeTriggerToVmeFPGAselection = 1; }
	fCombo_InternalHeTriggerToVMESelection->Select(internalHeTriggerToVmeFPGAselection, kTRUE); //  

	trigger_HE_suppress_mode_flag = 0 ;
	if (fChkTriggerHeSuppressMode->IsOn() == kTRUE) {
		trigger_HE_suppress_mode_flag = 1 ;
	}

	trigger_CFD_enable_flag = fCombo_InternalTriggerCfdSelection->GetSelected();
	if(trigger_CFD_enable_flag > 2) { trigger_CFD_enable_flag = 2; }
	fCombo_InternalTriggerCfdSelection->Select(trigger_CFD_enable_flag, kTRUE); //  


	for (i=0;i<20;i++) {
		trigger_enable_flags[i] = 0 ;
		if (fChkTriggerEnableCh[i]->IsOn() == kTRUE) {
			trigger_enable_flags[i] = 1 ;
		}
	}


/*** FIR Filter Energy ************/

    energy_peaking_val =  fNumericEntriesEnergyPeaking->GetIntNumber(); ;
	if (energy_peaking_val > FIR_ENERGY_MAX_PEAKING) { energy_peaking_val = FIR_ENERGY_MAX_PEAKING; } //
	if (energy_peaking_val < FIR_ENERGY_MIN_PEAKING)   { energy_peaking_val = FIR_ENERGY_MIN_PEAKING; }   //
	fNumericEntriesEnergyPeaking->SetIntNumber(energy_peaking_val );

    energy_gap_val =  fNumericEntriesEnergyGap->GetIntNumber(); ;
	if (energy_gap_val > FIR_ENERGY_MAX_GAP) { energy_gap_val = FIR_ENERGY_MAX_GAP; } //
	if (energy_gap_val < FIR_ENERGY_MIN_GAP)   { energy_gap_val = FIR_ENERGY_MIN_GAP; }   //
	fNumericEntriesEnergyGap->SetIntNumber(energy_gap_val );

    energy_decay_tau_table_val =  fNumericEntriesEnergyTauTable->GetIntNumber(); ;
	if (energy_decay_tau_table_val > FIR_ENERGY_MAX_TAU_TABLE) { energy_decay_tau_table_val = FIR_ENERGY_MAX_TAU_TABLE; } //
	fNumericEntriesEnergyTauTable->SetIntNumber(energy_decay_tau_table_val );

    energy_decay_tau_factor_val =  fNumericEntriesEnergyTauFactor->GetIntNumber(); ;
	if (energy_decay_tau_factor_val > FIR_ENERGY_MAX_TAU_FACTOR) { energy_decay_tau_factor_val = FIR_ENERGY_MAX_TAU_FACTOR; } //
	fNumericEntriesEnergyTauFactor->SetIntNumber(energy_decay_tau_factor_val );

    energy_additional_average_val =  fNumericEntriesEnergyAdditionalAverage->GetIntNumber(); ;
	if (energy_additional_average_val > FIR_ENERGY_MAX_ADD_AVERAGE) { energy_additional_average_val = FIR_ENERGY_MAX_ADD_AVERAGE; } //
	fNumericEntriesEnergyAdditionalAverage->SetIntNumber(energy_additional_average_val );

    energy_pickup_index =  fNumericEntriesEnergyPickupValueIndex->GetIntNumber(); ;
	if (energy_pickup_index > 16392 + 4500) { energy_pickup_index = 0; } // max of (2*p) + G) + PretriggerDelay then set to 0 -> take max value
	fNumericEntriesEnergyPickupValueIndex->SetIntNumber(energy_pickup_index );


/*** Accumulator parameters ************/

	for (i=0;i<8;i++) {
	    accumulator_start_index[i] =  fNumericEntriesAccumulatorStartIndex[i]->GetIntNumber(); ;
		if (accumulator_start_index[i] > ACCUMULATOR_MAX_START_INDEX) { accumulator_start_index[i] = ACCUMULATOR_MAX_START_INDEX; } //
		fNumericEntriesAccumulatorStartIndex[i]->SetIntNumber(accumulator_start_index[i] );

		accumulator_length[i] =  fNumericEntriesAccumulatorLength[i]->GetIntNumber(); ;
		if (accumulator_length[i] > ACCUMULATOR_MAX_LENGTH+1) { accumulator_length[i] = ACCUMULATOR_MAX_START_INDEX+1; } //
		if (accumulator_length[i] == 0) { accumulator_length[i] = 1; } //
		fNumericEntriesAccumulatorLength[i]->SetIntNumber(accumulator_length[i] );
	}

	display_histo_counter = 0 ;
	display_histo_ch_no   = 0 ;

	plot_length = sample_length;
	if (plot_length > MAX_ROOT_PLOT_LENGTH) {
		plot_length = MAX_ROOT_PLOT_LENGTH;
	}
/**************************************************************************************************************/
 
  // Raw Data Graph Setup
	if (fB_openfCanvas1WindowFlag == kFALSE) {
	  if (uint_SampleControl_BankModus == 0) {
		 fCanvas1 = new TCanvas("fCanvas1", "ADC Raw data ", SIS3316_RAW_DATA_WINDOW_POSTION_X_SINGLE, SIS3316_RAW_DATA_WINDOW_POSTION_Y_SINGLE, SIS3316_RAW_DATA_WINDOW_WIDTH_SINGLE, SIS3316_RAW_DATA_WINDOW_HIGH_SINGLE); // X, y, WITDH, High
	  }
	  else {
		 fCanvas1 = new TCanvas("fCanvas1", "ADC Raw data ", SIS3316_RAW_DATA_WINDOW_POSTION_X_MULTI, SIS3316_RAW_DATA_WINDOW_POSTION_Y_MULTI, SIS3316_RAW_DATA_WINDOW_WIDTH_MULTI, SIS3316_RAW_DATA_WINDOW_HIGH_MULTI); // X, y, WITDH, High
	  }
	  fB_openfCanvas1WindowFlag = kTRUE; //
	}

	fCanvas1->Clear();
    fCanvas1->cd();
    fCanvas1->SetGrid();
	fCanvas1->SetFillColor(DefineCanvasBackgroundColor);
	//(fCanvas1->GetCanvasImp())->DontClose();


/**************************************************************************************************************/

#ifdef HISTOGRAM

	histogram_gausfit_clear_flag = 0;
	histogram_gausfit_enable_flag = 0;

	for (i=0;i<16;i++) {
		gPad->SetFillColor(DefineCanvasBackgroundColor);
		iHistoAdc[i]->Reset(); //
		iHistoAdc[i]->BufferEmpty(1); // action =  1 histogram is filled and buffer is deleted
		//iHistoAdc[i]->BufferEmpty(0); // action =  0 histogram is filled from the buffer
		//iHistoAdc[i]->BufferEmpty(-1); // action =  -1 histogram is reset and refilled from the buffer (called by THistPainter::Paint)
		//iHistoAdc[i]->Draw();
		iHistoAdc[i]->SetFillColor(kRed);
	}

	for (i=0;i<16;i++) {
		double_histo_min_mean[i] = (double) this->root_histo_xmax_absolute;
		double_histo_max_mean[i] = 0.0 ;
	}

#endif

/**************************************************************************************************************/




	if (gl_sis3316_adc1->adc_125MHz_flag == 0) {
		// 250  MHz
		file_header_indentifier      =  0  ;
	}
	else {
		// 125 MHz
		file_header_indentifier      =  1  ;
	}

	return_code = gl_sis3316_adc1->register_read( SIS3316_MODID, &data);
	if(return_code != 0) {
		printf("register_read: data = 0x%08x     return_code = 0x%08x\n", data, return_code);
		SIS3316_Test_running_dim_widgets(kTRUE);
		fSIS3316_Test1_Run_Cmd = kFALSE;
		return;
	}


	// SIS3316 Configuration

	// Channel Header ID register (is already done with  "new gl_sis3316_adc1" )
	data = this->uint_test_vme_base_addr & 0xFF000000 ;
	gl_sis3316_adc1->write_channel_header_ID( data) ;
	
	// adc DAC offset setup
	SIS3316_Test_Write_DacOffset() ;
	// Gain/Termination
	SIS3316_Test_Write_TerminationGain() ;

	// Set NIM_Output_Selection
	SIS3316_Test_Write_NIM_Output_Selection() ;

	// pre_trigger_delay
	return_code = gl_sis3316_adc1->register_write( SIS3316_ADC_CH1_4_PRE_TRIGGER_DELAY_REG, pre_trigger_delay ); //
	return_code = gl_sis3316_adc1->register_write( SIS3316_ADC_CH5_8_PRE_TRIGGER_DELAY_REG, pre_trigger_delay ); //
	return_code = gl_sis3316_adc1->register_write( SIS3316_ADC_CH9_12_PRE_TRIGGER_DELAY_REG, pre_trigger_delay ); //
	return_code = gl_sis3316_adc1->register_write( SIS3316_ADC_CH13_16_PRE_TRIGGER_DELAY_REG, pre_trigger_delay ); //

	// sample_length and sample_start_index
	if (sample_length > 0x10000 -2) { // max 64k -2 
		if(sample_length > MAX_SAMPLE_LENGTH) {sample_length = MAX_SAMPLE_LENGTH;}
		return_code = gl_sis3316_adc1->register_write( SIS3316_ADC_CH1_4_RAW_DATA_BUFFER_CONFIG_REG, ((sample_length & 0xffff) << 16) + (sample_start_index & 0xffff) ); // Sample Length
		return_code = gl_sis3316_adc1->register_write( SIS3316_ADC_CH5_8_RAW_DATA_BUFFER_CONFIG_REG, ((sample_length & 0xffff) << 16) + (sample_start_index & 0xffff) ); // Sample Length
		return_code = gl_sis3316_adc1->register_write( SIS3316_ADC_CH9_12_RAW_DATA_BUFFER_CONFIG_REG, ((sample_length & 0xffff) << 16) + (sample_start_index & 0xffff) ); // Sample Length
		return_code = gl_sis3316_adc1->register_write( SIS3316_ADC_CH13_16_RAW_DATA_BUFFER_CONFIG_REG, ((sample_length & 0xffff) << 16) + (sample_start_index & 0xffff) ); // Sample Length

		return_code = gl_sis3316_adc1->register_write( SIS3316_ADC_CH1_4_EXTENDED_RAW_DATA_BUFFER_CONFIG_REG, sample_length ); // Sample Length
		return_code = gl_sis3316_adc1->register_write( SIS3316_ADC_CH5_8_EXTENDED_RAW_DATA_BUFFER_CONFIG_REG, sample_length ); // Sample Length
		return_code = gl_sis3316_adc1->register_write( SIS3316_ADC_CH9_12_EXTENDED_RAW_DATA_BUFFER_CONFIG_REG, sample_length ); // Sample Length
		return_code = gl_sis3316_adc1->register_write( SIS3316_ADC_CH13_16_EXTENDED_RAW_DATA_BUFFER_CONFIG_REG, sample_length ); // Sample Length
	}
	else {
		return_code = gl_sis3316_adc1->register_write( SIS3316_ADC_CH1_4_RAW_DATA_BUFFER_CONFIG_REG, ((sample_length & 0xffff) << 16) + (sample_start_index & 0xffff) ); // Sample Length
		return_code = gl_sis3316_adc1->register_write( SIS3316_ADC_CH5_8_RAW_DATA_BUFFER_CONFIG_REG, ((sample_length & 0xffff) << 16) + (sample_start_index & 0xffff) ); // Sample Length
		return_code = gl_sis3316_adc1->register_write( SIS3316_ADC_CH9_12_RAW_DATA_BUFFER_CONFIG_REG, ((sample_length & 0xffff) << 16) + (sample_start_index & 0xffff) ); // Sample Length
		return_code = gl_sis3316_adc1->register_write( SIS3316_ADC_CH13_16_RAW_DATA_BUFFER_CONFIG_REG, ((sample_length & 0xffff) << 16) + (sample_start_index & 0xffff) ); // Sample Length

		return_code = gl_sis3316_adc1->register_write( SIS3316_ADC_CH1_4_EXTENDED_RAW_DATA_BUFFER_CONFIG_REG, 0 ); // take sample length from SIS3316_ADC_CH1_4_RAW_DATA_BUFFER_CONFIG_REG register
		return_code = gl_sis3316_adc1->register_write( SIS3316_ADC_CH5_8_EXTENDED_RAW_DATA_BUFFER_CONFIG_REG, 0 ); //  
		return_code = gl_sis3316_adc1->register_write( SIS3316_ADC_CH9_12_EXTENDED_RAW_DATA_BUFFER_CONFIG_REG, 0 ); //  
		return_code = gl_sis3316_adc1->register_write( SIS3316_ADC_CH13_16_EXTENDED_RAW_DATA_BUFFER_CONFIG_REG, 0 ); //  
	}


	// pileup configuration
	return_code = gl_sis3316_adc1->register_write( SIS3316_ADC_CH1_4_PILEUP_CONFIG_REG, ((re_pileup_length_val & 0xffff) << 16) + (pileup_length_val & 0xffff) ); //  
	return_code = gl_sis3316_adc1->register_write( SIS3316_ADC_CH5_8_PILEUP_CONFIG_REG, ((re_pileup_length_val & 0xffff) << 16) + (pileup_length_val & 0xffff) ); //  
	return_code = gl_sis3316_adc1->register_write( SIS3316_ADC_CH9_12_PILEUP_CONFIG_REG, ((re_pileup_length_val & 0xffff) << 16) + (pileup_length_val & 0xffff) ); // 
	return_code = gl_sis3316_adc1->register_write( SIS3316_ADC_CH13_16_PILEUP_CONFIG_REG, ((re_pileup_length_val & 0xffff) << 16) + (pileup_length_val & 0xffff) ); //  




	maw_test_buffer_enable_flag      = 0 ;
	if (maw_test_buffer_length > 0) {
		maw_test_buffer_enable_flag      = 1 ;
	}

	if ((this->adc_fpga_firmware_version & 0x0f00) == 0x0200) { // Neutron/Gamma
		header_v0100_accu_4_values_enable_flag = 0 ;
		header_v0200_accu_5_values_enable_flag = 1 ;
		header_accu_6_values_enable_flag = 0 ;
		header_accu_2_values_enable_flag = 0 ;
		header_maw_3_values_enable_flag  = 0;
		header_energy_filter_values_enable_flag  = 0 ;

	// data format start values
		header_length = 3;                      
		header_accu_6_values_offset = 2 ;
		header_accu_2_values_offset = 2 ;
		header_maw_3_values_offset  = 2 ;
		header_energy_values_offset = 2 ;		//  
	}
	else { //  

		if ((this->adc_fpga_firmware_version & 0x0f00) == 0x0100) { // 
			header_v0100_accu_4_values_enable_flag = 1 ;
			header_v0200_accu_5_values_enable_flag = 0 ;
			header_accu_6_values_enable_flag = 0 ;
			header_accu_2_values_enable_flag = 0 ;
			header_maw_3_values_enable_flag  = 0;
			header_energy_filter_values_enable_flag  = 0 ;

		// data format start values
			header_length = 2;                      
			header_accu_6_values_offset = 2 ;
			header_accu_2_values_offset = 2 ;
			header_maw_3_values_offset  = 2 ;
			header_energy_values_offset = 2 ;		//  
		}
		else { // standard version
			header_v0100_accu_4_values_enable_flag = 0 ;
			header_v0200_accu_5_values_enable_flag = 0 ;
			header_accu_6_values_enable_flag = 0 ;
			header_accu_2_values_enable_flag = 0 ;
			header_maw_3_values_enable_flag  = 0;
			header_energy_filter_values_enable_flag  = 0 ;

			if (fChk_EventHitParameter_DataFormatBit0->IsOn() == kTRUE) {
				header_accu_6_values_enable_flag = 1;
			}

			if (fChk_EventHitParameter_DataFormatBit1->IsOn() == kTRUE) {
				header_accu_2_values_enable_flag = 1;
			}


			if (fChk_EventHitParameter_DataFormatBit2->IsOn() == kTRUE) {
				header_maw_3_values_enable_flag = 1;
			}

			if (fChk_EventHitParameter_DataFormatBit3->IsOn() == kTRUE) {
				header_energy_filter_values_enable_flag = 1;
			}
		// data format
			header_length = 3;
			header_accu_6_values_offset = 2 ;
			header_accu_2_values_offset = 2 ;
			header_maw_3_values_offset  = 2 ;
			header_energy_values_offset = 2 ;		//  
		}
	}

	data = 0 ;
	if (header_v0200_accu_5_values_enable_flag == 1) {
		header_length = header_length + 4 ;
		header_maw_3_values_offset  = header_maw_3_values_offset + 4 ;
		header_accu_2_values_offset  = header_accu_2_values_offset + 4 ;
		//data = data + 0x0 ; // 
	}

	if (header_v0100_accu_4_values_enable_flag == 1) {
		header_length = header_length + 4 ;
		header_maw_3_values_offset  = header_maw_3_values_offset + 4 ;
		header_accu_2_values_offset  = header_accu_2_values_offset + 4 ;
		data = data + 0x1 ; // set bit 0
	}

	if (header_accu_6_values_enable_flag == 1) {
		header_length = header_length + 7 ;
		header_maw_3_values_offset  = header_maw_3_values_offset + 7 ;
		header_accu_2_values_offset  = header_accu_2_values_offset + 7 ;
		header_energy_values_offset  = header_energy_values_offset + 7 ;
		data = data + 0x1 ; // set bit 0
	}
	if (header_accu_2_values_enable_flag == 1) {
		header_length = header_length + 2 ;
		header_maw_3_values_offset  = header_maw_3_values_offset + 2 ;
		header_energy_values_offset  = header_energy_values_offset + 2 ;
		data = data + 0x2 ; // set bit 1
	}
	if (header_maw_3_values_enable_flag == 1) {
		header_length = header_length + 3 ;
		header_energy_values_offset  = header_energy_values_offset + 3 ;
		data = data + 0x4 ; // set bit 2
	}

	if (header_energy_filter_values_enable_flag == 1) {
		header_length = header_length + 2 ;
		data = data + 0x8 ; // set bit 3
	}

	if (maw_test_buffer_enable_flag == 1) {
		data = data + 0x10 ; // set bit 4
	}
	if (maw_test_buffer_energy_mux_flag == 1) {
		data = data + 0x20 ; // set bit 5
	}




	data = data + (data << 8) + (data << 16) + (data << 24);
	// data Format
	header_offset_ushort_ptr = 2 * header_length ; //
	return_code = gl_sis3316_adc1->register_write( SIS3316_ADC_CH1_4_DATAFORMAT_CONFIG_REG, data );
	return_code = gl_sis3316_adc1->register_write( SIS3316_ADC_CH5_8_DATAFORMAT_CONFIG_REG, data );
	return_code = gl_sis3316_adc1->register_write( SIS3316_ADC_CH9_12_DATAFORMAT_CONFIG_REG, data );
	return_code = gl_sis3316_adc1->register_write( SIS3316_ADC_CH13_16_DATAFORMAT_CONFIG_REG, data );



	// MAW Test Buffer configuration
	data = maw_test_buffer_length + (maw_test_buffer_delay << 16) ;
	return_code = gl_sis3316_adc1->register_write( SIS3316_ADC_CH1_4_MAW_TEST_BUFFER_CONFIG_REG, data );
	return_code = gl_sis3316_adc1->register_write( SIS3316_ADC_CH5_8_MAW_TEST_BUFFER_CONFIG_REG, data );
	return_code = gl_sis3316_adc1->register_write( SIS3316_ADC_CH9_12_MAW_TEST_BUFFER_CONFIG_REG, data );
	return_code = gl_sis3316_adc1->register_write( SIS3316_ADC_CH13_16_MAW_TEST_BUFFER_CONFIG_REG, data );

	// MAW Test Buffer start index  configuration
	data = maw_test_buffer_start_index + (energy_pickup_index << 16) ;
	for (i_adc_fpga=0; i_adc_fpga<4; i_adc_fpga++) {
		return_code = gl_sis3316_adc1->register_write( (i_adc_fpga*SIS3316_FPGA_ADC_REG_OFFSET) + SIS3316_ADC_CH1_MAW_START_INDEX_ENERGY_PICKUP_CONFIG_REG, data) ;
		return_code = gl_sis3316_adc1->register_write( (i_adc_fpga*SIS3316_FPGA_ADC_REG_OFFSET) + SIS3316_ADC_CH2_MAW_START_INDEX_ENERGY_PICKUP_CONFIG_REG, data) ;
		return_code = gl_sis3316_adc1->register_write( (i_adc_fpga*SIS3316_FPGA_ADC_REG_OFFSET) + SIS3316_ADC_CH3_MAW_START_INDEX_ENERGY_PICKUP_CONFIG_REG, data) ;
		return_code = gl_sis3316_adc1->register_write( (i_adc_fpga*SIS3316_FPGA_ADC_REG_OFFSET) + SIS3316_ADC_CH4_MAW_START_INDEX_ENERGY_PICKUP_CONFIG_REG, data) ;
	}




	unsigned int trigger_gate_window_length_conditions[11];

	// trigger_gate_window_length
	trigger_gate_window_length_conditions[0] = pre_trigger_delay + (energy_gap_val & 0xfff) + (2 * (energy_peaking_val & 0xfff)); // should be longer than the ENERGY FIR Filter (could be 1 x energy_peaking_val)
	if (sample_length == 0) {
		trigger_gate_window_length_conditions[1] = 0;
	}
	else {
		trigger_gate_window_length_conditions[1] = sample_start_index + sample_length + 10;
	}
	trigger_gate_window_length_conditions[2] = maw_test_buffer_start_index + maw_test_buffer_length + 10;


	if (header_accu_6_values_enable_flag == 1) {
		for (i_accu = 0; i_accu < 6; i_accu++) {
			trigger_gate_window_length_conditions[3 + i_accu] = (accumulator_length[i_accu] & 0x1ff) + (accumulator_start_index[i_accu] & 0xffff);
		}
	}
	else {
		for (i_accu = 0; i_accu < 6; i_accu++) {
			trigger_gate_window_length_conditions[3 + i_accu] = 0;
		}
	}

	if (header_accu_2_values_enable_flag == 1) {
		for (i_accu = 6; i_accu < 8; i_accu++) {
			trigger_gate_window_length_conditions[3 + i_accu] = (accumulator_length[i_accu] & 0x1ff) + (accumulator_start_index[i_accu] & 0xffff);
		}
	}
	else {
		for (i_accu = 6; i_accu < 8; i_accu++) {
			trigger_gate_window_length_conditions[3 + i_accu] = 0;
		}
	}

	// find the highest condition
	trigger_gate_window_length = trigger_gate_window_length_conditions[0];
	for (i = 1; i < 11; i++) {
		if (trigger_gate_window_length_conditions[i] > trigger_gate_window_length) {
			trigger_gate_window_length = trigger_gate_window_length_conditions[i];
		}
	}

	trigger_gate_window_length = trigger_gate_window_length + 4;
	if (trigger_gate_window_length > 0x10000 - 1) { trigger_gate_window_length = 0x10000 - 2; }
	//	printf("trigger_gate_window_length = 0x%08x  %d \n", trigger_gate_window_length, trigger_gate_window_length);
	return_code = gl_sis3316_adc1->register_write(SIS3316_ADC_CH1_4_TRIGGER_GATE_WINDOW_LENGTH_REG, (trigger_gate_window_length & 0xffff)); // trigger_gate_window_length
	return_code = gl_sis3316_adc1->register_write(SIS3316_ADC_CH5_8_TRIGGER_GATE_WINDOW_LENGTH_REG, (trigger_gate_window_length & 0xffff)); // trigger_gate_window_length
	return_code = gl_sis3316_adc1->register_write(SIS3316_ADC_CH9_12_TRIGGER_GATE_WINDOW_LENGTH_REG, (trigger_gate_window_length & 0xffff)); // trigger_gate_window_length
	return_code = gl_sis3316_adc1->register_write(SIS3316_ADC_CH13_16_TRIGGER_GATE_WINDOW_LENGTH_REG, (trigger_gate_window_length & 0xffff)); // trigger_gate_window_length


//**************************************************************************************************


// FIR Energy
	// set FIR Energy Setup
	for (i_adc_fpga=0; i_adc_fpga<4; i_adc_fpga++) {
		data =    ((energy_decay_tau_table_val & FIR_ENERGY_MAX_TAU_TABLE) << 30)
			   +  ((energy_decay_tau_factor_val & FIR_ENERGY_MAX_TAU_FACTOR) << 24)
			   +  ((energy_additional_average_val  & 0x3) << 22)
			   +  ((energy_gap_val  & 0x3ff) << 12)
			   +  (energy_peaking_val & 0xfff) ; //

		if ( ((this->adc_fpga_firmware_version & 0x0f00) == 0x0100) || ((this->adc_fpga_firmware_version & 0x0f00) == 0x0200)){ // Neutron/Gamma
			data =  0  ; // PSD Histogram configuration register have to be set 0
		}

		return_code = gl_sis3316_adc1->register_write( (i_adc_fpga*SIS3316_FPGA_ADC_REG_OFFSET) + SIS3316_ADC_CH1_FIR_ENERGY_SETUP_REG, data) ;
		return_code = gl_sis3316_adc1->register_write( (i_adc_fpga*SIS3316_FPGA_ADC_REG_OFFSET) + SIS3316_ADC_CH2_FIR_ENERGY_SETUP_REG, data) ;
		return_code = gl_sis3316_adc1->register_write( (i_adc_fpga*SIS3316_FPGA_ADC_REG_OFFSET) + SIS3316_ADC_CH3_FIR_ENERGY_SETUP_REG, data) ;
		return_code = gl_sis3316_adc1->register_write( (i_adc_fpga*SIS3316_FPGA_ADC_REG_OFFSET) + SIS3316_ADC_CH4_FIR_ENERGY_SETUP_REG, data) ;
	}


// Accumulatoren
	for (i_adc_fpga=0; i_adc_fpga<4; i_adc_fpga++) {
		for (i_accu=0; i_accu<8; i_accu++) {
			data =    (((accumulator_length[i_accu]  & 0x1ff) - 1) << 16)
			       +  (accumulator_start_index[i_accu] & 0xffff) ; //
			return_code = gl_sis3316_adc1->register_write( (i_adc_fpga*SIS3316_FPGA_ADC_REG_OFFSET) + (SIS3316_ADC_CH1_4_ACCUMULATOR_GATE1_CONFIG_REG + (i_accu*4)), data) ;
		}
	}

	

	// FIR Trigger
// disable all FIR Triggers
	data = 0x00000000 ;
	for (i_adc_fpga=0; i_adc_fpga<4; i_adc_fpga++) {
		return_code = gl_sis3316_adc1->register_write( (i_adc_fpga*SIS3316_FPGA_ADC_REG_OFFSET) + SIS3316_ADC_CH1_4_SUM_FIR_TRIGGER_THRESHOLD_REG, data );  // disable all ch_sum
		return_code = gl_sis3316_adc1->register_write( (i_adc_fpga*SIS3316_FPGA_ADC_REG_OFFSET) + SIS3316_ADC_CH1_FIR_TRIGGER_THRESHOLD_REG, data );  // disable ch1, 5, 9, 13
		return_code = gl_sis3316_adc1->register_write( (i_adc_fpga*SIS3316_FPGA_ADC_REG_OFFSET) + SIS3316_ADC_CH2_FIR_TRIGGER_THRESHOLD_REG, data );  // disable ch2, ..
		return_code = gl_sis3316_adc1->register_write( (i_adc_fpga*SIS3316_FPGA_ADC_REG_OFFSET) + SIS3316_ADC_CH3_FIR_TRIGGER_THRESHOLD_REG, data );  // disable ch3, ..
		return_code = gl_sis3316_adc1->register_write( (i_adc_fpga*SIS3316_FPGA_ADC_REG_OFFSET) + SIS3316_ADC_CH4_FIR_TRIGGER_THRESHOLD_REG, data );  // disable ch4, ..
	}

	// set HighEnergy Threshold
	data =  0x08000000 + (trigger_peaking_val * 0x1000) ; // gt 4096
	for (i_adc_fpga=0; i_adc_fpga<4; i_adc_fpga++) {
		return_code = gl_sis3316_adc1->register_write( (i_adc_fpga*SIS3316_FPGA_ADC_REG_OFFSET) + SIS3316_ADC_CH1_FIR_HIGH_ENERGY_THRESHOLD_REG, data);
		return_code = gl_sis3316_adc1->register_write( (i_adc_fpga*SIS3316_FPGA_ADC_REG_OFFSET) + SIS3316_ADC_CH2_FIR_HIGH_ENERGY_THRESHOLD_REG, data);
		return_code = gl_sis3316_adc1->register_write( (i_adc_fpga*SIS3316_FPGA_ADC_REG_OFFSET) + SIS3316_ADC_CH3_FIR_HIGH_ENERGY_THRESHOLD_REG, data);
		return_code = gl_sis3316_adc1->register_write( (i_adc_fpga*SIS3316_FPGA_ADC_REG_OFFSET) + SIS3316_ADC_CH4_FIR_HIGH_ENERGY_THRESHOLD_REG, data);
		return_code = gl_sis3316_adc1->register_write( (i_adc_fpga*SIS3316_FPGA_ADC_REG_OFFSET) + SIS3316_ADC_CH1_4_SUM_FIR_HIGH_ENERGY_THRESHOLD_REG, data);
	}

	// set FIR Trigger Setup
	for (i_adc_fpga=0; i_adc_fpga<4; i_adc_fpga++) {
		return_code = gl_sis3316_adc1->register_write( (i_adc_fpga*SIS3316_FPGA_ADC_REG_OFFSET) + SIS3316_ADC_CH1_4_SUM_FIR_TRIGGER_SETUP_REG, 0) ; // clear FIR Trigger Setup -> a following Setup will reset the logic !
		return_code = gl_sis3316_adc1->register_write( (i_adc_fpga*SIS3316_FPGA_ADC_REG_OFFSET) + SIS3316_ADC_CH1_FIR_TRIGGER_SETUP_REG, 0) ;
		return_code = gl_sis3316_adc1->register_write( (i_adc_fpga*SIS3316_FPGA_ADC_REG_OFFSET) + SIS3316_ADC_CH2_FIR_TRIGGER_SETUP_REG, 0) ;
		return_code = gl_sis3316_adc1->register_write( (i_adc_fpga*SIS3316_FPGA_ADC_REG_OFFSET) + SIS3316_ADC_CH3_FIR_TRIGGER_SETUP_REG, 0) ;
		return_code = gl_sis3316_adc1->register_write( (i_adc_fpga*SIS3316_FPGA_ADC_REG_OFFSET) + SIS3316_ADC_CH4_FIR_TRIGGER_SETUP_REG, 0) ;
	}

	// set FIR Trigger Setup
	for (i_adc_fpga=0; i_adc_fpga<4; i_adc_fpga++) {
		data =  ((trigger_pulse_length_val & 0xff) << 24)   + ((trigger_gap_val  & 0x3ff) << 12)  + (trigger_peaking_val & 0x3ff) ; //
		return_code = gl_sis3316_adc1->register_write( (i_adc_fpga*SIS3316_FPGA_ADC_REG_OFFSET) + SIS3316_ADC_CH1_4_SUM_FIR_TRIGGER_SETUP_REG, data) ;
		return_code = gl_sis3316_adc1->register_write( (i_adc_fpga*SIS3316_FPGA_ADC_REG_OFFSET) + SIS3316_ADC_CH1_FIR_TRIGGER_SETUP_REG, data) ;
		return_code = gl_sis3316_adc1->register_write( (i_adc_fpga*SIS3316_FPGA_ADC_REG_OFFSET) + SIS3316_ADC_CH2_FIR_TRIGGER_SETUP_REG, data) ;
		return_code = gl_sis3316_adc1->register_write( (i_adc_fpga*SIS3316_FPGA_ADC_REG_OFFSET) + SIS3316_ADC_CH3_FIR_TRIGGER_SETUP_REG, data) ;
		return_code = gl_sis3316_adc1->register_write( (i_adc_fpga*SIS3316_FPGA_ADC_REG_OFFSET) + SIS3316_ADC_CH4_FIR_TRIGGER_SETUP_REG, data) ;
	}


	// set Trigger Threshold
	trigger_threshold_val =  (trigger_peaking_val * trigger_threshold_adc_val) & 0x7ffffff ;   // bits 26:0
	trigger_threshold_val =  trigger_threshold_val + 0x08000000;   // bit is to 1 (zero line of MAW)
	if(trigger_HE_suppress_mode_flag == 1) {
		trigger_threshold_val =  trigger_threshold_val + 0x40000000;   // bit 30 
	}
	if(trigger_CFD_enable_flag == 1) {
		trigger_threshold_val =  trigger_threshold_val + 0x20000000;   // CFD zero crossing enable 29:28
	}
	else {
		if(trigger_CFD_enable_flag == 2) {
			trigger_threshold_val =  trigger_threshold_val + 0x30000000;   // CFD 50% enable 29:28
		}
	}


	i_ch=0 ;
	for (i_adc_fpga=0; i_adc_fpga<4; i_adc_fpga++) {
		data =  trigger_threshold_val ; //
		if (trigger_enable_flags[i_ch] == 1) {
			data =  data + 0x80000000 ; // Trigger Enable
		}
		return_code = gl_sis3316_adc1->register_write( (i_adc_fpga*SIS3316_FPGA_ADC_REG_OFFSET) + SIS3316_ADC_CH1_FIR_TRIGGER_THRESHOLD_REG, data) ;
		i_ch++ ;

		data =  trigger_threshold_val ; //
		if (trigger_enable_flags[i_ch] == 1) {
			data =  data + 0x80000000 ; // Trigger Enable
		}
		return_code = gl_sis3316_adc1->register_write( (i_adc_fpga*SIS3316_FPGA_ADC_REG_OFFSET) + SIS3316_ADC_CH2_FIR_TRIGGER_THRESHOLD_REG, data) ;
		i_ch++ ;

		data =  trigger_threshold_val ; //
		if (trigger_enable_flags[i_ch] == 1) {
			data =  data + 0x80000000 ; // Trigger Enable
		}
		return_code = gl_sis3316_adc1->register_write( (i_adc_fpga*SIS3316_FPGA_ADC_REG_OFFSET) + SIS3316_ADC_CH3_FIR_TRIGGER_THRESHOLD_REG, data) ;
		i_ch++ ;

		data =  trigger_threshold_val ; //
		if (trigger_enable_flags[i_ch] == 1) {
			data =  data + 0x80000000 ; // Trigger Enable
		}
		return_code = gl_sis3316_adc1->register_write( (i_adc_fpga*SIS3316_FPGA_ADC_REG_OFFSET) + SIS3316_ADC_CH4_FIR_TRIGGER_THRESHOLD_REG, data) ;
		i_ch++ ;
	}


	i_ch=16 ;
	for (i_adc_fpga=0; i_adc_fpga<4; i_adc_fpga++) {
		data =  trigger_threshold_val ; //
		if (trigger_enable_flags[i_ch+i_adc_fpga] == 1) {
			data =  data + 0x80000000 ; // Trigger Enable
		}
		return_code = gl_sis3316_adc1->register_write( (i_adc_fpga*SIS3316_FPGA_ADC_REG_OFFSET) + SIS3316_ADC_CH1_4_SUM_FIR_TRIGGER_THRESHOLD_REG, data) ;
		i_ch++ ;
	}



	// set HE-Trigger Threshold

	trigger_HE_threshold_val =  (trigger_peaking_val * trigger_HE_threshold_adc_val) & 0x7ffffff ;   // bits 26:0
	trigger_HE_threshold_val =  trigger_HE_threshold_val + 0x08000000;   // bit is to 1 (zero line of MAW)
	if(internalHeTriggerToVmeFPGAselection == 1) {
		trigger_HE_threshold_val =  trigger_HE_threshold_val + 0x40000000;   // bit 30 
	}
	if(internalTriggerToVmeFPGAselection == 1) {
		trigger_HE_threshold_val =  trigger_HE_threshold_val + 0x10000000;   //   29:28
	}
	else {
		if(internalTriggerToVmeFPGAselection == 2) {
			trigger_HE_threshold_val =  trigger_HE_threshold_val + 0x20000000;   //   29:28
		}
	}


	for (i_adc_fpga=0; i_adc_fpga<4; i_adc_fpga++) {
		return_code = gl_sis3316_adc1->register_write( (i_adc_fpga*SIS3316_FPGA_ADC_REG_OFFSET) + SIS3316_ADC_CH1_FIR_HIGH_ENERGY_THRESHOLD_REG, trigger_HE_threshold_val) ;
		return_code = gl_sis3316_adc1->register_write( (i_adc_fpga*SIS3316_FPGA_ADC_REG_OFFSET) + SIS3316_ADC_CH2_FIR_HIGH_ENERGY_THRESHOLD_REG, trigger_HE_threshold_val) ;
		return_code = gl_sis3316_adc1->register_write( (i_adc_fpga*SIS3316_FPGA_ADC_REG_OFFSET) + SIS3316_ADC_CH3_FIR_HIGH_ENERGY_THRESHOLD_REG, trigger_HE_threshold_val) ;
		return_code = gl_sis3316_adc1->register_write( (i_adc_fpga*SIS3316_FPGA_ADC_REG_OFFSET) + SIS3316_ADC_CH4_FIR_HIGH_ENERGY_THRESHOLD_REG, trigger_HE_threshold_val) ;
		return_code = gl_sis3316_adc1->register_write( (i_adc_fpga*SIS3316_FPGA_ADC_REG_OFFSET) + SIS3316_ADC_CH1_4_SUM_FIR_HIGH_ENERGY_THRESHOLD_REG, trigger_HE_threshold_val) ;
	}

//*******************


	uint_soft_trigger_flag = 0;  //
	if (fChkKeyTrigger->IsOn() == kTRUE) {
		uint_soft_trigger_flag = 1;  //
	}

 
	
	// 	Event Configuration
	fpga_data[0] = 0;
	fpga_data[1] = 0;
	fpga_data[2] = 0;
	fpga_data[3] = 0;
	// Bit 0: Invert
	for (i = 0; i < 4; i++) { // i_adc_fpga
		if (fChkInvertChannel[(4 * i) + 0]->IsOn() == kTRUE) { fpga_data[i] = fpga_data[i] + 0x1; }
		if (fChkInvertChannel[(4 * i) + 1]->IsOn() == kTRUE) { fpga_data[i] = fpga_data[i] + 0x100; }
		if (fChkInvertChannel[(4 * i) + 2]->IsOn() == kTRUE) { fpga_data[i] = fpga_data[i] + 0x10000; }
		if (fChkInvertChannel[(4 * i) + 3]->IsOn() == kTRUE) { fpga_data[i] = fpga_data[i] + 0x1000000; }
	}

	// Bit 1: Internal SUM-Trigger Enable bit
	for (i = 0; i < 4; i++) {
		if (fChkIntSumTriggerEnableCh[(4 * i) + 0]->IsOn() == kTRUE) { fpga_data[i] = fpga_data[i] + 0x2; }
		if (fChkIntSumTriggerEnableCh[(4 * i) + 1]->IsOn() == kTRUE) { fpga_data[i] = fpga_data[i] + 0x200; }
		if (fChkIntSumTriggerEnableCh[(4 * i) + 2]->IsOn() == kTRUE) { fpga_data[i] = fpga_data[i] + 0x20000; }
		if (fChkIntSumTriggerEnableCh[(4 * i) + 3]->IsOn() == kTRUE) { fpga_data[i] = fpga_data[i] + 0x2000000; }
	}

	// Bit 2: Internal Trigger Enable bit
	for (i = 0; i < 4; i++) {
		if (fChkIntTriggerEnableCh[(4 * i) + 0]->IsOn() == kTRUE) { fpga_data[i] = fpga_data[i] + 0x4; }
		if (fChkIntTriggerEnableCh[(4 * i) + 1]->IsOn() == kTRUE) { fpga_data[i] = fpga_data[i] + 0x400; }
		if (fChkIntTriggerEnableCh[(4 * i) + 2]->IsOn() == kTRUE) { fpga_data[i] = fpga_data[i] + 0x40000; }
		if (fChkIntTriggerEnableCh[(4 * i) + 3]->IsOn() == kTRUE) { fpga_data[i] = fpga_data[i] + 0x4000000; }
	}


	// Bit 3: External Trigger Enable bit
	for (i = 0; i < 4; i++) {
		if (fChkExtTriggerEnableCh[(4 * i) + 0]->IsOn() == kTRUE) { fpga_data[i] = fpga_data[i] + 0x8; }
		if (fChkExtTriggerEnableCh[(4 * i) + 1]->IsOn() == kTRUE) { fpga_data[i] = fpga_data[i] + 0x800; }
		if (fChkExtTriggerEnableCh[(4 * i) + 2]->IsOn() == kTRUE) { fpga_data[i] = fpga_data[i] + 0x80000; }
		if (fChkExtTriggerEnableCh[(4 * i) + 3]->IsOn() == kTRUE) { fpga_data[i] = fpga_data[i] + 0x8000000; }
	}

	// Bit 6: External Gate Enable bit
	for (i = 0; i < 4; i++) {
		if (fChkExtGateEnableCh[(4 * i) + 0]->IsOn() == kTRUE) { fpga_data[i] = fpga_data[i] + 0x40; }
		if (fChkExtGateEnableCh[(4 * i) + 1]->IsOn() == kTRUE) { fpga_data[i] = fpga_data[i] + 0x4000; }
		if (fChkExtGateEnableCh[(4 * i) + 2]->IsOn() == kTRUE) { fpga_data[i] = fpga_data[i] + 0x400000; }
		if (fChkExtGateEnableCh[(4 * i) + 3]->IsOn() == kTRUE) { fpga_data[i] = fpga_data[i] + 0x40000000; }
	}

	// Bit 7: External Veto Enable bit
	for (i = 0; i < 4; i++) {
		if (fChkExtVetoEnableCh[(4 * i) + 0]->IsOn() == kTRUE) { fpga_data[i] = fpga_data[i] + 0x80; }
		if (fChkExtVetoEnableCh[(4 * i) + 1]->IsOn() == kTRUE) { fpga_data[i] = fpga_data[i] + 0x8000; }
		if (fChkExtVetoEnableCh[(4 * i) + 2]->IsOn() == kTRUE) { fpga_data[i] = fpga_data[i] + 0x800000; }
		if (fChkExtVetoEnableCh[(4 * i) + 3]->IsOn() == kTRUE) { fpga_data[i] = fpga_data[i] + 0x80000000; }
	}

	return_code = gl_sis3316_adc1->register_write( SIS3316_ADC_CH1_4_EVENT_CONFIG_REG, fpga_data[0] ); //  Event Configuration
	return_code = gl_sis3316_adc1->register_write( SIS3316_ADC_CH5_8_EVENT_CONFIG_REG, fpga_data[1] ); //
	return_code = gl_sis3316_adc1->register_write( SIS3316_ADC_CH9_12_EVENT_CONFIG_REG, fpga_data[2]); //
	return_code = gl_sis3316_adc1->register_write( SIS3316_ADC_CH13_16_EVENT_CONFIG_REG, fpga_data[3]); //


	// Extended	Event Configuration
	fpga_data[0] = 0;
	fpga_data[1] = 0;
	fpga_data[2] = 0;
	fpga_data[3] = 0;
	// Bit 0: Internal Pileup Trigger
	for (i = 0; i < 4; i++) {
		if (fChkIntPileupTriggerEnableCh[(4 * i) + 0]->IsOn() == kTRUE) { fpga_data[i] = fpga_data[i] + 0x1; }
		if (fChkIntPileupTriggerEnableCh[(4 * i) + 1]->IsOn() == kTRUE) { fpga_data[i] = fpga_data[i] + 0x100; }
		if (fChkIntPileupTriggerEnableCh[(4 * i) + 2]->IsOn() == kTRUE) { fpga_data[i] = fpga_data[i] + 0x10000; }
		if (fChkIntPileupTriggerEnableCh[(4 * i) + 3]->IsOn() == kTRUE) { fpga_data[i] = fpga_data[i] + 0x1000000; }
	}

	// Bit 4: Save Raw data of first Event of Bankbuffer only” Enable
	uint_save_raw_data_first_event_only_flag = 0;
	if (fChk_SaveRawDataFirstEventOnly->IsOn() == kTRUE) {
		uint_save_raw_data_first_event_only_flag = 1;
		for (i = 0; i < 4; i++) {  // i_adc_fpga
			fpga_data[i] = fpga_data[i] + 0x10101010 ;
		}
	}

	return_code = gl_sis3316_adc1->register_write( SIS3316_ADC_CH1_4_EXTENDED_EVENT_CONFIG_REG, fpga_data[0]); //   
	return_code = gl_sis3316_adc1->register_write( SIS3316_ADC_CH5_8_EXTENDED_EVENT_CONFIG_REG, fpga_data[1]); //
	return_code = gl_sis3316_adc1->register_write( SIS3316_ADC_CH9_12_EXTENDED_EVENT_CONFIG_REG, fpga_data[2]); //
	return_code = gl_sis3316_adc1->register_write( SIS3316_ADC_CH13_16_EXTENDED_EVENT_CONFIG_REG, fpga_data[3]); //



	// Internal Trigger Feedback Select register
	data = 0;
	if (fChkFeedbackCoincidence1TriggerEnable->IsOn() == kTRUE) {
		data = data + 0x1000000;
	}
	for (i = 0; i < 16; i++) { // i_ch
		if (fChkIntFeedbackTriggerEnableCh[i]->IsOn() == kTRUE) {
			data = data + (1 << i);
		}
	}
	return_code = gl_sis3316_adc1->register_write(SIS3316_INTERNAL_TRIGGER_FEEDBACK_SELECT_REG, data); //   
	//printf("SIS3316_INTERNAL_TRIGGER_FEEDBACK_SELECT_REG  = 0x%08x \n", data);

	
	//Internal Trigger Delay Configuration registers
	data = fNumericEntries_InternalDelay->GetIntNumber();
	data = data & 0x1fe; // only even
	fNumericEntries_InternalDelay->SetIntNumber(data); //  
	data = (data >> 1); //  

	return_code = gl_sis3316_adc1->register_write(SIS3316_NIM_INPUT_CONTROL_REG, (data << 24) + (data << 16) + (data << 8) + data); //


	data = (fNumericEntries_VetoDelay->GetIntNumber()) & 0xffff; //   
	return_code = gl_sis3316_adc1->register_write(SIS3316_EXTERNAL_VETO_GATE_DELAY_REG, (data << 24) + (data << 16) + (data << 8) + data); //



// Lemo Input "TI" configuration
	data = 0;
	if (fChkLemoInTiEnable->IsOn() == kTRUE) {
		data = data + 0x10;
	}
	if (fChkLemoInUiAsVetoEnable->IsOn() == kTRUE) {
		data = data + 0x1000;
	}
	return_code = gl_sis3316_adc1->register_write( SIS3316_NIM_INPUT_CONTROL_REG, data ); //


	// enbale external (global) functions
	data = 0;
	if (fChkExternalTriggerDisableWithBusyEnable->IsOn() == kTRUE) {
		data = data + 0x8000; //External Trigger Disable with internal Busy select
	}
	if (fChkFeedbackInternalTriggerEnable->IsOn() == kTRUE) {
		data = data + 0x4000; //Feedback Selected Internal Trigger as External Trigger Enable
	}
	if (fChkExternalTriggerFunc->IsOn() == kTRUE) {
		data = data + 0x100; // enable "external Trigger function" (NIM In, if enabled and VME key write)
	}

	if (fChkExternalTriggerFuncAsVeto->IsOn() == kTRUE) {
		data = data + 0x200; // enable "external Trigger function" (NIM In, if enabled and VME key write)
	}

	if (fChkLocalVetoFuncAsVeto->IsOn() == kTRUE) {
		data = data + 0x800; //  
	}


	data = data + 0x400 ; // enable "external Timestamp clear function" (NIM In, if enabled and VME key write)
	return_code = gl_sis3316_adc1->register_write( SIS3316_ACQUISITION_CONTROL_STATUS, data );

/**************************************************************************************************************/
 
	event_length       = (header_length + (sample_length / 2) + maw_test_buffer_length);
	short_event_length = (header_length + maw_test_buffer_length);
	

	if (uint_SampleControl_BankModus == 0) { // SINGLE_EVENT_SINGLE_BANK 
		nof_events_pro_bank	= 1; // 
		possibe_nof_events_pro_bank = 1; // 
	}
	else { // MULTI_EVENT_DOUBLE_BANK
		if (event_length > (SIS3316_ADC_MEMORY_BANK_32BIT_SIZE / 2)) { // if event length > halffull then set to 1 event
			nof_events_pro_bank = 1; // 
			possibe_nof_events_pro_bank = 1; // 
		}
		else {
			if (uint_save_raw_data_first_event_only_flag == 1) {
				possibe_nof_events_pro_bank = (((SIS3316_ADC_MEMORY_BANK_32BIT_SIZE * 3) / 4) - event_length) / short_event_length; // (3/4 memory - 1 Event ) / short_event_length
				possibe_nof_events_pro_bank = possibe_nof_events_pro_bank + 1; // 
			}
			else {
				possibe_nof_events_pro_bank = ((SIS3316_ADC_MEMORY_BANK_32BIT_SIZE * 3) / 4) / event_length; // 3/4 full
				if (possibe_nof_events_pro_bank == 0) {
					possibe_nof_events_pro_bank = 1; // 
				}
			}
		}

		// limit NofEvents ?
		if (this->root_chk_bank_event_nof_limit_on_flag == 1) {
		//if (fChkNofEvents_ProBank->IsOn() == kTRUE) {
				gui_max_events_pro_bank = fNumericEntries_SampleControl_MaxNofEvents_ProBank->GetIntNumber() ; //
			if (gui_max_events_pro_bank >= possibe_nof_events_pro_bank) {
				nof_events_pro_bank	= possibe_nof_events_pro_bank; // 
			}
			else {
				nof_events_pro_bank	= gui_max_events_pro_bank; // 
			}
		}
		else {
				nof_events_pro_bank	= possibe_nof_events_pro_bank; // 
		}
	}
	fNumericEntries_SampleControl_NofEvents_ProBank->SetIntNumber(nof_events_pro_bank); // 
	fNumericEntries_SampleControl_PossibleMaxNofEvents_ProBank->SetIntNumber(possibe_nof_events_pro_bank); // 

	if (uint_save_raw_data_first_event_only_flag == 1) {
		address_threshold = event_length + ((nof_events_pro_bank - 1) * short_event_length) - 2;  //    ;  //  
	}
	else {
		address_threshold = (nof_events_pro_bank * event_length) - 2;  //    ;  //  
	}


	// bit 31: “Suppress saving of more Hits/Events if Memory Address Threshold Flag is valid” Enable* 
	if (fChk_SuppressEventsIfAddrThresFlag->IsOn() == kTRUE) {
		address_threshold = address_threshold + 0x80000000 ;
	}

	return_code = gl_sis3316_adc1->register_write( SIS3316_ADC_CH1_4_ADDRESS_THRESHOLD_REG, address_threshold ); //
	return_code = gl_sis3316_adc1->register_write( SIS3316_ADC_CH5_8_ADDRESS_THRESHOLD_REG, address_threshold); //
	return_code = gl_sis3316_adc1->register_write( SIS3316_ADC_CH9_12_ADDRESS_THRESHOLD_REG, address_threshold ); //
	return_code = gl_sis3316_adc1->register_write( SIS3316_ADC_CH13_16_ADDRESS_THRESHOLD_REG, address_threshold ); //



	//printf("INFO:  trigger_gate_window_length = %08d    \n", trigger_gate_window_length);
	//printf("INFO:  nof_events_pro_bank = %08d    event_length =%08d  address_threshold = 0x%08x\n", nof_events_pro_bank, event_length,  address_threshold, return_code);
	fNumericEntries_EventHitParameter[6]->SetIntNumber(event_length); // Info: Event Length (32-bit words)
	fNumericEntries_EventHitParameter[7]->SetIntNumber(trigger_gate_window_length); // Info: Active Trigger Gate Length
	fNumericEntries_EventHitParameter[8]->SetIntNumber(address_threshold&0x7FFFFFFF); // Info: address_threshold

/**************************************************************************************************************/

	uint_WriteData_to_File_OpenFlag = 0 ;
	uint_WriteData_to_File_counter  = 0 ;


	if (uint_SampleControl_BankModus == 0) {
		uint_WriteData_to_File_EnableFlag    = 0 ;
		uint_WriteData_to_MultipleFiles_Flag = 0 ;
	}
	else {
		if (fChkWriteDataToFile->IsOn() == kTRUE)  {
			uint_WriteData_to_File_EnableFlag    = 1 ;
			if (fChkWriteMultipleFiles->IsOn() == kTRUE)  {
				uint_WriteData_to_MultipleFiles_Flag = 1 ;
			}
			else {
				uint_WriteData_to_MultipleFiles_Flag = 0 ;
			}
		}
		else {
			uint_WriteData_to_File_EnableFlag    = 0 ;	
			uint_WriteData_to_MultipleFiles_Flag = 0 ;
		}
	}
 
	if (uint_WriteData_to_File_EnableFlag == 1) {
		strcpy(char_WriteData_to_File_initialize_filename, fTextEntryDataFilePath->GetText()) ; //  
		for (i=0;i<strlen(char_WriteData_to_File_initialize_filename);i++) {
			if (char_WriteData_to_File_initialize_filename[i] == '.')  {
				break ;
			}
		}
		char_WriteData_to_File_initialize_filename[i] = '\0';   /* null character manually added */
	}


	// Statistic Update Mode = 1:
	// The Readout - Trigger - Statistic - Counter - Latches will be latched
	// with each bank switching (at the end of a bank sampling).
	return_code = gl_sis3316_adc1->register_write(SIS3316_ADC_CH1_4_TRIGGER_STATISTIC_COUNTER_MODE_REG, 1);  //  
	return_code = gl_sis3316_adc1->register_write(SIS3316_ADC_CH5_8_TRIGGER_STATISTIC_COUNTER_MODE_REG, 1);  //  
	return_code = gl_sis3316_adc1->register_write(SIS3316_ADC_CH9_12_TRIGGER_STATISTIC_COUNTER_MODE_REG, 1);  //  
	return_code = gl_sis3316_adc1->register_write(SIS3316_ADC_CH13_16_TRIGGER_STATISTIC_COUNTER_MODE_REG, 1);  //  


	SIS3316_Test_running_dim_widgets(kFALSE);
	fNumericEntriesTimeSecCounterView->SetIntNumber(0); //  
	fNumericEntriesBankLoopCounterView->SetIntNumber(0); //  
	gl_sis3316_adc1->register_read(SIS3316_INTERNAL_TEMPERATURE_REG, &data);
	fNumericEntriesTemperatureView->SetIntNumber((data & 0x3FF) / 4); //  

	/* get Start Time */
	gettimeofday(&time_start, NULL);


	unsigned int zoom_draw_length;



	/****************************************************************************************************************/
	/***********************                                           **********************************************/
	/***********************   Start of SINGLE_EVENT_SINGLE_BANK Loop  **********************************************/
	/***********************                                           **********************************************/

	if (uint_SampleControl_BankModus == 0) {

		#define SINGLE_EVENT_SINGLE_BANK
		#ifdef SINGLE_EVENT_SINGLE_BANK

		bank_buffer_counter=0;


		

#ifdef FFT_GRAPH
		// FFT Graph Setup
		fGraph_fft[0] = new TGraph(MAX_ROOT_PLOT_LENGTH / 2, this->root_float_fft_x, this->root_float_fft_y);
		fGraph_fft[1] = new TGraph(MAX_ROOT_PLOT_LENGTH / 2, this->root_float_fft_x, this->root_float_fft_y1);
		fGraph_fft[0]->SetLineColor(DefineChannel_1_Color);
		fGraph_fft[1]->SetLineColor(DefineCanvasBackgroundColor);

		fftw_plan p;
		fftw_complex* fftw_complex_in, * fftw_complex_out;

		fftw_complex_in = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * plot_length);
		fftw_complex_out = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * plot_length);

		double_fft_frequency = this->double_clock_configure_fft_frequency;
#endif 
		/**************************************************************************************************************/


		unsigned int first_time_draw_flag;
		first_time_draw_flag = 0;

	// running loop
		while (fSIS3316_Test1_Run_Cmd) {

			// Clear Timestamp  */
			return_code = gl_sis3316_adc1->register_write( SIS3316_KEY_TIMESTAMP_CLEAR , 0);  //

		// ** Paramter Refreh in Single Event Modus
		// refresh PreTriggerDelay
			pre_trigger_delay = fNumericEntries_EventHitParameter[0]->GetIntNumber();
			if (old_pre_trigger_delay != pre_trigger_delay) {
				if (pre_trigger_delay > MAX_PRETRIGGER_DELAY) {
					pre_trigger_delay = MAX_PRETRIGGER_DELAY ;
					fNumericEntries_EventHitParameter[0]->SetIntNumber(pre_trigger_delay );
				}
				old_pre_trigger_delay = pre_trigger_delay ;
 				return_code = gl_sis3316_adc1->register_write( SIS3316_ADC_CH1_4_PRE_TRIGGER_DELAY_REG, pre_trigger_delay ); //
				return_code = gl_sis3316_adc1->register_write( SIS3316_ADC_CH5_8_PRE_TRIGGER_DELAY_REG, pre_trigger_delay ); //
				return_code = gl_sis3316_adc1->register_write( SIS3316_ADC_CH9_12_PRE_TRIGGER_DELAY_REG, pre_trigger_delay ); //
				return_code = gl_sis3316_adc1->register_write( SIS3316_ADC_CH13_16_PRE_TRIGGER_DELAY_REG, pre_trigger_delay ); //
			}


		// refresh MAW PreTriggerDelay
			maw_test_buffer_delay = fNumericEntries_EventHitParameter[3]->GetIntNumber();
			if (old_maw_test_buffer_delay != maw_test_buffer_delay) {
				if (maw_test_buffer_delay > 1022) {
					maw_test_buffer_delay = 1022 ;
					fNumericEntries_EventHitParameter[3]->SetIntNumber(maw_test_buffer_delay );
				}
				old_maw_test_buffer_delay = maw_test_buffer_delay ;
				data = maw_test_buffer_length + (maw_test_buffer_delay << 16) ;
				return_code = gl_sis3316_adc1->register_write( SIS3316_ADC_CH1_4_MAW_TEST_BUFFER_CONFIG_REG, data );
				return_code = gl_sis3316_adc1->register_write( SIS3316_ADC_CH5_8_MAW_TEST_BUFFER_CONFIG_REG, data );
				return_code = gl_sis3316_adc1->register_write( SIS3316_ADC_CH9_12_MAW_TEST_BUFFER_CONFIG_REG, data );
				return_code = gl_sis3316_adc1->register_write( SIS3316_ADC_CH13_16_MAW_TEST_BUFFER_CONFIG_REG, data );
			}

			// refresh DAC
			if ((offset_parameter_has_changed_flag == 1) || ((fChkDacInrementTest->IsOn() == kTRUE)) ) {
				// refresh adc DAC offset setup
				SIS3316_Test_Write_DacOffset() ;
 				usleep(2000); // 2 ms (because max. pretrigger delay)
			}

			// refresh Gain/Termination
			if(gain_termination_parameter_has_changed_flag != 0) {
				SIS3316_Test_Write_TerminationGain() ;
  				usleep(2000); // 2 ms (because max. pretrigger delay)
			}

			gSystem->ProcessEvents();  // handle GUI events

		/******************************************/
		//Note: Start sampling on Bank on alternate Bank, check Bit 24 in the register "previous Bank sample address" 
			gl_sis3316_adc1->register_read( SIS3316_ADC_CH1_PREVIOUS_BANK_SAMPLE_ADDRESS_REG, &data);
			if((data & 0x1000000) == 0x1000000 ) { 	// bank2 flag is set ?
				//printf("bank2 flag is set\n"); // start sampling an alternate bank
				bank1_armed_flag = 1 ;
				bank2_read_flag  = 0 ;
				// start sampling
				return_code = gl_sis3316_adc1->register_write( SIS3316_KEY_DISARM_AND_ARM_BANK1, 0 ); // //  Arm
			}	
			else {
				//printf("bank2 flag is not set\n"); // start sampling an alternate bank
				bank1_armed_flag = 0 ;
				bank2_read_flag  = 1 ;
				// start sampling
				return_code = gl_sis3316_adc1->register_write( SIS3316_KEY_DISARM_AND_ARM_BANK2, 0 ); // //  Arm
			}
			gSystem->ProcessEvents();  // handle GUI events

			if (uint_soft_trigger_flag == 1) {
				usleep(1);
				return_code = gl_sis3316_adc1->register_write( SIS3316_KEY_TRIGGER , 0);  //  Trigger
			}

		// wait for address threshold flag
			//printf("wait for address threshold flag\n");
			do {
				gSystem->ProcessEvents();  // handle GUI events
				return_code = gl_sis3316_adc1->register_read( SIS3316_ACQUISITION_CONTROL_STATUS, &data);
			} while (((data & 0x80000) == 0x0) && (fSIS3316_Test1_Run_Cmd == kTRUE) && (return_code == 0)) ; // address Threshold ?
			//	Sleep(50); // 50ms
			if (return_code != 0) {
				fSIS3316_Test1_Run_Cmd = kFALSE ;
			}

			return_code = gl_sis3316_adc1->register_write( SIS3316_KEY_DISARM, 0 );  //  Disarm
			if (fSIS3316_Test1_Run_Cmd == kTRUE) {

			do {
				gSystem->ProcessEvents();  // handle GUI events
				gl_sis3316_adc1->register_read( SIS3316_ACQUISITION_CONTROL_STATUS, &data);
					//printf("SIS3316_KEY_DISARM   0x%08x     \n", data);
			} while ((data & 0x50000) != 0x0) ; // wait until logic is not Busy
			gSystem->ProcessEvents();  // handle GUI events
			

			// read all ADC channels which have data
			//req_nof_32bit_words =  (sample_length/2) +  header_length ;
			req_nof_32bit_words =  event_length ;
			for (i_ch=0; i_ch<16; i_ch++) {
				return_code = gl_sis3316_adc1->read_DMA_Channel_PreviousBankDataBuffer( bank2_read_flag /*bank2_read_flag*/, i_ch /* 0 to 15 */, req_nof_32bit_words/*max_read_nof_words */, &got_nof_32bit_words, (unsigned int *) ushort_adc_buffer_array_ptr[i_ch] ) ;
				if (return_code != 0) {
					printf("Error: read_DMA_Channel_PreviousBankDataBuffer: i_ch %d  got_nof_32bit_words = 0x%08x  return_code = 0x%08x\n",i_ch,  got_nof_32bit_words, return_code);
					ch_data_valid[i_ch] = 0;
					fSIS3316_Test1_Run_Cmd = kFALSE ;
				}
				else {
					if (got_nof_32bit_words >= req_nof_32bit_words) {
						ch_data_valid[i_ch] = 1;
					}
					else {
						ch_data_valid[i_ch] = 0;
					}
				}
			}
			  


			/*************************************************************************************************************************/

//#define TEST_PRINT_HEADER
#ifdef TEST_PRINT_HEADER

			printf("header_length = 0x%08x     \n", header_length);
			unsigned int* uint_buffer_ptr;
			i_ch = 0;
			if (ch_data_valid[i_ch] == 1) {
				uint_buffer_ptr = (unsigned int*)this->ushort_adc_buffer_array_ptr[i_ch];
			}
			for (i = 0; i < header_length+1; i++) {
				printf("header %1d = 0x%08x     \n", i, uint_buffer_ptr[i]);
			}
			printf("\n");
#endif

			
		/*************************************************************************************************************************/
			/*  Display Graph */
		/*************************************************************************************************************************/

			/*  Display Graph */
			//printf("Display Graph plot_length = %d \n", plot_length);
			if(plot_length != 0) {	
				if (fChkDisplayDisableDeleteGraph->IsOn() == kFALSE) {
					first_time_draw_flag = 0;
					xmax = this->raw_graph_xmax;
					xmin = this->raw_graph_xmin;
					zoom_draw_length = xmax - xmin;

					// check min and max for y-cordiante
					if (fChkDisplayAutoZoom->IsOn() == kTRUE) { 
						// search for min and max for y-cordiante
						ymax = 0;
						ymin = 0xffff;
						for (i = 0; i < 16; i++) {
							ushort_adc_buffer_ptr = this->ushort_adc_buffer_array_ptr[i];
							if (fChkDisplayAdc[i]->IsOn() == kTRUE) {
								if (ch_data_valid[i] == 1) {
									//for (ui = header_offset_ushort_ptr; ui < plot_length + header_offset_ushort_ptr; ui++) {
									for (ui = header_offset_ushort_ptr + xmin; ui < zoom_draw_length + xmin + header_offset_ushort_ptr ; ui++) {
											if (ushort_adc_buffer_ptr[ui] < ymin) ymin = ushort_adc_buffer_ptr[ui];
										if (ushort_adc_buffer_ptr[ui] > ymax) ymax = ushort_adc_buffer_ptr[ui];
									}
								}
							}
						} // i
					}
					else { // Autozoom is off
						ymax = this->raw_graph_ymax;
						ymin = this->raw_graph_ymin;
					}
						
					// clear Display
					fCanvas1->Clear();
					fCanvas1->cd();
					// Display X and Y-axis
					for (i = 0; i < plot_length; i++) {
						this->root_gr_y[i] = ymin + ((ymax - ymin) / 2);
					}
					this->root_gr_y[(zoom_draw_length / 2) + xmin ] = ymin;
					this->root_gr_y[((zoom_draw_length / 2)) + xmin + 1] = ymax;
					fGraph_ch[16]->DrawGraph(zoom_draw_length, &this->root_gr_x[xmin], &this->root_gr_y[xmin], "AL");
				} // end of (disable_delete_raw_data_plot_flag == 0) 
				else  { //(ChkDisplayDisableDeleteGraph->IsOn() == kTRUE)
					// x-axis
					xmin = 0;
					xmax = this->raw_graph_xmax_absolute;
					// y-axis
					ymin = 0;
					ymax = this->raw_graph_ymax_absolute;
					zoom_draw_length = xmax - xmin;

					if (first_time_draw_flag == 0) {
						fCanvas1->Clear();
						fCanvas1->cd();
						// Display axis
						for (i = 0; i < plot_length; i++) {
							this->root_gr_y[i] = ymin + ((ymax - ymin) / 2);
						}
						this->root_gr_y[(plot_length / 2)] = ymin;
						this->root_gr_y[((plot_length / 2)) + 1] = ymax;
						fGraph_ch[16]->DrawGraph(plot_length, this->root_gr_x, this->root_gr_y, "AL");
						first_time_draw_flag = 1;
					}
				}
 
				ywidth = ymax - ymin;
				y_delta = ywidth / 17;
				fCanvas1->cd();
				//fCanvas1->Update();

				// Display channels
				for(i_ch = 0; i_ch < 16; i_ch++){
					if(ch_data_valid[i_ch] == 1) {
						ushort_adc_buffer_ptr = this->ushort_adc_buffer_array_ptr[i_ch] + header_offset_ushort_ptr ;
						if (fChkDisplayAdc[i_ch]->IsOn() == kTRUE) {
							for (ui=0; ui<plot_length; ui++){
								this->root_gr_y[ui] = (int)(ushort_adc_buffer_ptr[ui] )  ;
							}
							fGraph_ch[i_ch]->DrawGraph(plot_length, this->root_gr_x, this->root_gr_y, "L");
							fGraph_Text_ch[i_ch]->DrawLatex(xmax + (zoom_draw_length / 10), ymax - ((16 - i_ch) * y_delta), chkDisAdcLabel[i_ch]);
						}
					}
				}
				fCanvas1->Update();
				//fCanvas1->Modified();
				gSystem->ProcessEvents();  // handle GUI events

			#ifdef TEST_PRINT
					for(i_ch = 0; i_ch < 16; i_ch++){
						if(ch_data_valid[i_ch] == 1) {
							ushort_adc_buffer_ptr = this->ushort_adc_buffer_array_ptr[i_ch]  ;
							printf("ch%2d Channel ID = 0x%03x     \n", i_ch+1, ( ((unsigned int ) ushort_adc_buffer_ptr[0]) & 0xFFF0 ) >> 4);
						}
					}
					printf("\n");
			#endif
			}

	/***************************************************************************************************/

	/*************************************************************************************************************************/
	/*  Display FIR Filter MAW Graph  (Trigger or Energy Filter) */
	/*************************************************************************************************************************/

		#ifdef MAW_GRAPH
			if (maw_test_buffer_length != 0) {
				display_MAW_choice = fCombo_Display_MAW->GetSelected();
				if (display_MAW_choice > 0)  {
					if (fB_openfCanvas4WindowFlag == kFALSE) {
						for(i=0;i<MAX_ROOT_PLOT_MAW_LENGTH;i++) {
							this->root_gr_maw_x[i] = i ;
							this->root_gr_maw_y[i] = 0 ;
						}
						fCanvas4 = new TCanvas("fCanvas4","SIS3316 Filter: Moving Average Window", SIS3316_MAW_DATA_WINDOW_POSTION_X, SIS3316_MAW_DATA_WINDOW_POSTION_Y, SIS3316_MAW_WINDOW_WIDTH, SIS3316_MAW_DATA_WINDOW_HIGH);
						fB_openfCanvas4WindowFlag = kTRUE; //

						fCanvas4->Clear();
						fCanvas4->cd();
						fCanvas4->SetGrid();
						fCanvas4->SetFillColor(DefineCanvasBackgroundColor);
						fGraph_maw = new TGraph(MAX_ROOT_PLOT_MAW_LENGTH, this->root_gr_maw_x, this->root_gr_maw_y);

						fGraph_maw->SetTitle("MAW");
						fGraph_maw->SetLineColor(DefineChannel_1_Color);

					}
					else {
						fCanvas4->Clear();
						fCanvas4->cd();
					}
				}
				else {
					if (fB_openfCanvas4WindowFlag == kTRUE) {
						delete fCanvas4;
						delete fGraph_maw;
						fB_openfCanvas4WindowFlag = kFALSE; //
					}
				}

				ushort_adc_buffer_ptr = this->ushort_adc_buffer_array_ptr[display_MAW_choice-1];
				uint_maw_ptr = (unsigned int*) ushort_adc_buffer_ptr ;

				if ((display_MAW_choice > 0) && (ch_data_valid[display_MAW_choice-1] == 1)) {
					for (ui=0; ui<maw_test_buffer_length; ui++){
						this->root_gr_maw_y[ui] = uint_maw_ptr[ui + header_length + (sample_length/2)] - uint_trigger_maw_offset ; //  0x08000000 or 0x0
					}
					fGraph_maw->DrawGraph(maw_test_buffer_length, this->root_gr_maw_x, this->root_gr_maw_y,"AL");
					fCanvas4->Update();
				}
			}

		#endif

	/***************************************************************************************************/

	/*************************************************************************************************************************/
	/*  Display FFT  */
	/*************************************************************************************************************************/

	#ifdef FFT_GRAPH

				display_FFT_choice = fCombo_Display_FFT_Ch->GetSelected();
				if (display_FFT_choice > 0)  {
					if (fB_openfCanvas3WindowFlag == kFALSE) {
						fCanvas3 = new TCanvas("fCanvas3","Fast Fourier Transform ", SIS3316_FFT_WINDOW_POSTION_X, SIS3316_FFT_WINDOW_POSTION_Y, SIS3316_FFT_WINDOW_WIDTH, SIS3316_FFT_WINDOW_HIGH);
						fB_openfCanvas3WindowFlag = kTRUE; //
						fChkFFT_Sum->SetState(kButtonUp)   ; // is Off !
						fChkFFTLogY->SetState(kButtonUp)   ; // is Off !
						fChkFFT_Sum->SetEnabled(kFALSE)   ; //
						fChkFFTLogY->SetEnabled(kFALSE)   ; //
						fChkFFT_Db->SetEnabled(kTRUE)   ; //
						fChkFFT_Db->SetState(kButtonDown)   ; // is On !
					}
					fCanvas3->Clear();
					fCanvas3->cd();
					fCanvas3->SetGrid();
					fCanvas3->SetFillColor(DefineCanvasBackgroundColor);

					fCanvas3->SetFrameBorderMode(1);
					fCanvas3->SetFrameBorderSize(6);
					fCanvas3->SetFrameFillColor(DefineCanvasBackgroundColor);
					//fCanvas3->Update();
				}
				else {
					if (fB_openfCanvas3WindowFlag == kTRUE) {
						delete fCanvas3 ;
						fB_openfCanvas3WindowFlag = kFALSE; //
						fChkFFT_Db->SetEnabled(kTRUE)   ; //
						if (fChkFFT_Db->IsOn() == kTRUE)  {
							fChkFFT_Sum->SetEnabled(kFALSE)   ; //
							fChkFFTLogY->SetEnabled(kFALSE)   ; //
						}
						else {
							fChkFFT_Sum->SetEnabled(kTRUE)   ; //
							fChkFFTLogY->SetEnabled(kTRUE)   ; //
						}
					}
				}

				fft_plot_length = sample_length / 2 ;
				if ((display_FFT_choice > 0) && (ch_data_valid[display_FFT_choice-1] == 1)) {
					ushort_adc_buffer_ptr = this->ushort_adc_buffer_array_ptr[display_FFT_choice-1] + header_offset_ushort_ptr;

					int int_sum_value ;
					int_sum_value = 0 ;
					for(ui = 0;ui < sample_length;ui++){
						int_sum_value =  int_sum_value + (unsigned int)(ushort_adc_buffer_ptr[ui] & 0xFFFF) ;
					}
					int_sum_value =  ((int_sum_value   /  sample_length) );

					unsigned int uint_value ;
					int int_value ;
					for(ui = 0;ui < sample_length;ui++){
						uint_value =  (unsigned int)(ushort_adc_buffer_ptr[ui] & 0xFFFF) ;
						int_value =  (int)uint_value ;
						this->root_int_save_adc_buffer[ui] =   int_value - int_sum_value  ;
					}

				//#ifdef Window_FUNCTION
					#define M_PI       3.14159265358979323846
					double double_a0, double_a1, double_a2, double_a3 ;
					double double_window_correctur_factor ;
					double_window_correctur_factor = 1.0 ;

					for(ui = 0;ui < sample_length;ui++){
						this->root_double_window_weight[ui] =  1.015 ; // no window
					}
					display_FFT_Window_choice = fCombo_Display_FFT_Window->GetSelected();
					switch(display_FFT_Window_choice) {
						case 1: //Hamming window
							double_window_correctur_factor = 1.368 * 1.368 ;
							for(ui = 0;ui < sample_length;ui++){
								this->root_double_window_weight[ui] =  0.54 - (0.46 * (cos((2 * M_PI * ui) / (sample_length-1) )))  ; // Hamming Window
								//printf ( "1  %3d  %12f \n", ui, this->root_double_window_weight[ui] );
							}
							break;
						case 2: //Hann window
							double_window_correctur_factor = 1.419  * 1.419 ;
							for(ui = 0;ui < sample_length;ui++){
								this->root_double_window_weight[ui] =  0.5 * (1 - cos( (2 * M_PI * ui) / (sample_length-1) ))  ; // HAnn Window
								//printf ( "2  %3d  %12f \n", ui, this->root_double_window_weight[ui] );
							}
							break;
						case 3: //Blackmann window
							double_window_correctur_factor = 1.5365 * 1.5365 ;
							double_a0 = 7938.0/18608.0 ;
							double_a1 = 9240.0/18608.0 ;
							double_a2 = 1430.0/18608.0 ;
							for(ui = 0;ui < sample_length;ui++){
								this->root_double_window_weight[ui] =  double_a0 - (double_a1 * cos( (2 * M_PI * ui) / (sample_length-1) ))  + (double_a2 * cos( (4 * M_PI * ui) / (sample_length-1) )); // Blackmann Window
								//printf ( "2  %3d  %12f \n", ui, this->root_double_window_weight[ui] );
							}
							break;
						case 4: //Blackmann-Harris window
							double_window_correctur_factor = 1.6697 * 1.6697 ;
							double_a0 = 0.35875 ;
							double_a1 = 0.48829 ;
							double_a2 = 0.14128 ;
							double_a3 = 0.01168 ;
							for(ui = 0;ui < sample_length;ui++){
								this->root_double_window_weight[ui] =  double_a0 - (double_a1 * cos( (2 * M_PI * ui) / (sample_length-1) ))  + (double_a2 * cos( (4 * M_PI * ui) / (sample_length-1) )) - (double_a3 * cos( (6 * M_PI * ui) / (sample_length-1) )) ; // Blackmann-Harris Window
								//printf ( "2  %3d  %12f \n", ui, this->root_double_window_weight[ui] );
							}
							break;
					}

				//#endif
				// FFT
					for(ui = 0;ui < sample_length;ui++){
						fftw_complex_in[ui][0] =  (double)this->root_int_save_adc_buffer[ui]  * this->root_double_window_weight[ui] ; // real.
						fftw_complex_in[ui][1] =  (double) 0.0 ; // imag..
					}

					p = fftw_plan_dft_1d (sample_length, fftw_complex_in, fftw_complex_out, FFTW_FORWARD, FFTW_ESTIMATE);
					fftw_execute(p);

					for(ui = 1;ui < fft_plot_length;ui++){
						this->root_double_fft_spectrum[ui] =  double_window_correctur_factor * (double)((sqrtf  ( (float)(fftw_complex_out[ui][0] * fftw_complex_out[ui][0]) + (float)(fftw_complex_out[ui][1]  * fftw_complex_out[ui][1]) )  / (fft_plot_length/2))) ;
						//printf ( "  %3d  %12f  %12f  %12f\n", ui, fftw_complex_out[ui][0], fftw_complex_out[ui][1], spectrum[ui] );
					}
					fftw_destroy_plan(p) ;

					fCanvas3->Clear();
					fCanvas3->cd();
				//
					double log10_value ;
					double noise_floor;
					log10_value = log10 ((float)(sample_length/2));

					if (gl_sis3316_adc1->adc_125MHz_flag == 1) {
						noise_floor = ((6.02*16) + 1.76 + (10 * log10_value)) ; // 16-bit ADC
					}
					else {
						noise_floor = ((6.02*14) + 1.76 + (10 * log10_value)) ; // 14-bit ADC
					}
					//printf("noise_floor = %f\n", noise_floor);

					for (ui=0;ui<fft_plot_length;ui++){
						this->root_float_fft_x[ui] = (float) ((ui * double_fft_frequency) / fft_plot_length / 2 ) ;
						  if (fChkFFT_Db->IsOn() == kTRUE)  {
							   if (gl_sis3316_adc1->adc_125MHz_flag == 1) {
								   this->root_float_fft_y[ui] = (float)(20.0 * ( log10 (this->root_double_fft_spectrum[ui] / 65535.0) ))  ; // Amplitude Spectrum 16 bit
									//	this->root_float_fft_y[ui] = 10.0 * ( log10 ((this->root_double_fft_spectrum[ui] * this->root_double_fft_spectrum[ui]  ) / (65535.0 * 65535.0)) ) ; // Power Spectrum
							   }
							   else {
								   this->root_float_fft_y[ui] = (float)(20.0 * ( log10 (this->root_double_fft_spectrum[ui] / 16383.0) )) ; // Amplitude Spectrum 14 bit
							   }
						  }
						  else {
								if (fChkFFT_Sum->IsOn() == kTRUE)  {
									this->root_float_fft_y[ui] = this->root_float_fft_y[ui] + (Float_t)this->root_double_fft_spectrum[ui] ;
								}
								else {
									this->root_float_fft_y[ui] = (Float_t)this->root_double_fft_spectrum[ui]  ;
								}
							}
					}

					if (fChkFFT_AutoScale->IsOn() == kFALSE)  {
						fGraph_fft[1]-> DrawGraph(fft_plot_length-1, &this->root_float_fft_x[1], &this->root_float_fft_y1[1],"AL");
						fGraph_fft[0]-> DrawGraph(fft_plot_length-1, &this->root_float_fft_x[1], &this->root_float_fft_y[1],"L");
					}
					else {
						fGraph_fft[0]-> DrawGraph(fft_plot_length-1, &this->root_float_fft_x[1], &this->root_float_fft_y[1],"AL");
					}
					fCanvas3->Update();
				}
		#endif

	
				gSystem->ProcessEvents();  // handle GUI events

	/*************************************************************************************************************************/
	/*  Build and Display HISTOGRAM  */
	/*************************************************************************************************************************/

		#ifdef HISTOGRAM
				display_histogram_choice = fCombo_Display_Histos_Ch->GetSelected();
	
				if (display_histogram_choice == 0) {
					if (fB_openfCanvas2WindowFlag == kTRUE) {
						delete fCanvas2;
						fB_openfCanvas2WindowFlag = kFALSE; //
						fChkHistoSum->SetState(kButtonUp); // is Off !

					}
				}
				else { // (display_histogram_choice == 0)
					if (fB_openfCanvas2WindowFlag == kFALSE) {
						fCanvas2 = new TCanvas("fCanvas2", "ADC Output Code Histogram ", SIS3316_HISTOGRAM_WINDOW_POSTION_X_SINGLE, SIS3316_HISTOGRAM_WINDOW_POSTION_Y_SINGLE, SIS3316_HISTOGRAM_WINDOW_WIDTH_SINGLE, SIS3316_HISTOGRAM_WINDOW_HIGH_SINGLE);
						fB_openfCanvas2WindowFlag = kTRUE; // Setup
					}
					if (display_histogram_choice == 18) {
						fCanvas2->Clear();
						fCanvas2->Divide(2, 8);
						for (i = 0; i < 16; i++) {
							fCanvas2->cd(1 + i);
							gPad->SetGrid();
							gPad->SetFillColor(DefineCanvasBackgroundColor);
						}
					}
					else {
						fCanvas2->Clear();
						fCanvas2->Divide(1);
						fCanvas2->cd();
						fCanvas2->SetGrid();
						fCanvas2->SetFillColor(DefineCanvasBackgroundColor);
					}

					// loop over N channels (build histograms)
					for (i = 0; i < 16; i++) {
						if (fChkHistoSum->IsOn() == kFALSE) {
							iHistoAdc[i]->Reset(); //
							iHistoAdc[i]->BufferEmpty(1); // action =  1 histogram is filled and buffer is deleted
						}

						if (ch_data_valid[i] == 1) {
							ushort_adc_buffer_ptr = this->ushort_adc_buffer_array_ptr[i] + header_offset_ushort_ptr;
							for (ui = 0; ui < plot_length; ui++) {
								iHistoAdc[i]->Fill((int)ushort_adc_buffer_ptr[ui]);
							}

							double_histo_mean = iHistoAdc[i]->GetMean(1); //
							if (double_histo_mean < double_histo_min_mean[i]) {
								double_histo_min_mean[i] = double_histo_mean;
							}
							if (double_histo_mean > double_histo_max_mean[i]) {
								double_histo_max_mean[i] = double_histo_mean;
							}
							histo_pave_text[i]->Clear();
							histo_pave_text[i]->SetTextSize((float)0.04);
							sprintf(char_temp, "current mean %5.1f", double_histo_mean);
							histo_pave_text[i]->AddText(char_temp);
							sprintf(char_temp, "minimal mean %5.1f", double_histo_min_mean[i]);
							histo_pave_text[i]->AddText(char_temp);
							sprintf(char_temp, "maximal mean %5.1f", double_histo_max_mean[i]);
							histo_pave_text[i]->AddText(char_temp);

							if (fChkHistoZoomMean->IsOn() == kTRUE) {
								double_histo_mean = iHistoAdc[i]->GetMean(1); //
								//printf("double_histo_mean = %f\n", double_histo_mean);
								double_histo_min_x = 0.0;
								// double_histo_max_x = 65535.0;
								// double_histo_max_x = 16384;
								double_histo_max_x = (double)(this->root_histo_xmax_absolute - 1); //  65535.0;
								if (double_histo_mean > 35.0) { double_histo_min_x = double_histo_mean - 35.0; }
								//if (double_histo_mean < 65515.0) { double_histo_max_x = double_histo_mean + 20.0;}
								if (double_histo_mean < (double)(this->root_histo_xmax_absolute - 35)) { double_histo_max_x = double_histo_mean + 35.0; }
								iHistoAdc[i]->SetAxisRange(double_histo_min_x, double_histo_max_x, "X"); //
							}
							else {
								double_histo_min_x = (double)this->root_histo_xmin;
								//double_histo_max_x = 16384;
								double_histo_max_x = (double)(this->root_histo_xmax - 1);
								iHistoAdc[i]->SetAxisRange(double_histo_min_x, double_histo_max_x, "X"); //
							}
						} // (ch_data_valid[i] == 1)
					} // loop over N channels
				} // (display_histogram_choice == 0)


				 // display histograms
				if (display_histogram_choice > 1) { // display histograms

					// display histograms
					display_histo_counter++;
					//if (display_histo_counter > 20) { // display every 20 Loops
					if (display_histo_counter >= 0) {
						//printf("\ndisplay_histo_counter  %d \n",display_histo_counter);
						display_histo_counter=0;

						if (fChkHistoGaussFit->IsOn() == kTRUE)  {
						  histogram_gausfit_enable_flag = 1 ;
						  histogram_gausfit_clear_flag  = 0 ;
						}
						else {
						  if (histogram_gausfit_enable_flag == 1) {
							histogram_gausfit_clear_flag  = 1 ;
						  }
						  histogram_gausfit_enable_flag = 0 ;
						}

						//printf("display_histogram_choice = %d\n", display_histogram_choice);

						if (display_histogram_choice == 18) {
							display_histo_ch_no = 0;
							for (i=0;i<16;i++) {
								fCanvas2->cd(1+display_histo_ch_no);
								if (histogram_gausfit_enable_flag == 1)  {
								  iHistoAdc[display_histo_ch_no]->Fit("gaus","Q");
								}
								if (histogram_gausfit_clear_flag == 1)  {
								  iHistoAdc[display_histo_ch_no]->Fit("gaus","0","Q");
								  histogram_gausfit_clear_flag = 0 ;
								}

								iHistoAdc[display_histo_ch_no]->Draw();
								histo_pave_text[display_histo_ch_no]->Draw();
								//printf("display_histo_ch_no = %d\n",display_histo_ch_no);
								display_histo_ch_no++ ;
								if (display_histo_ch_no > 15) {
									display_histo_ch_no = 0 ;
								}
							}
						}
						else {
							display_histo_ch_no = display_histogram_choice-2;
							fCanvas2->cd();
							if (histogram_gausfit_enable_flag == 1)  {
							  iHistoAdc[display_histo_ch_no]->Fit("gaus","Q");
							}
							if (histogram_gausfit_clear_flag == 1)  {
							  iHistoAdc[display_histo_ch_no]->Fit("gaus","0","Q");
							  histogram_gausfit_clear_flag = 0 ;
							}
							iHistoAdc[display_histo_ch_no]->Draw();
							histo_pave_text[display_histo_ch_no]->Draw();
						}
						  fCanvas2->Update();
						//fCanvas2->Modified();
					}
				}

		#endif
			}
			gSystem->ProcessEvents();  // handle GUI events


			//*********************************************************************************************************************

			bank_buffer_counter++;
			if (fChkStopAfterBanks->IsOn() == kTRUE)  {
				 if (bank_buffer_counter >= (unsigned int)fNumericEntriesStopAfterBanks->GetIntNumber() ) {
					fSIS3316_Test1_Run_Cmd = kFALSE ;
				 }
			}

			// check stop condition 
			gettimeofday(&time_actual, NULL);
			double_run_time_sec = ((double)time_actual.tv_sec - (double)time_start.tv_sec);
			uint_run_time_sec = (unsigned int)double_run_time_sec;
			//printf("uint_run_time_sec = %d\n", uint_run_time_sec);
			if (fChkStopAfterTime->IsOn() == kTRUE) {
				if (uint_run_time_sec >= (unsigned int)fNumericEntriesStopAfterTime->GetIntNumber()) {
					fSIS3316_Test1_Run_Cmd = kFALSE;
				}
			}

			fNumericEntriesTimeSecCounterView->SetIntNumber(uint_run_time_sec); //  
			fNumericEntriesBankLoopCounterView->SetIntNumber(bank_buffer_counter); //  
			gl_sis3316_adc1->register_read(SIS3316_INTERNAL_TEMPERATURE_REG, &data);
			fNumericEntriesTemperatureView->SetIntNumber((data & 0x3FF) / 4); //  


		} //while (fSIS3316_Test1_Run_Cmd)
		#endif //#ifdef SINGLE_EVENT_SINGLE_BANK

#ifdef FFT_GRAPH
		delete fGraph_fft[0];
		delete fGraph_fft[1];
		fftw_free(fftw_complex_in);
		fftw_free(fftw_complex_out);
#endif

	/***********************   End of SINGLE_EVENT_SINGLE_BANK Loop   **********************************************/
	} //if (uint_SampleControl_BankModus == 0)









	/****************************************************************************************************************/
	/***********************                                           **********************************************/
	/***********************   Start of MULTI_EVENT_DOUBLE_BANK Loop   **********************************************/
	/***********************                                           **********************************************/

	plot_counter=0;
	bank_buffer_counter=0;


	if (uint_SampleControl_BankModus == 1) {
		fCombo_Display_Histos_Build->Select(1, kTRUE); // FIR Energy, only (yet)

		// Clear Timestamp  */
		return_code = gl_sis3316_adc1->register_write( SIS3316_KEY_TIMESTAMP_CLEAR , 0);  //  
	
		//Note: Start sampling on Bank on alternate Bank, check Bit 24 in the register "previous Bank sample address" 
		gl_sis3316_adc1->register_read( SIS3316_ADC_CH1_PREVIOUS_BANK_SAMPLE_ADDRESS_REG, &data);
		if((data & 0x1000000) == 0x1000000 ) { 	// bank2 flag is set ?
			//printf("bank2 flag is set\n"); // start sampling an alternate bank
			bank1_armed_flag = 1 ;
			// start sampling
			return_code = gl_sis3316_adc1->register_write( SIS3316_KEY_DISARM_AND_ARM_BANK1, 0 ); // //  Arm
			//printf("SIS3316_KEY_DISARM_AND_ARM_BANK1 \n");
		}	
		else {
			//printf("bank2 flag is not set\n"); // start sampling an alternate bank
			bank1_armed_flag = 0 ;
			// start sampling
			return_code = gl_sis3316_adc1->register_write( SIS3316_KEY_DISARM_AND_ARM_BANK2, 0 ); // //  Arm
			//printf("SIS3316_KEY_DISARM_AND_ARM_BANK2 \n");
		}

		unsigned int max_req_nof_32bit_words;
		unsigned int ch_event_counter;



		// start Loop
		while (fSIS3316_Test1_Run_Cmd) {

			do {
				// generates triggers if it is enabled
				if (uint_soft_trigger_flag == 1) {
					return_code = gl_sis3316_adc1->register_write( SIS3316_KEY_TRIGGER , 0);  //  Trigger
					gSystem->ProcessEvents();  // handle GUI events
				}
				poll_counter=100;
				do {
					gSystem->ProcessEvents();  // handle GUI events
					poll_counter--;
				} while (poll_counter != 0) ;
				return_code = gl_sis3316_adc1->register_read (SIS3316_ACQUISITION_CONTROL_STATUS, &data);  
			} while (((data & 0x80000) == 0x0) && (fSIS3316_Test1_Run_Cmd == kTRUE) && (return_code == 0)) ; // address Threshold ?

			if (return_code != 0) {
				fSIS3316_Test1_Run_Cmd = kFALSE ;
			}

			if (fSIS3316_Test1_Run_Cmd == kTRUE) {

			// bank swap command
				if (bank1_armed_flag == 1) {
					return_code = gl_sis3316_adc1->register_write(SIS3316_KEY_DISARM_AND_ARM_BANK2 , 0);  //  Arm Bank2
					bank1_armed_flag = 0; // bank 2 is armed
					//printf("SIS3316_KEY_DISARM_AND_ARM_BANK2 \n");
				}
				else {
					return_code = gl_sis3316_adc1->register_write(SIS3316_KEY_DISARM_AND_ARM_BANK1 , 0);  //  Arm Bank1
					bank1_armed_flag = 1; // bank 1 is armed
					//printf("SIS3316_KEY_DISARM_AND_ARM_BANK1 \n");
				}
 
			// prepare file write
				if (uint_WriteData_to_File_EnableFlag == 1) {   ; //  
					if (uint_WriteData_to_File_OpenFlag == 0) {   // first time
						if (uint_WriteData_to_MultipleFiles_Flag == 0) {   //  
							sprintf(char_WriteData_to_File_filename,"%s.dat",char_WriteData_to_File_initialize_filename ) ;
						}
						else {
							sprintf(char_WriteData_to_File_filename,"%s_%d.dat",char_WriteData_to_File_initialize_filename ,  uint_WriteData_to_File_counter) ;
						}
						file_WriteData_to_File_Pointer = fopen(char_WriteData_to_File_filename,"wb") ;
						uint_WriteData_to_File_OpenFlag = 1 ;
					}
					else { // file is open
						if (uint_WriteData_to_MultipleFiles_Flag == 1) {   //  
							if (uint_WritenData_to_File_32bit_words > WRITE_DTATA_TO_FILE_MAX_32BIT_WORDS) {
								uint_WritenData_to_File_32bit_words = 0 ;
								fclose(file_WriteData_to_File_Pointer);
								uint_WriteData_to_File_counter++;
								sprintf(char_WriteData_to_File_filename,"%s_%d.dat",char_WriteData_to_File_initialize_filename ,  uint_WriteData_to_File_counter) ;
								file_WriteData_to_File_Pointer = fopen(char_WriteData_to_File_filename,"wb") ;
							}
						}
					}
				}


				/*************************************************************************************************************************/
				/*  Display Raw Data Graph X and Y-axis  */
				/*************************************************************************************************************************/
				if (plot_length != 0) {

					// Display: prepare X and Y-axis
					xmax = this->raw_graph_xmax;
					xmin = this->raw_graph_xmin;
					zoom_draw_length = raw_graph_xmax - raw_graph_xmin;

					ymax = this->raw_graph_ymax;
					ymin = this->raw_graph_ymin;
					ywidth = ymax - ymin;
					y_delta = ywidth / 17;

					// clear Display
					fCanvas1->Clear();
					fCanvas1->cd();

					// Display X and Y-axis
					for (i = 0; i < plot_length; i++) {
						this->root_gr_y[i] = ymin + ((ymax - ymin) / 2);
					}
					this->root_gr_y[(zoom_draw_length / 2) + xmin] = ymin;
					this->root_gr_y[((zoom_draw_length / 2)) + xmin + 1] = ymax;
					//fGraph_ch[16]->DrawGraph(plot_length, this->root_gr_x, this->root_gr_y, "AL");
					fGraph_ch[16]->DrawGraph(zoom_draw_length, &this->root_gr_x[xmin], &this->root_gr_y[xmin], "AL");
				}

				//max_req_nof_32bit_words = nof_events_pro_bank * event_length ; // max_request is limited by request nof_events
				max_req_nof_32bit_words = SIS3316_ADC_MEMORY_BANK_32BIT_SIZE ; // max_request is limited by Memory Banks_Size
				
				display_histogram_choice = fCombo_Display_Histos_Ch->GetSelected();


				// loop over n channels
				for (i_ch=0; i_ch<16; i_ch++) {
					gSystem->ProcessEvents();  // handle GUI events
					// read channel events
					return_code = gl_sis3316_adc1->read_DMA_Channel_PreviousBankDataBuffer(bank1_armed_flag /*bank2_read_flag*/, i_ch /* 0 to 15 */, max_req_nof_32bit_words, &got_nof_32bit_words, dma_data_buffer) ; // read maximun (all) events
					if (return_code != 0) {
						printf("read_DMA_Channel_PreviousBankDataBuffer ch%d: return_code = 0x%08x\n", i_ch+1, return_code);
						fSIS3316_Test1_Run_Cmd = kFALSE ;
						break;
					}
					//printf("read_Channel_PreviousBankDataBuffer: i_ch %d  got_nof_32bit_words = 0x%08x  return_code = 0x%08x\n",i_ch+1,  got_nof_32bit_words, return_code);

					ch_event_counter = 0 ;
					if (got_nof_32bit_words > 0) {
						if (uint_save_raw_data_first_event_only_flag == 1) {
							ch_event_counter = 1 + ((got_nof_32bit_words - event_length) / short_event_length);
						}
						else {
							ch_event_counter = (got_nof_32bit_words / event_length);
						}
					}

					//if (got_nof_32bit_words > 0) {
					//	printf("ch %d  got_nof_32bit_words = %d  event_length = %d   short_event_length = %d    ch_event_counter  = %d  \n", i_ch + 1, got_nof_32bit_words, event_length, short_event_length, ch_event_counter);
					//}
						
					/*************************************************************************************************************************/
					/* write to file  */
					/*************************************************************************************************************************/

					file_header_short_and_maw_length = 0;
					if (uint_save_raw_data_first_event_only_flag == 1) {
						file_header_short_and_maw_length = 0x80000000 + ((short_event_length & 0x7fff) << 16) ; // set bit 31 and short_length
					}
					file_header_short_and_maw_length = file_header_short_and_maw_length + (maw_test_buffer_length & 0xffff)   ; // set bit 31 and short_length

					file_header_reserved_EventBufferLen = got_nof_32bit_words & 0xfffffff ;

					if (got_nof_32bit_words > 0) {
						if (uint_WriteData_to_File_OpenFlag == 1) {
							uint_WritenData_to_File_32bit_words += SIS3316_WriteBankChannelHeaderToDataFile(file_WriteData_to_File_Pointer, file_header_indentifier, bank_buffer_counter, i_ch, ch_event_counter, event_length, file_header_short_and_maw_length, file_header_reserved_EventBufferLen);
							uint_WritenData_to_File_32bit_words += SIS3316_WriteBankChannelEventBufferToDataFile(file_WriteData_to_File_Pointer, dma_data_buffer, got_nof_32bit_words);
						}
					}


					if (ch_event_counter > 0) {

						/*************************************************************************************************************************/
						/*  Display Raw data Graph   (Note : only first Event                                            )                       */
						/*************************************************************************************************************************/
						// Raw data graph
						if (plot_length != 0) {
							if (fChkDisplayAdc[i_ch]->IsOn() == kTRUE) {
								//for (i = 0; i < ch_event_counter; i++) {
								for (i = 0; i < 1; i++) { // Display only first Event !!!
									ushort_adc_buffer_ptr = (unsigned short*)(&dma_data_buffer[i * (event_length)+header_length]);
									for (ui = 0; ui < plot_length; ui++) {
										this->root_gr_y[ui] = (ushort_adc_buffer_ptr[ui]);
									}
									fGraph_ch[i_ch]->DrawGraph(plot_length, this->root_gr_x, this->root_gr_y, "L");
								}
								fGraph_Text_ch[i_ch]->DrawLatex(xmax + (zoom_draw_length / 10), ymax - ((16 - i_ch) * y_delta), chkDisAdcLabel[i_ch]);
								fCanvas1->Update();
							}
						}


						/*************************************************************************************************************************/
						/*  Energy Filter Histogramming Example   */
						/*************************************************************************************************************************/

							//volatile unsigned int uint_save_energy_max ;
						unsigned int uint_energy_max ;
 
						// Fir Filter Energy histogram (build histograms)
						if (((display_histogram_choice > 0)) && (header_energy_filter_values_enable_flag == 1)) {

							if (uint_save_raw_data_first_event_only_flag == 1) {
								// first event
								i = 0;
								uint_energy_max = dma_data_buffer[(i * event_length) + header_energy_values_offset + 1]; // Max. Energy value
								uint_energy_max = (unsigned int)(uint_energy_max / fNumericEntriesHistogramXaxisDivider->GetNumber()) + fNumericEntriesHistogramXaxisOffset->GetIntNumber();
								if ((uint_energy_max > 0) && (uint_energy_max < this->root_histo_xmax_absolute)) {
									iHistoAdc[i_ch]->Fill((int)uint_energy_max);
								}
								if (ch_event_counter > 1) {
									for (i = 1; i < ch_event_counter; i++) {
										uint_energy_max = dma_data_buffer[(event_length)+((i - 1) * short_event_length) + header_energy_values_offset + 1];
										uint_energy_max = (unsigned int)(uint_energy_max / fNumericEntriesHistogramXaxisDivider->GetNumber()) + fNumericEntriesHistogramXaxisOffset->GetIntNumber();
										if ((uint_energy_max > 0) && (uint_energy_max < this->root_histo_xmax_absolute)) {
											iHistoAdc[i_ch]->Fill((int)uint_energy_max);
										}
									}
								}
							}
							else {
								for (i = 0; i < ch_event_counter; i++) {
									uint_energy_max = dma_data_buffer[(i * event_length) + header_energy_values_offset + 1];
									uint_energy_max = (unsigned int)(uint_energy_max / fNumericEntriesHistogramXaxisDivider->GetNumber()) + fNumericEntriesHistogramXaxisOffset->GetIntNumber();
									if ((uint_energy_max > 0) && (uint_energy_max < this->root_histo_xmax_absolute)) {
										iHistoAdc[i_ch]->Fill((int)uint_energy_max);
									}
									//printf("uint_energy_max = %d\n", uint_energy_max);
								}
							}
						}

					} //if (ch_event_counter > 0) {
				} // loop over n channels


				// open / close histograms
				if (display_histogram_choice == 0) {
					if (fB_openfCanvas2WindowFlag == kTRUE) {
						delete fCanvas2;
						fB_openfCanvas2WindowFlag = kFALSE; //
						fChkHistoSum->SetState(kButtonUp); // is Off !
					}
				}
				else { // (display_histogram_choice == 0)
					if (fB_openfCanvas2WindowFlag == kFALSE) {
						fCanvas2 = new TCanvas("fCanvas2", "Energy Histogram ", SIS3316_HISTOGRAM_WINDOW_POSTION_X_MULTI, SIS3316_HISTOGRAM_WINDOW_POSTION_Y_MULTI, SIS3316_HISTOGRAM_WINDOW_WIDTH_MULTI, SIS3316_HISTOGRAM_WINDOW_HIGH_MULTI);
						fB_openfCanvas2WindowFlag = kTRUE; // Setup
					}
					if (display_histogram_choice == 18) {
						fCanvas2->Clear();
						fCanvas2->Divide(2, 8);
						for (i = 0; i < 16; i++) {
							fCanvas2->cd(1 + i);
							gPad->SetGrid();
							gPad->SetFillColor(DefineCanvasBackgroundColor);
						}
					}
					else {
						fCanvas2->Clear();
						fCanvas2->Divide(1);
						fCanvas2->cd();
						fCanvas2->SetGrid();
						fCanvas2->SetFillColor(DefineCanvasBackgroundColor);
					}
				} // (else of display_histogram_choice == 0)

				if (display_histogram_choice > 1) { // display histograms
					if (fChkHistoGaussFit->IsOn() == kTRUE) {
						histogram_gausfit_enable_flag = 1;
						histogram_gausfit_clear_flag = 0;
					}
					else {
						if (histogram_gausfit_enable_flag == 1) {
							histogram_gausfit_clear_flag = 1;
						}
						histogram_gausfit_enable_flag = 0;
					}

					//printf("display_histogram_choice = %d\n", display_histogram_choice);
					double_histo_min_x = (double)this->root_histo_xmin;
					double_histo_max_x = (double)(this->root_histo_xmax - 1);

					if (display_histogram_choice == 18) {
						display_histo_ch_no = 0;
						for (i = 0; i < 16; i++) {
							fCanvas2->cd(1 + display_histo_ch_no);
							iHistoAdc[display_histo_ch_no]->SetAxisRange(double_histo_min_x, double_histo_max_x, "X"); //

							if (histogram_gausfit_enable_flag == 1) {
								iHistoAdc[display_histo_ch_no]->Fit("gaus", "Q");
							}
							if (histogram_gausfit_clear_flag == 1) {
								iHistoAdc[display_histo_ch_no]->Fit("gaus", "0", "Q");
								histogram_gausfit_clear_flag = 0;
							}

							iHistoAdc[display_histo_ch_no]->Draw();
							//histo_pave_text[display_histo_ch_no]->Draw();
							//printf("display_histo_ch_no = %d\n",display_histo_ch_no);
							display_histo_ch_no++;
							if (display_histo_ch_no > 15) {
								display_histo_ch_no = 0;
							}
						}
					}
					else {
						display_histo_ch_no = display_histogram_choice - 2;
						fCanvas2->cd();
						iHistoAdc[display_histo_ch_no]->SetAxisRange(double_histo_min_x, double_histo_max_x, "X"); //
						if (histogram_gausfit_enable_flag == 1) {
							iHistoAdc[display_histo_ch_no]->Fit("gaus", "Q");
						}
						if (histogram_gausfit_clear_flag == 1) {
							iHistoAdc[display_histo_ch_no]->Fit("gaus", "0", "Q");
							histogram_gausfit_clear_flag = 0;
						}
						iHistoAdc[display_histo_ch_no]->Draw();
						//histo_pave_text[display_histo_ch_no]->Draw();
					}
					fCanvas2->Update();
				}

			} // if (fSIS3316_Test1_Run_Cmd == kTRUE) {



			// check stop condition Loops (nof bank switches)
			bank_buffer_counter++;
			if (fChkStopAfterBanks->IsOn() == kTRUE)  {
				 if (bank_buffer_counter >= (unsigned int)fNumericEntriesStopAfterBanks->GetIntNumber() ) {
					fSIS3316_Test1_Run_Cmd = kFALSE ;
				 }
			}

			// check stop condition Time
			gettimeofday(&time_actual, NULL);
			double_run_time_sec = ((double)time_actual.tv_sec - (double)time_start.tv_sec);
			uint_run_time_sec = (unsigned int)double_run_time_sec;
			//printf("uint_run_time_sec = %d\n", uint_run_time_sec);
			if (fChkStopAfterTime->IsOn() == kTRUE) {
				if (uint_run_time_sec >= (unsigned int)fNumericEntriesStopAfterTime->GetIntNumber()) {
					fSIS3316_Test1_Run_Cmd = kFALSE;
				}
			}

			// display information
			fNumericEntriesTimeSecCounterView->SetIntNumber(uint_run_time_sec); //  
			fNumericEntriesBankLoopCounterView->SetIntNumber(bank_buffer_counter); //  
			gl_sis3316_adc1->register_read(SIS3316_INTERNAL_TEMPERATURE_REG, &data);
			fNumericEntriesTemperatureView->SetIntNumber((data & 0x3FF) / 4); //  
			
			gSystem->ProcessEvents();  // handle GUI events



			if (fChkDisplayStatisticCounters->IsOn() == kTRUE) {
				fTextView->Clear();
				fTextView->ShowBottom();
				sprintf(this->char_TextView, "Internal Trigger Counters   \n");
				fTextView->AddLineFast(this->char_TextView);
				sprintf(this->char_TextView, "\n");
				fTextView->AddLineFast(this->char_TextView);
				sprintf(this->char_TextView, "            All       Hits/    Deadtime      Pileup        Veto    supressed\n");
				fTextView->AddLineFast(this->char_TextView);
				sprintf(this->char_TextView, "                     Events                                      High Energy \n");
				fTextView->AddLineFast(this->char_TextView);
				sprintf(this->char_TextView, "\n");
				fTextView->AddLineFast(this->char_TextView);
				for (i_fpga = 0; i_fpga < 4; i_fpga++) {
					gl_sis3316_adc1->read_Channel_StatisticCounter(i_fpga /* 0 to 3 */, uint_statistic_buffer); // new 27.08.2019

					for (i_ch = 0; i_ch < 4; i_ch++) {
						sprintf(this->char_TextView, "ch%2d ", (i_fpga * 4) + i_ch + 1);
						for (i = 0; i < 6; i++) {
							sprintf(this->char_TextView + strlen(this->char_TextView), "%10d  ", uint_statistic_buffer[(i_ch * 6) + i]);
						}
						//printf("\n");
						fTextView->AddLineFast(this->char_TextView);
					}
				}
				sprintf(this->char_TextView, "\n");
				fTextView->AddLineFast(this->char_TextView);
				fTextView->ShowBottom();
				fTextView->Update();
			}


		} //while (fSIS3316_Test1_Run_Cmd)


		// end of acquisition		  
		  
		  // Disarm 
		return_code = gl_sis3316_adc1->register_write( SIS3316_KEY_DISARM , 0);  //   
		// Close File
		if (uint_WriteData_to_File_OpenFlag == 1) {   ; //  
			fclose(file_WriteData_to_File_Pointer);
			uint_WriteData_to_File_OpenFlag = 0 ;
		}


#ifdef not_used_tests
		printf("\n");
		// Statistic Counters
		printf("     \t Internal Trigger Counters   \n");
		printf("     \t All          Hits/Events  Deadtime     Pileup       Veto         High Energy supressed \n");
		for (i_fpga = 0; i_fpga < 4; i_fpga++) {
				gl_sis3316_adc1->read_Channel_StatisticCounter(i_fpga /* 0 to 3 */, uint_statistic_buffer); // new 27.08.2019
				for (i_ch = 0; i_ch < 4; i_ch++) {
					printf("ch%d \t", (i_fpga * 4) + i_ch + 1);
					for (i = 0; i < 6; i++) {
						printf(" 0x%08x  ", uint_statistic_buffer[(i_ch * 6) + i]);
					}
					printf("\n");
				}
			}
#endif
		if (fChkDisplayStatisticCounters->IsOn() == kTRUE) {
			fTextView->Clear();
			fTextView->ShowBottom();
			sprintf(this->char_TextView, "Internal Trigger Counters   \n");
			fTextView->AddLineFast(this->char_TextView);
			sprintf(this->char_TextView, "\n");
			fTextView->AddLineFast(this->char_TextView);
			sprintf(this->char_TextView, "            All       Hits/    Deadtime      Pileup        Veto    supressed\n");
			fTextView->AddLineFast(this->char_TextView);
			sprintf(this->char_TextView, "                     Events                                      High Energy \n");
			fTextView->AddLineFast(this->char_TextView);
			sprintf(this->char_TextView, "\n");
			fTextView->AddLineFast(this->char_TextView);
			for (i_fpga = 0; i_fpga < 4; i_fpga++) {
				gl_sis3316_adc1->read_Channel_StatisticCounter(i_fpga /* 0 to 3 */, uint_statistic_buffer); // new 27.08.2019

				for (i_ch = 0; i_ch < 4; i_ch++) {
					sprintf(this->char_TextView, "ch%2d ", (i_fpga * 4) + i_ch + 1);
					for (i = 0; i < 6; i++) {
						sprintf(this->char_TextView + strlen(this->char_TextView), "%10d  ", uint_statistic_buffer[(i_ch * 6) + i]);
					}
					//printf("\n");
					fTextView->AddLineFast(this->char_TextView);
				}
			}
			sprintf(this->char_TextView, "\n");
			fTextView->AddLineFast(this->char_TextView);
			fTextView->ShowBottom();
			fTextView->Update();

			//Resize(SIS3316_TEST_WINDOW_WIDTH, SIS3316_TEST_WINDOW_HIGH);   // resize to default size
			fTab->SetTab(9); // set active
		}




		/***********************   End of MULTI_EVENT_DOUBLE_BANK  Loop  **********************************************/
	} //if (uint_SampleControl_BankModus == 1)

	SIS3316_Test_running_dim_widgets(kTRUE);

}



/**********************************************************************************************************************************/

/**********************************************************************************************************************************/



#define FILE_FORMAT_EVENT_HEADER        	0xDEADBEEF  

int SIS3316_WriteBankChannelHeaderToDataFile (FILE *file_data_ptr, unsigned int indentifier, unsigned int bank_loop_no, unsigned int channel_no, unsigned int nof_events, unsigned int event_length, unsigned int short_and_maw_length, unsigned int reserved_EventBufferLen)
{   
int written ;
int data ;
  //header
	data = FILE_FORMAT_EVENT_HEADER ;    
    written=fwrite(&data,0x4,0x1, file_data_ptr); // write one  uint word
    written+=fwrite(&indentifier,0x4,0x1,file_data_ptr); // write one  uint word
    written+=fwrite(&bank_loop_no,0x4,0x1,file_data_ptr); // write one  uint word
    written+=fwrite(&channel_no,0x4,0x1,file_data_ptr); // write one  uint word
    written+=fwrite(&nof_events,0x4,0x1,file_data_ptr); // write one  uint word
    written+=fwrite(&event_length,0x4,0x1,file_data_ptr); // write one  uint word
    written+=fwrite(&short_and_maw_length,0x4,0x1,file_data_ptr); // write one  uint word
    written+=fwrite(&reserved_EventBufferLen,0x4,0x1,file_data_ptr); // write one  uint word
 	return written;

}


//--------------------------------------------------------------------------- 
int SIS3316_WriteBankChannelEventBufferToDataFile (FILE *file_data_ptr, unsigned int* memory_data_array, unsigned int nof_write_length_lwords)
{   
int nof_write_elements ;
int written ;
		nof_write_elements = nof_write_length_lwords ;
		written=fwrite(memory_data_array,0x4, nof_write_elements, file_data_ptr); //  
		//gl_uint_DataEvent_LWordCounter = gl_uint_DataEvent_LWordCounter + written  ;
		if(nof_write_elements != written) { 
    		printf ("Data File Write Error in  WriteEventToDataFile()  \n");
 		 }

 	return written;

}


/**
 * void setGuiParameters(sis3316_get_configuration_parametes *params)
 *
 * Sets Gui specific control element parameters to 
 * settings provided by supplied parameter set.
 * Picks only gui related parameters from set.
 *
 * @param params parameter set to set gui control elements to.
 */
void SIS3316TestDialog::setGuiParameters(sis3316_get_configuration_parameters *params) {

	/* stop after time enable */
	fChkStopAfterTime->SetState(params->uint_stop_after_time_seconds_enable ? kButtonDown : kButtonUp);
	/* stop after time seconds value */
	fNumericEntriesStopAfterTime->SetIntNumber(params->uint_stop_aftertime_seconds_value);
	if (params->uint_stop_after_time_seconds_enable == 0) {
		fNumericEntriesStopAfterTime->SetState(kFALSE); //
	}
	else {
		fNumericEntriesStopAfterTime->SetState(kTRUE); //
	}

	/* stop after loops enable */
	fChkStopAfterBanks->SetState(params->uint_stop_after_nof_loops_enable ? kButtonDown : kButtonUp);
	/* stop after nof loops value */
	fNumericEntriesStopAfterBanks->SetIntNumber(
		params->uint_stop_after_nof_loops_value
	);
	if(params->uint_stop_after_nof_loops_enable == 0) {
		fNumericEntriesStopAfterBanks->SetState(kFALSE); // dim
	}
	else {
		fNumericEntriesStopAfterBanks->SetState(kTRUE); // not dim
	}

	/* TEXT_PARAMETER_SAMPLE_CONTROL_MODE  */
	fCombo_SampleControl_BankModus->Select(params->uint_sample_control_mode, kTRUE); //   
	fChkNofEvents_ProBank->SetState(params->uint_sample_control_use_max_enable ? kButtonDown : kButtonUp);

	this->root_chk_bank_event_nof_limit_on_flag = params->uint_sample_control_use_max_enable;

	fNumericEntries_SampleControl_MaxNofEvents_ProBank->SetIntNumber(params->uint_sample_control_use_max_value);

	if (params->uint_sample_control_mode == 0) {
		fChkNofEvents_ProBank->SetState(kButtonUp); // is Off!
		fChkNofEvents_ProBank->SetEnabled(kFALSE); //  dim
		fNumericEntries_SampleControl_MaxNofEvents_ProBank->SetState(kFALSE); // dim
		fNumericEntries_SampleControl_MaxNofEvents_ProBank->SetIntNumber(1);
	}
	else {
		fChkNofEvents_ProBank->SetEnabled(kTRUE); // not dim
		fChkNofEvents_ProBank->SetState(this->root_chk_bank_event_nof_limit_on_flag ? kButtonDown : kButtonUp);
		if (this->root_chk_bank_event_nof_limit_on_flag == 0) {
			fNumericEntries_SampleControl_MaxNofEvents_ProBank->SetState(kFALSE); // dim
		}
		else {
			fNumericEntries_SampleControl_MaxNofEvents_ProBank->SetState(kTRUE); // not dim
		}
	}





	/* pre trigger delay */
	fNumericEntries_EventHitParameter[0]->SetIntNumber(
		params->uint_pre_trigger_delay
		);

	/* raw data sample index */
	fNumericEntries_EventHitParameter[1]->SetIntNumber(
		params->uint_raw_sample_start_index
		);
	/* raw data sample length */
	fNumericEntries_EventHitParameter[2]->SetIntNumber(	params->uint_raw_sample_length	);
	this->raw_sample_length = fNumericEntries_EventHitParameter[2]->GetIntNumber();


	/* MAW select */
	fCombo_SetSelectMAW_TestBuffer->Select(
		params->uint_maw_test_buffer_select_energy_flag, 
		kTRUE
		); //  Trigger
	/* MAW pre trigger delay */
	fNumericEntries_EventHitParameter[3]->SetIntNumber(
		params->uint_maw_test_buffer_delay
		);
	/* MAW start index */
	fNumericEntries_EventHitParameter[4]->SetIntNumber(
		params->uint_maw_test_start_index
		);
	/* MAW buffer length */
	fNumericEntries_EventHitParameter[5]->SetIntNumber(
		params->uint_maw_test_buffer_length
		);


	
	fChk_SuppressEventsIfAddrThresFlag->SetState(params->uint_SuppressEventsIfAddrThres_enable_flag ? kButtonDown : kButtonUp);


	/* Format enables */
	fChk_EventHitParameter_DataFormatBit0->SetState(params->uint_format_accum_enable_flag ? kButtonDown : kButtonUp	);
	fChk_EventHitParameter_DataFormatBit1->SetState(params->uint_format_accum78_enable_flag ? kButtonDown : kButtonUp);
	fChk_EventHitParameter_DataFormatBit2->SetState(params->uint_format_maw_enable_flag ? kButtonDown : kButtonUp);
	fChk_EventHitParameter_DataFormatBit3->SetState(params->uint_format_energy_enable_flag ? kButtonDown : kButtonUp);

	fChk_SaveRawDataFirstEventOnly->SetState(params->uint_SaveRawDataFirstEventOnly_enable_flag ? kButtonDown : kButtonUp);


	/* Sampling Trigger Conditions */
	fChkExternalTriggerFunc->SetState(params->uint_ext_trigger_function_as_extTrigger_enable_flag ? kButtonDown : kButtonUp	);

	fChkKeyTrigger->SetState(params->uint_trigger_cond_software_key_flag ? kButtonDown : kButtonUp);
	fChkLemoInTiEnable->SetState(params->uint_trigger_cond_vme_lemo_ti_flag ? kButtonDown : kButtonUp);

	fChkExternalTriggerDisableWithBusyEnable->SetState(params->uint_extTrig_disable_with_Busy_flag ? kButtonDown : kButtonUp);

	fChkFeedbackInternalTriggerEnable->SetState(params->uint_feedback_intTrig_as_extTrig_flag ? kButtonDown : kButtonUp	);



	/* Select Feedback */
	fChkFeedbackCoincidence1TriggerEnable->SetState(params->uint_Coincidence1FeedbackTrigger_enable ? kButtonDown : kButtonUp);

	for (int i = 0; i < SIS3316_CHANNEL_COUNT; i++) {
		fChkIntFeedbackTriggerEnableCh[SIS3316_CHANNEL_COUNT - 1 - i]->SetState(params->uint_IntFeedbackTrigger_enable[i] ? kButtonDown : kButtonUp	);
	}


//*******************************************************
	// External Veto/Gate
	fChkExternalTriggerFuncAsVeto->SetState(
		params->uint_ext_trigger_function_as_Veto_enable_flag ? kButtonDown : kButtonUp
	);
	fChkLocalVetoFuncAsVeto->SetState(
		params->uint_local_veto_function_as_Veto_enable_flag ? kButtonDown : kButtonUp
	);
	fChkLemoInUiAsVetoEnable->SetState(
		params->uint_LemoInUI_as_local_veto_function_enable_flag ? kButtonDown : kButtonUp
	);

	fNumericEntries_VetoDelay->SetIntNumber(
		params->uint_ext_Veto_delay
	);
	fNumericEntries_InternalDelay->SetIntNumber(
		params->uint_int_Trigger_delay[0]
	);


	 

//*******************************************************

	/* Select External Trigger */
	for (int i = 0; i < SIS3316_CHANNEL_COUNT; i++) {
		fChkExtTriggerEnableCh[SIS3316_CHANNEL_COUNT - 1 - i]->SetState(
			params->uint_ExtTrigger_enable[i] ? kButtonDown : kButtonUp
		);
	}

	/* Select Internal Trigger */
	for (int i = 0; i < SIS3316_CHANNEL_COUNT; i++) {
		fChkIntTriggerEnableCh[SIS3316_CHANNEL_COUNT - 1 - i]->SetState(
			params->uint_IntTrigger_enable[i] ? kButtonDown : kButtonUp
		);
	}

	/* Select Internal Sum Trigger */
	for (int i = 0; i < SIS3316_CHANNEL_COUNT; i++) {
		fChkIntSumTriggerEnableCh[SIS3316_CHANNEL_COUNT - 1 - i]->SetState(
			params->uint_SumTrigger_enable[i] ? kButtonDown : kButtonUp
		);
	}



	/* Select Internal Pileup Trigger */
	for (int i = 0; i < SIS3316_CHANNEL_COUNT; i++) {
		fChkIntPileupTriggerEnableCh[SIS3316_CHANNEL_COUNT - 1 - i]->SetState(
			params->uint_IntPileupTrigger_enable[i] ? kButtonDown : kButtonUp
		);
	}


	/* Select External Gate */
	for (int i = 0; i < SIS3316_CHANNEL_COUNT; i++) {
		fChkExtGateEnableCh[SIS3316_CHANNEL_COUNT - 1 - i]->SetState(
			params->uint_ExtGate_enable[i] ? kButtonDown : kButtonUp
		);
	}

	/* Select External Veto */
	for (int i = 0; i < SIS3316_CHANNEL_COUNT; i++) {
		fChkExtVetoEnableCh[SIS3316_CHANNEL_COUNT - 1 - i]->SetState(
			params->uint_ExtVeto_enable[i] ? kButtonDown : kButtonUp
		);
	}




	// display
	for (int i = 0; i < SIS3316_CHANNEL_COUNT; i++) {
		fChkDisplayAdc[i]->SetState(params->uint_DisplayChannel_enable[i] ? kButtonDown : kButtonUp);
	}

	fChkDisplayStatisticCounters->SetState(
		params->uint_DisplayStatisticCounter_enable ? kButtonDown : kButtonUp
	);


	fCombo_Display_MAW->Select(params->uint_DisplayMAW_select, kTRUE); //  
	fCombo_Display_Histos_Ch->Select(params->uint_DisplayHistogram_select, kTRUE); //  
	fCombo_Display_FFT_Ch->Select(params->uint_DisplayFFT_select, kTRUE); //  

	

 



}

/**
 * void setAdcParameters(sis3316_get_configuration_parametes *params)
 *
 * Sets Adc specific parameters to settings provided by supplied
 * parameter set.
 * Picks only Adc related parameters from set.
 */
void SIS3316TestDialog::setAdcParameters(sis3316_get_configuration_parameters *params) {
	
	/* set any known gui paramers to those supplied by the parameter set */
	/* flat approach: hard coded parameter dependencies */

	/* polarity */
	for (int i = 0; i < SIS3316_CHANNEL_COUNT; i++) {
		// reversed control element order
		fChkInvertChannel[SIS3316_CHANNEL_COUNT - 1 - i]->SetState(
			params->uint_channel_polarity_invert[i] ? kButtonDown : kButtonUp
		);
	}

	/* gain/offset */
	for (int i = 0; i < SIS3316_CHANNEL_COUNT; i++) {
		int elemIdx = SIS3316_CHANNEL_COUNT - 1 - i;
		/* termination */
		// reversed control element order
		fChkTerminationChannel[elemIdx]->SetState(
			params->uint_channel_50ohm_termination_disable[i] ? kButtonUp : kButtonDown
			);
		/* input range */
		// reversed control element order
		fChkInputRange0Channel[elemIdx]->SetState(
			params->uint_channel_range_2V[i] ? kButtonUp : kButtonDown
			);
		/* dac offset */
		// reversed control element order
		fNumericEntriesAnalogOffset[elemIdx]->SetIntNumber(
			params->uint_channel_adc_offset[i]
			);

	}

	/* trigger */
	/* take channel 0 value for control element */
	/* pulse length */
	fNumericEntriesTriggerPulse_length->SetIntNumber(
		params->uint_channel_trigger_pulse_length[0]
		);
	/* gap */
	fNumericEntriesTriggerGap->SetIntNumber(
		params->uint_channel_trigger_gap[0]
		);
	/* peaking */
	fNumericEntriesTriggerPeaking->SetIntNumber(
		params->uint_channel_trigger_peaking[0]
		);
	/* threshold */
	fNumericEntriesTriggerThreshold->SetIntNumber(
		params->uint_channel_trigger_threshold[0]
		);
	/* he threshold */
	fNumericEntriesHeTriggerThreshold->SetIntNumber(
		params->uint_channel_he_trigger_threshold[0]
		);
	/* he suppress */
	fChkTriggerHeSuppressMode->SetState(
		params->uint_channel_he_trigger_suppress ? kButtonDown : kButtonUp
		);
	/* cfd selector */
	fCombo_InternalTriggerCfdSelection->Select(
		params->uint_channel_he_trigger_generation_cfd_function_idx, 
		kTRUE
		);

	for (int i = 0; i < SIS3316_CHANNEL_COUNT; i++) {
		int elemIdx = SIS3316_CHANNEL_COUNT - 1 - i;
		/* trigger enable */
		// reversed control element order
		fChkTriggerEnableCh[elemIdx]->SetState(	params->uint_channel_trigger_enable[i] ? kButtonDown : kButtonUp);
	}

	/* sum enable */
	fChkTriggerEnableCh[16]->SetState(params->uint_sum_trigger_12_15 ? kButtonUp : kButtonDown);
	fChkTriggerEnableCh[17]->SetState(params->uint_sum_trigger_8_11 ? kButtonUp : kButtonDown);
	fChkTriggerEnableCh[18]->SetState(params->uint_sum_trigger_4_7 ? kButtonUp : kButtonDown);
	fChkTriggerEnableCh[19]->SetState(params->uint_sum_trigger_0_3 ? kButtonUp : kButtonDown);

	/* pileup */
	fNumericEntriesPileup_length->SetIntNumber(	params->uint_pileup_window_length);
	fNumericEntriesRepileup_length->SetIntNumber(
		params->uint_re_pileup_window_length
		);

	/* trigger to vme */
	fCombo_InternalTriggerToVMESelection->Select(
		params->uint_vme_trigger_idx, 
		kTRUE
		);
	fCombo_InternalHeTriggerToVMESelection->Select(
		params->uint_vme_he_trigger_idx, 
		kTRUE
		);

	/* energy */
	fNumericEntriesEnergyPeaking->SetIntNumber(
		params->uint_energy_peaking_value
		);
	fNumericEntriesEnergyGap->SetIntNumber(
		params->uint_energy_gap_value
		);
	fNumericEntriesEnergyTauTable->SetIntNumber(
		params->uint_energy_decay_tau_table
		);
	fNumericEntriesEnergyTauFactor->SetIntNumber(
		params->uint_energy_decay_tau_factor
		);
	fNumericEntriesEnergyAdditionalAverage->SetIntNumber(
		params->uint_energy_average_factor
		);
	fNumericEntriesEnergyPickupValueIndex->SetIntNumber(
		params->uint_energy_pickup_idx
		);

	fNumericEntriesAccumulatorStartIndex[0]->SetIntNumber(
		params->uint_gate1_start_index
		);
	fNumericEntriesAccumulatorLength[0]->SetIntNumber(
		params->uint_gate1_length
		);
	fNumericEntriesAccumulatorStartIndex[1]->SetIntNumber(
		params->uint_gate2_start_index
		);
	fNumericEntriesAccumulatorLength[1]->SetIntNumber(
		params->uint_gate2_length
		);
	fNumericEntriesAccumulatorStartIndex[2]->SetIntNumber(
		params->uint_gate3_start_index
		);
	fNumericEntriesAccumulatorLength[2]->SetIntNumber(
		params->uint_gate3_length
		);
	fNumericEntriesAccumulatorStartIndex[3]->SetIntNumber(
		params->uint_gate4_start_index
		);
	fNumericEntriesAccumulatorLength[3]->SetIntNumber(
		params->uint_gate4_length
		);
	fNumericEntriesAccumulatorStartIndex[4]->SetIntNumber(
		params->uint_gate5_start_index
		);
	fNumericEntriesAccumulatorLength[4]->SetIntNumber(
		params->uint_gate5_length
		);
	fNumericEntriesAccumulatorStartIndex[5]->SetIntNumber(
		params->uint_gate6_start_index
		);
	fNumericEntriesAccumulatorLength[5]->SetIntNumber(
		params->uint_gate6_length
		);


	fNumericEntriesAccumulatorStartIndex[6]->SetIntNumber(
		params->uint_gate7_start_index
	);
	fNumericEntriesAccumulatorLength[6]->SetIntNumber(
		params->uint_gate7_length
	);

	fNumericEntriesAccumulatorStartIndex[7]->SetIntNumber(
		params->uint_gate8_start_index
	);
	fNumericEntriesAccumulatorLength[7]->SetIntNumber(
		params->uint_gate8_length
	);



	/* sample clock */
	fCombo_SetInternalClockFreq->Select(params->uint_internal_sample_clock_idx, kTRUE);
	fChkFP_BUS_ClockMaster->SetState(params->uint_internal_sample_clock_fp_en ? kButtonDown : kButtonUp	);
	fCombo_FP_BUS_ClockOutMux->Select(	params->uint_fp_clock_idx,	kTRUE);
	fCombo_SampleClock_source->Select(	params->uint_sample_clock_idx, 	kTRUE);
	fCombo_SetClockMultiplierMode->Select(	params->uint_multiplier_idx, kTRUE	);

	fCombo_CoincidenceLookupTableMode->Select(params->uint_CoincidenceLookupMode_idx, kTRUE);


	/* lemo out CO */
	fNumericEntriesNimOutput[0]->SetHexNumber(	params->uint_lemo_out_CO_select);
	for (int i = 0; i < 32; i++) {
		if ( ( (params->uint_lemo_out_CO_select >> i) & 0x1) == 1) {
			fChkLemoOutCoEnableCh[i]->SetState(kButtonDown); // is ON !
		}
		else {
			fChkLemoOutCoEnableCh[i]->SetState(kButtonUp); // is OFF !
		}
	}

	/* lemo out TO */
	fNumericEntriesNimOutput[1]->SetHexNumber(params->uint_lemo_out_TO_select);
	for (int i = 0; i < 32; i++) {
		if (((params->uint_lemo_out_TO_select >> i) & 0x1) == 1) {
			fChkLemoOutToEnableCh[i]->SetState(kButtonDown); // is ON !
		}
		else {
			fChkLemoOutToEnableCh[i]->SetState(kButtonUp); // is OFF !
		}
	}

		
	/* lemo out UO */
	fNumericEntriesNimOutput[2]->SetHexNumber(params->uint_lemo_out_UO_select);
	for (int i = 0; i < 32; i++) {
		if (((params->uint_lemo_out_UO_select >> i) & 0x1) == 1) {
			fChkLemoOutUoEnableCh[i]->SetState(kButtonDown); // is ON !
		}
		else {
			fChkLemoOutUoEnableCh[i]->SetState(kButtonUp); // is OFF !
		}
	}

}

/**
 * void getGuiParameters(sis3316_get_configuration_parametes *params)
 *
 * Gets Gui specific parameters from control elements.
 * Picks only gui related parameters from set.
 *
 * @param params parameter set to store gui control elements to.
 */
void SIS3316TestDialog::getGuiParameters(sis3316_get_configuration_parameters *params) {



	/* stop after time enable */
	params->uint_stop_after_time_seconds_enable = fChkStopAfterTime->GetState() == kButtonDown ? 1 : 0;
	/* stop after time seconds value */
	params->uint_stop_aftertime_seconds_value = fNumericEntriesStopAfterTime->GetIntNumber();


	/* stop after loops enable */
	params->uint_stop_after_nof_loops_enable = 
		fChkStopAfterBanks->GetState() == kButtonDown ? 1 : 0;
	/* stop after nof loops value */
	params->uint_stop_after_nof_loops_value = 
		fNumericEntriesStopAfterBanks->GetIntNumber();

	/* TEXT_PARAMETER_SAMPLE_CONTROL_MODE select */
	params->uint_sample_control_mode = 
		fCombo_SampleControl_BankModus->GetSelected();
	/* TEXT_PARAMETER_SAMPLE_CONTROL_USE_MAX_ENABLE enable */
	//params->uint_sample_control_use_max_enable = 	fChkNofEvents_ProBank->GetState() == kButtonDown ? 1 : 0;
	params->uint_sample_control_use_max_enable = this->root_chk_bank_event_nof_limit_on_flag;


	/* TEXT_PARAMETER_SAMPLE_CONTROL_USE_MAX_VALUE */
	params->uint_sample_control_use_max_value = 
		fNumericEntries_SampleControl_MaxNofEvents_ProBank->GetIntNumber();




	/* pre trigger delay */
	params->uint_pre_trigger_delay = 
		fNumericEntries_EventHitParameter[0]->GetIntNumber();
	/* raw data sample index */
	params->uint_raw_sample_start_index =
		fNumericEntries_EventHitParameter[1]->GetIntNumber();
	/* raw data sample length */
	params->uint_raw_sample_length = 
		fNumericEntries_EventHitParameter[2]->GetIntNumber();

	/* MAW select */
	params->uint_maw_test_buffer_select_energy_flag = 	fCombo_SetSelectMAW_TestBuffer->GetSelected();
	/* MAW pre trigger delay */
	params->uint_maw_test_buffer_delay  = 	fNumericEntries_EventHitParameter[3]->GetIntNumber();
	/* MAW start index */
	params->uint_maw_test_start_index   =		fNumericEntries_EventHitParameter[4]->GetIntNumber();
	/* MAW buffer length */
	params->uint_maw_test_buffer_length =		fNumericEntries_EventHitParameter[5]->GetIntNumber();


	params->uint_SuppressEventsIfAddrThres_enable_flag = fChk_SuppressEventsIfAddrThresFlag->GetState() == kButtonDown ? 1 : 0;

	/* Format enables */
	params->uint_format_accum_enable_flag   =		fChk_EventHitParameter_DataFormatBit0->GetState() == kButtonDown ? 1 : 0;
	params->uint_format_accum78_enable_flag =		fChk_EventHitParameter_DataFormatBit1->GetState() == kButtonDown ? 1 : 0;
	params->uint_format_maw_enable_flag     =		fChk_EventHitParameter_DataFormatBit2->GetState() == kButtonDown ? 1 : 0;
	params->uint_format_energy_enable_flag  =		fChk_EventHitParameter_DataFormatBit3->GetState() == kButtonDown ? 1 : 0;

	params->uint_SaveRawDataFirstEventOnly_enable_flag = fChk_SaveRawDataFirstEventOnly->GetState() == kButtonDown ? 1 : 0;



	/* Sampling Trigger Conditions */

	params->uint_ext_trigger_function_as_extTrigger_enable_flag =
		fChkExternalTriggerFunc->GetState() == kButtonDown ? 1 : 0;

	params->uint_trigger_cond_software_key_flag =
		fChkKeyTrigger->GetState() == kButtonDown ? 1 : 0;
	params->uint_trigger_cond_vme_lemo_ti_flag =
		fChkLemoInTiEnable->GetState() == kButtonDown ? 1 : 0;

	params->uint_extTrig_disable_with_Busy_flag =
		fChkExternalTriggerDisableWithBusyEnable->GetState() == kButtonDown ? 1 : 0;

	params->uint_feedback_intTrig_as_extTrig_flag =
		fChkFeedbackInternalTriggerEnable->GetState() == kButtonDown ? 1 : 0;




	/* Select Feedback */
	params->uint_Coincidence1FeedbackTrigger_enable = fChkFeedbackCoincidence1TriggerEnable->GetState() == kButtonDown ? 1 : 0;
	for (int i = 0; i < SIS3316_CHANNEL_COUNT; i++) {
		// reversed control element order
		params->uint_IntFeedbackTrigger_enable[i] =	fChkIntFeedbackTriggerEnableCh[SIS3316_CHANNEL_COUNT - 1 - i]->GetState() == kButtonDown ? 1 : 0;
	}


	
	//*******************************************************
	// External Veto/Gate
	params->uint_ext_trigger_function_as_Veto_enable_flag    =	  fChkExternalTriggerFuncAsVeto->GetState() == kButtonDown ? 1 : 0;
	params->uint_local_veto_function_as_Veto_enable_flag     =    fChkLocalVetoFuncAsVeto->GetState() == kButtonDown ? 1 : 0;
	params->uint_LemoInUI_as_local_veto_function_enable_flag =    fChkLemoInUiAsVetoEnable->GetState() == kButtonDown ? 1 : 0;


	params->uint_ext_Veto_delay = fNumericEntries_VetoDelay->GetIntNumber();

	for (int i = 0; i < SIS3316_CHANNEL_COUNT; i++) {
		params->uint_int_Trigger_delay[i] =    fNumericEntries_InternalDelay->GetIntNumber();
	}




//*****************

	/* Select External Trigger */
	for (int i = 0; i < SIS3316_CHANNEL_COUNT; i++) {
		// reversed control element order
		params->uint_ExtTrigger_enable[i] =
			fChkExtTriggerEnableCh[SIS3316_CHANNEL_COUNT - 1 - i]->GetState() == kButtonDown ? 1 : 0;
	}

	/* Select Internal Trigger */
	for (int i = 0; i < SIS3316_CHANNEL_COUNT; i++) {
		// reversed control element order
		params->uint_IntTrigger_enable[i] =
			fChkIntTriggerEnableCh[SIS3316_CHANNEL_COUNT - 1 - i]->GetState() == kButtonDown ? 1 : 0;
	}


	/* Select Internal Sum Trigger */
	for (int i = 0; i < SIS3316_CHANNEL_COUNT; i++) {
		// reversed control element order
		params->uint_SumTrigger_enable[i] =
			fChkIntSumTriggerEnableCh[SIS3316_CHANNEL_COUNT - 1 - i]->GetState() == kButtonDown ? 1 : 0;
	}


 


	/* Select Internal Pileup Trigger */
	for (int i = 0; i < SIS3316_CHANNEL_COUNT; i++) {
		// reversed control element order
		params->uint_IntPileupTrigger_enable[i] =
			fChkIntPileupTriggerEnableCh[SIS3316_CHANNEL_COUNT - 1 - i]->GetState() == kButtonDown ? 1 : 0;
	}

	/* Select External Gate */
	for (int i = 0; i < SIS3316_CHANNEL_COUNT; i++) {
		// reversed control element order
		params->uint_ExtGate_enable[i] =
			fChkExtGateEnableCh[SIS3316_CHANNEL_COUNT - 1 - i]->GetState() == kButtonDown ? 1 : 0;
	}

	/* Select External Veto */
	for (int i = 0; i < SIS3316_CHANNEL_COUNT; i++) {
		// reversed control element order
		params->uint_ExtVeto_enable[i] =
			fChkExtVetoEnableCh[SIS3316_CHANNEL_COUNT - 1 - i]->GetState() == kButtonDown ? 1 : 0;
	}



	// display
	for (int i = 0; i < SIS3316_CHANNEL_COUNT; i++) {
		params->uint_DisplayChannel_enable[i] = fChkDisplayAdc[i]->GetState() == kButtonDown ? 1 : 0;
	}

	params->uint_DisplayStatisticCounter_enable = fChkDisplayStatisticCounters->GetState() == kButtonDown ? 1 : 0;



	params->uint_DisplayMAW_select       = fCombo_Display_MAW->GetSelected();
	params->uint_DisplayHistogram_select = fCombo_Display_Histos_Ch->GetSelected();
	params->uint_DisplayFFT_select       = fCombo_Display_FFT_Ch->GetSelected();




}

/**
 * void getAdcParameters(sis3316_get_configuration_parametes *params)
 *
 * Gets Adc specific parameters from gui controle elements.
 * Picks only Adc related parameters from set.
 *
 * @param params parameter set to store gui control elements to.
 */
void SIS3316TestDialog::getAdcParameters(sis3316_get_configuration_parameters *params) {

	/* polarity */
	for (int i = 0; i < SIS3316_CHANNEL_COUNT; i++) {
		// reversed control element order
		params->uint_channel_polarity_invert[i] = 
			fChkInvertChannel[SIS3316_CHANNEL_COUNT - 1 - i]->GetState() == kButtonDown ? 1 : 0;
	}

	/* gain/offset */
	for (int i = 0; i < SIS3316_CHANNEL_COUNT; i++) {
		int elemIdx = SIS3316_CHANNEL_COUNT - 1 - i;
		/* termination */
		// reversed control element order
		params->uint_channel_50ohm_termination_disable[i] = 
			fChkTerminationChannel[elemIdx]->GetState() == kButtonUp ? 1 : 0;
		/* input range */
		// reversed control element order
		params->uint_channel_range_2V[i] = 
			fChkInputRange0Channel[elemIdx]->GetState() == kButtonUp ? 1 : 0;
		/* dac offset */
		// reversed control element order
		params->uint_channel_adc_offset[i] = 
			fNumericEntriesAnalogOffset[elemIdx]->GetIntNumber();
	}

	/* trigger */
	/* take channel 0 value for control element */
	/* pulse length */
	params->uint_channel_trigger_pulse_length[0] = 
		fNumericEntriesTriggerPulse_length->GetIntNumber();
	/* gap */
	params->uint_channel_trigger_gap[0] = 
		fNumericEntriesTriggerGap->GetIntNumber();
	/* peaking */
	params->uint_channel_trigger_peaking[0] = 
		fNumericEntriesTriggerPeaking->GetIntNumber();
	/* threshold */
	params->uint_channel_trigger_threshold[0] = 
		fNumericEntriesTriggerThreshold->GetIntNumber();
	/* he threshold */
	params->uint_channel_he_trigger_threshold[0] = 
		fNumericEntriesHeTriggerThreshold->GetIntNumber();
	/* he suppress */
	params->uint_channel_he_trigger_suppress = 
		fChkTriggerHeSuppressMode->GetState() == kButtonDown ? 1 : 0;
	/* cfd selector */
	params->uint_channel_he_trigger_generation_cfd_function_idx = 
		fCombo_InternalTriggerCfdSelection->GetSelected();

	for (int i = 0; i < SIS3316_CHANNEL_COUNT; i++) {
		int elemIdx = SIS3316_CHANNEL_COUNT - 1 - i;
		/* trigger enable */
		// reversed control element order
		params->uint_channel_trigger_enable[i] = 	fChkTriggerEnableCh[elemIdx]->GetState() == kButtonDown ? 1 : 0;
	}


	/* sum enable */
	params->uint_sum_trigger_12_15 = 	fChkTriggerEnableCh[16]->GetState() == kButtonDown ? 1 : 0;
	params->uint_sum_trigger_8_11  =	fChkTriggerEnableCh[17]->GetState() == kButtonDown ? 1 : 0;
	params->uint_sum_trigger_4_7   =	fChkTriggerEnableCh[18]->GetState() == kButtonDown ? 1 : 0;
	params->uint_sum_trigger_0_3   =	fChkTriggerEnableCh[19]->GetState() == kButtonDown ? 1 : 0;

	/* pileup */
	params->uint_pileup_window_length = 
		fNumericEntriesPileup_length->GetIntNumber();
	params->uint_re_pileup_window_length = 
		fNumericEntriesRepileup_length->GetIntNumber();

	/* trigger to vme */
	params->uint_vme_trigger_idx = 
		fCombo_InternalTriggerToVMESelection->GetSelected();
	params->uint_vme_he_trigger_idx = 
		fCombo_InternalHeTriggerToVMESelection->GetSelected();

	/* energy */
	params->uint_energy_peaking_value = 
		fNumericEntriesEnergyPeaking->GetIntNumber();
	params->uint_energy_gap_value = 
		fNumericEntriesEnergyGap->GetIntNumber();
	params->uint_energy_decay_tau_table = 
		fNumericEntriesEnergyTauTable->GetIntNumber();
	params->uint_energy_decay_tau_factor = 
		fNumericEntriesEnergyTauFactor->GetIntNumber();
	params->uint_energy_average_factor = 
		fNumericEntriesEnergyAdditionalAverage->GetIntNumber();
	params->uint_energy_pickup_idx = 
		fNumericEntriesEnergyPickupValueIndex->GetIntNumber();

	params->uint_gate1_start_index = 
		fNumericEntriesAccumulatorStartIndex[0]->GetIntNumber();
	params->uint_gate1_length = 
		fNumericEntriesAccumulatorLength[0]->GetIntNumber();
	params->uint_gate2_start_index =
		fNumericEntriesAccumulatorStartIndex[1]->GetIntNumber();
	params->uint_gate2_length = 
		fNumericEntriesAccumulatorLength[1]->GetIntNumber();
	params->uint_gate3_start_index = 
		fNumericEntriesAccumulatorStartIndex[2]->GetIntNumber();
	params->uint_gate3_length = 
		fNumericEntriesAccumulatorLength[2]->GetIntNumber();
	params->uint_gate4_start_index =
		fNumericEntriesAccumulatorStartIndex[3]->GetIntNumber();
	params->uint_gate4_length =
		fNumericEntriesAccumulatorLength[3]->GetIntNumber();
	params->uint_gate5_start_index =
		fNumericEntriesAccumulatorStartIndex[4]->GetIntNumber();
	params->uint_gate5_length = 
		fNumericEntriesAccumulatorLength[4]->GetIntNumber();
	params->uint_gate6_start_index =
		fNumericEntriesAccumulatorStartIndex[5]->GetIntNumber();
	params->uint_gate6_length = 
		fNumericEntriesAccumulatorLength[5]->GetIntNumber();

	params->uint_gate7_start_index =
		fNumericEntriesAccumulatorStartIndex[6]->GetIntNumber();
	params->uint_gate7_length =
		fNumericEntriesAccumulatorLength[6]->GetIntNumber();


	params->uint_gate8_start_index =
		fNumericEntriesAccumulatorStartIndex[7]->GetIntNumber();
	params->uint_gate8_length =
		fNumericEntriesAccumulatorLength[7]->GetIntNumber();




	/* sample clock */
	params->uint_internal_sample_clock_idx   = 	fCombo_SetInternalClockFreq->GetSelected();
	params->uint_internal_sample_clock_fp_en =	fChkFP_BUS_ClockMaster->GetState() == kButtonDown ? 1 : 0;
	params->uint_fp_clock_idx =          		fCombo_FP_BUS_ClockOutMux->GetSelected();
	params->uint_sample_clock_idx =      		fCombo_SampleClock_source->GetSelected();
	params->uint_multiplier_idx = fCombo_SetClockMultiplierMode->GetSelected();
	params->uint_CoincidenceLookupMode_idx = fCombo_CoincidenceLookupTableMode->GetSelected();




	/* lemo out */
	params->uint_lemo_out_CO_select = 	fNumericEntriesNimOutput[0]->GetHexNumber();
	params->uint_lemo_out_TO_select =	fNumericEntriesNimOutput[1]->GetHexNumber();
	params->uint_lemo_out_UO_select =	fNumericEntriesNimOutput[2]->GetHexNumber();
}


/********************************************************************************************************************************/

void SIS3316TestDialog::LoadConfigurationFile()
{
	static const char *gDefTypes[] = { "configuration files", "*.ini",
									   "all files",    "*",
                                        0,             0 };

	fileInfoConfFile.fFileTypes = gDefTypes;
	new TGFileDialog(gClient->GetRoot(), this, kFDOpen, &fileInfoConfFile);
	if(fileInfoConfFile.fFilename) {
		/* change gui/adc settings upon succesful parsing */
		if (!params->read_parameter_file(fileInfoConfFile.fFilename)) {
			setGuiParameters(params);
			setAdcParameters(params);
			this->SIS3316_Test_Update_Gui_Entries();
			this->SIS3316_Test_Calculate_MaxNofEventsEachBank();
		}
	} 
}

/********************************************************************************************************************************/


void SIS3316TestDialog::SaveConfigurationFile()
{

	static const char *gDefTypes[] = { "configuration files", "*.ini",
									   "all files",    "*",
                                        0,             0 };

	fileInfoConfFile.fFileTypes = gDefTypes;
	new TGFileDialog(gClient->GetRoot(), this, kFDSave, &fileInfoConfFile);
	if(fileInfoConfFile.fFilename) {
		getAdcParameters(params);
		getGuiParameters(params);
		params->write_parameter_file(fileInfoConfFile.fFilename);
	}
}

/********************************************************************************************************************************/

void SIS3316TestDialog::CloseWindow()
{
	fSIS3316_Test1_Run_Cmd = kFALSE;
    DeleteWindow(); 
}


/********************************************************************************************************************************/

Bool_t SIS3316TestDialog::ProcessMessage(Long_t msg, Long_t parm1, Long_t)
{
   // Process messages coming from widgets associated with the dialog.

int i;
unsigned int  adc_input_voltage_select;
unsigned int  data;
unsigned int  i_adc_fpga_group, i_adc_chip;
unsigned int  i_adc_fpga;


unsigned int energy_peaking_val, old_energy_peaking_val ;
unsigned int energy_gap_val, old_energy_gap_val ;
unsigned int energy_decay_tau_factor_val, old_energy_decay_tau_factor_val ;
unsigned int energy_decay_tau_table_val, old_energy_decay_tau_table_val ;
unsigned int energy_additional_average_val, old_energy_additional_average_val ;

static const char *dataFilePath_gDefTypes[] = { "binary files", "*.dat",
//									   "text files",    "*.txt",
                                        0,             0 };


//    char tmp[20];
//   static int newtab = 0;
   Pixel_t green;
   fClient->GetColorByName("green", green);
   Pixel_t red;
   fClient->GetColorByName("red", red);

   Pixel_t yellow;
   fClient->GetColorByName("yellow", yellow);

   //printf("\nSIS3316TestDialog::ProcessMessage:case kC_COMMAND;kCM_BUTTON \n");

 
   switch (GET_MSG(msg)) {

		case kC_COMMAND:
          switch (GET_SUBMSG(msg)) {

/********************************************/
            case kCM_MENU:
               //printf("kCM_MENU id=%ld\n", parm1);
               switch (parm1) {

                  case TEST1_FILE_EXIT:
					 fSIS3316_Test1_Run_Cmd = kFALSE;
                     CloseWindow();   // this also terminates theApp
                     break;


				  case M_LOAD_CONFIGURATION_DLG:
					 LoadConfigurationFile();   //  
                     break;
					 
				  case M_SAVE_CONFIGURATION_DLG:
                     SaveConfigurationFile();   //  
                     break;
					 

                  default:
                     break;
               } // kCM_MENU switch (parm1)
               break;

/********************************************/
 

            case kCM_TAB:
				// printf("kCM_TAB id=%ld\n", parm1);

				 if((unsigned int)parm1 < (unsigned int)this->sis3316Test1_nof_valid_tabel_tabs) {
					for(i=0; (unsigned int)i<this->sis3316Test1_nof_valid_tabel_tabs;i++) {
						tabel_tab[i]->ChangeBackground(tab_color_not_active);
					}
					tabel_tab[parm1]->ChangeBackground(tab_color_active);
					if (*fBTest1_Run_Busy == kFALSE) {
						Resize(SIS3316_TEST_WINDOW_WIDTH, SIS3316_TEST_WINDOW_HIGH);   // resize to default size
					}
				 }


			   break;

/********************************************/



			case kCM_BUTTON:
		//printf("\nSIS3316TestDialog::ProcessMessage:case kC_COMMAND;kCM_BUTTON \n");
               switch(parm1) {
		
				//case 1:
                  //case 2:
                     //printf("\nTerminating dialog: %s pressed\n", (parm1 == 1) ? "Quit" : "Cancel");
                  //   fSIS3316_Test1_Run_Cmd = kFALSE;
                     CloseWindow();
                   //  break;
				  //case 4:  // start test
				  case SIS3316TestDialog_kCM_BUTTON_IRQ_NO_4:  // start test
					  if(fSIS3316_Test1_Run_Cmd != kTRUE) { // start only if not running
						if (fCombo_SampleControl_BankModus->GetSelected() == 0) {
							Resize(SIS3316_TEST_WINDOW_WIDTH_RUN_SINGLE, SIS3316_TEST_WINDOW_HIGH);   // resize to Single Run Size
						}
						else {
							  Resize(SIS3316_TEST_WINDOW_WIDTH_RUN_MULTI, SIS3316_TEST_WINDOW_HIGH);   // resize to Multi Run Size						  
						}
						fStartB->ChangeBackground(red);
						fStopB->ChangeBackground(green);
						fSIS3316_Test1_Run_Cmd = kTRUE;
						*fBTest1_Run_Busy = kTRUE;
			
						printf("\nstart Test1\n");
						SIS3316_Test1();
						*fBTest1_Run_Busy = kFALSE;
						printf("stop Test1\n");
					    fStartB->ChangeBackground(green);
						fStopB->ChangeBackground(red);
						//Resize(SIS3316_TEST_WINDOW_WIDTH, SIS3316_TEST_WINDOW_HIGH);   // resize to default size
					  }
					  break;
                  //case 5:  //  stop test
				  case SIS3316TestDialog_kCM_BUTTON_IRQ_NO_5:  //  
                     fSIS3316_Test1_Run_Cmd = kFALSE;
					 // fStartB->ChangeBackground(green);
					  fStopB->ChangeBackground(red);
					  //Resize(SIS3316_TEST_WINDOW_WIDTH, SIS3316_TEST_WINDOW_HIGH);   // resize to default size
                     break;

				  case SIS3316TestDialog_kCM_BUTTON_IRQ_NO_6:  //  
						SIS3316_Test_Write_Sample_Clock_Configuration() ;
						fStartB->SetEnabled(kTRUE); // dim
						fStartB->ChangeBackground(green);
						fStopB->SetEnabled(kTRUE); // dim
						fClockConfiguration->SetEnabled(kFALSE); // not dim
						fClockConfiguration->ChangeBackground(this->fClockConfiguration_background_color);
						fTab->SetTab(0) ; // set active
						Resize(SIS3316_TEST_WINDOW_WIDTH, SIS3316_TEST_WINDOW_HIGH);   // resize to default size

                     break;
	
					  
				  case SIS3316TestDialog_kCM_BUTTON_EXT_IRQ_NO_10:
					  for (i = 0; i < 16; i++) {
						  fChkExtTriggerEnableCh[i]->SetState(kButtonDown); // is ON !
					  }
					  break;
				  case SIS3316TestDialog_kCM_BUTTON_EXT_IRQ_NO_11:
					  for (i = 0; i < 16; i++) {
						  fChkExtTriggerEnableCh[i]->SetState(kButtonUp); // is OFF !
					  }
					  break;



				  case SIS3316TestDialog_kCM_BUTTON_INT_IRQ_NO_12:
					  for (i = 0; i < 16; i++) {
						  fChkIntTriggerEnableCh[i]->SetState(kButtonDown); // is ON !
					  }
					  break;
				  case SIS3316TestDialog_kCM_BUTTON_INT_IRQ_NO_13:
					  for (i = 0; i < 16; i++) {
						  fChkIntTriggerEnableCh[i]->SetState(kButtonUp); // is OFF !
					  }
					  break;

				  case SIS3316TestDialog_kCM_BUTTON_INTSUM_IRQ_NO_14:
					  for (i = 0; i < 16; i++) {
						  fChkIntSumTriggerEnableCh[i]->SetState(kButtonDown); // is ON !
					  }
					  break;
				  case SIS3316TestDialog_kCM_BUTTON_INTSUM_IRQ_NO_15:
					  for (i = 0; i < 16; i++) {
						  fChkIntSumTriggerEnableCh[i]->SetState(kButtonUp); // is OFF !
					  }
					  break;

				  case SIS3316TestDialog_kCM_BUTTON_INTPILE_IRQ_NO_16:
					  for (i = 0; i < 16; i++) {
						  fChkIntPileupTriggerEnableCh[i]->SetState(kButtonDown); // is ON !
					  }
					  break;
				  case SIS3316TestDialog_kCM_BUTTON_INTPILE_IRQ_NO_17:
					  for (i = 0; i < 16; i++) {
						  fChkIntPileupTriggerEnableCh[i]->SetState(kButtonUp); // is OFF !
					  }
					  break;
 

				  case SIS3316TestDialog_kCM_BUTTON_INTFEEDBACK_IRQ_NO_18:
					  for (i = 0; i < 16; i++) {
						  fChkIntFeedbackTriggerEnableCh[i]->SetState(kButtonDown); // is ON !
					  }
					  break;
				  case SIS3316TestDialog_kCM_BUTTON_INTFEEDBACK_IRQ_NO_19:
					  for (i = 0; i < 16; i++) {
						  fChkIntFeedbackTriggerEnableCh[i]->SetState(kButtonUp); // is OFF !
					  }
					  break;



				  case SIS3316TestDialog_kCM_BUTTON_EXTGATE_IRQ_NO_20:
					  for (i = 0; i < 16; i++) {
						  fChkExtGateEnableCh[i]->SetState(kButtonDown); // is ON !
					  }
					  break;
				  case SIS3316TestDialog_kCM_BUTTON_EXTGATE_IRQ_NO_21:
					  for (i = 0; i < 16; i++) {
						  fChkExtGateEnableCh[i]->SetState(kButtonUp); // is OFF !
					  }
					  break;



				  case SIS3316TestDialog_kCM_BUTTON_EXTVETO_IRQ_NO_22:
					  for (i = 0; i < 16; i++) {
						  fChkExtVetoEnableCh[i]->SetState(kButtonDown); // is ON !
					  }
					  break;
				  case SIS3316TestDialog_kCM_BUTTON_EXTVETO_IRQ_NO_23:
					  for (i = 0; i < 16; i++) {
						  fChkExtVetoEnableCh[i]->SetState(kButtonUp); // is OFF !
					  }
					  break;


 


 
				  case SIS3316TestDialog_kCM_BUTTON_IRQ_NO_30:
						for (i = 0; i < 16; i++) {
							fChkTriggerEnableCh[i]->SetState(kButtonDown)   ; // is ON !
						}
						//printf("\n pressed 60\n");
						break;
				  case SIS3316TestDialog_kCM_BUTTON_IRQ_NO_31:
						for (i = 0; i < 16; i++) {
							fChkTriggerEnableCh[i]->SetState(kButtonUp)   ; // is OFF !
						}
						break;





				  case SIS3316TestDialog_kCM_BUTTON_IRQ_NO_42:  //  
						fileInfoDataFile.fFileTypes = dataFilePath_gDefTypes;
						new TGFileDialog(gClient->GetRoot(), this, kFDOpen, &fileInfoDataFile);
						if(fileInfoDataFile.fFilename){
							fTextEntryDataFilePath->SetText(fileInfoDataFile.fFilename);
						}else{
							fTextEntryDataFilePath->SetText("sis3316_test_data.dat");
						}


						break;





				  case 50:
						for (i = 0; i < 16; i++) {
							fChkDisplayAdc[i]->SetState(kButtonDown)   ; // is ON !
						}
						//printf("\n pressed 50\n");
						break;
				  case 51:
						for (i = 0; i < 16; i++) {
							fChkDisplayAdc[i]->SetState(kButtonUp)   ; // is OFF !
						}
						break;




				  case SIS3316TestDialog_kCM_BUTTON_IRQ_NO_70:
						for (i = 0; i < 16; i++) {
							fChkInvertChannel[i]->SetState(kButtonDown)   ; // is ON !
						}
						invert_parameter_has_changed_flag = 1 ;
						//printf("\n pressed 60\n");
						break;
				  case SIS3316TestDialog_kCM_BUTTON_IRQ_NO_71:
						for (i = 0; i < 16; i++) {
							fChkInvertChannel[i]->SetState(kButtonUp)   ; // is OFF !
						}
						invert_parameter_has_changed_flag = 1 ;
						break;


				  case SIS3316TestDialog_kCM_BUTTON_IRQ_NO_80:
						for (i = 0; i < 16; i++) {
							fChkTerminationChannel[i]->SetState(kButtonDown)   ; // is ON !
						}
						gain_termination_parameter_has_changed_flag = 1 ;
						break;

				  case SIS3316TestDialog_kCM_BUTTON_IRQ_NO_81:
						for (i = 0; i < 16; i++) {
							fChkTerminationChannel[i]->SetState(kButtonUp)   ; // is OFF !
						}
						gain_termination_parameter_has_changed_flag = 1 ;
						break;


				  case SIS3316TestDialog_kCM_BUTTON_IRQ_NO_82:
						for (i = 0; i < 16; i++) {
							fChkInputRange0Channel[i]->SetState(kButtonDown)   ; // is ON !
						}
						gain_termination_parameter_has_changed_flag = 1 ;
						break;

				  case SIS3316TestDialog_kCM_BUTTON_IRQ_NO_83:
						for (i = 0; i < 16; i++) {
							fChkInputRange0Channel[i]->SetState(kButtonUp)   ; // is OFF !
						}
						gain_termination_parameter_has_changed_flag = 1 ;
						break;


				  default:
                     break;






               } //switch(parm1)
               break; //case kCM_BUTTON
#ifdef not_used
            case kCM_RADIOBUTTON:
               switch (parm1) {
                  case 81:
                     break;
                  case 82:
                     break;
                  default:
                     break;
               }
               break;
#endif

	    case kCM_CHECKBUTTON:
	      //printf("\nSIS3316TestDialog::ProcessMessage:case kC_COMMAND;kCM_CHECKBUTTON parm1 = %d \n",parm1);
               switch (parm1) {

	
			   case SIS3316TestDialog_kCM_CHECKBUTTON_IRQ_NO_9:  //  
				   if (fChkStopAfterTime->IsOn() == kTRUE) {
					   fNumericEntriesStopAfterTime->SetState(kTRUE); //
				   }
				   else {
					   fNumericEntriesStopAfterTime->SetState(kFALSE); //
				   }
				   break;


					case SIS3316TestDialog_kCM_CHECKBUTTON_IRQ_NO_10:  //  
						if (fChkStopAfterBanks->IsOn() == kTRUE)  {
							fNumericEntriesStopAfterBanks->SetState(kTRUE); //
						}
						else {
							fNumericEntriesStopAfterBanks->SetState(kFALSE); //
						}
                     break;



					case SIS3316TestDialog_kCM_CHECKBUTTON_IRQ_NO_11:  //  
							if (fChkNofEvents_ProBank->IsOn() == kTRUE)  {
								this->root_chk_bank_event_nof_limit_on_flag = 1;
								fNumericEntries_SampleControl_MaxNofEvents_ProBank->SetState(kTRUE); // // not dim
							}
							else {
								this->root_chk_bank_event_nof_limit_on_flag = 0;
								fNumericEntries_SampleControl_MaxNofEvents_ProBank->SetState(kFALSE); // dim
							}
                     break;


					case SIS3316TestDialog_kCM_CHECKBUTTON_IRQ_NO_12:  //  
							if (fChkWriteDataToFile->IsOn() == kTRUE)  {
								fChkWriteMultipleFiles->SetEnabled(kTRUE); //
								fTextEntryDataFilePath->SetEnabled(kTRUE); //
								fTextButtonDataFilePath->SetEnabled(kTRUE); //
							}
							else {
								fChkWriteMultipleFiles->SetEnabled(kFALSE); //
								fTextEntryDataFilePath->SetEnabled(kFALSE); //
								fTextButtonDataFilePath->SetEnabled(kFALSE); //
							}

                     break;


						 
					case SIS3316TestDialog_kCM_CHECKBUTTON_IRQ_NO_21:  //  DisplayAutoZoom
						if (fChkDisplayAutoZoom->IsOn() == kTRUE) {
							fChkDisplayDisableDeleteGraph->SetState(kButtonUp); // is Off !
							fChkDisplayDisableDeleteGraph->SetEnabled(kFALSE); // dim
							fNumericEntriesGraph_Yaxis[0]->SetState(kFALSE); // dim
							fNumericEntriesGraph_Yaxis[1]->SetState(kFALSE); // dim
							fNumericEntriesGraph_Xaxis[0]->SetState(kTRUE); // not dim
							fNumericEntriesGraph_Xaxis[1]->SetState(kTRUE); // not dim
						}
						else {
							if (fChkDisplayDisableDeleteGraph->IsOn() == kFALSE) {
								fChkDisplayDisableDeleteGraph->SetEnabled(kTRUE); // not dim
							}
							if (fChkDisplayDisableDeleteGraph->IsOn() == kTRUE) {
								fNumericEntriesGraph_Yaxis[0]->SetState(kFALSE); // dim
								fNumericEntriesGraph_Yaxis[1]->SetState(kFALSE); // dim
								fNumericEntriesGraph_Xaxis[0]->SetState(kFALSE); // dim
								fNumericEntriesGraph_Xaxis[1]->SetState(kFALSE); // dim
							}
							else {
								fNumericEntriesGraph_Yaxis[0]->SetState(kTRUE); // not dim
								fNumericEntriesGraph_Yaxis[1]->SetState(kTRUE); // not dim
								fNumericEntriesGraph_Xaxis[0]->SetState(kTRUE); // not dim
								fNumericEntriesGraph_Xaxis[1]->SetState(kTRUE); // not dim
							}
						}
						break;




					  case SIS3316TestDialog_kCM_CHECKBUTTON_IRQ_NO_40: // Sample Clock configuration
							fStartB->SetEnabled(kFALSE); // dim
							fStartB->ChangeBackground(red);
							fStopB->SetEnabled(kFALSE); // dim
							fClockConfiguration->SetEnabled(kTRUE); // not dim
							fClockConfiguration->ChangeBackground(green);
							sample_clock_configuration_valid_flag = 0 ;
						  break;
					 
	

					  case SIS3316TestDialog_kCM_CHECKBUTTON_IRQ_NO_60:  //  
						  data = 0;


						  for (i = 0; i < 32; i++) {
							  if (fChkLemoOutCoEnableCh[i]->GetState() == kButtonDown) {
								  data = data + (1 << i);
							  }
						  }
						  fNumericEntriesNimOutput[0]->SetHexNumber(data);

						  data = 0;
						  for (i = 0; i < 32; i++) {
							  if (fChkLemoOutToEnableCh[i]->GetState() == kButtonDown) {
								  data = data + (1 << i);
							  }
						  }
						  fNumericEntriesNimOutput[1]->SetHexNumber(data);

						  data = 0;
						  for (i = 0; i < 32; i++) {
							  if (fChkLemoOutUoEnableCh[i]->GetState() == kButtonDown) {
								  data = data + (1 << i);
							  }
						  }
						  fNumericEntriesNimOutput[2]->SetHexNumber(data);


						  SIS3316_Test_Write_NIM_Output_Selection(); // write to registers

						// clr TO Pulse generation bit
						  data = fNumericEntriesNimOutput[1]->GetIntNumber();
						  data = data & 0x7FFFFFFF; // Bit 31 : pulse generation
						  fNumericEntriesNimOutput[1]->SetHexNumber(data);
						  fChkLemoOutToEnableCh[31]->SetState(kButtonUp); // is OFF !


						  // clr UO Pulse generation bits
						  data = fNumericEntriesNimOutput[2]->GetIntNumber();
						  data = data & 0x7FFF8FFF; // Bit 31, 14,13,12 : pulse generation
						  fNumericEntriesNimOutput[2]->SetHexNumber(data);
						  fChkLemoOutUoEnableCh[31]->SetState(kButtonUp); // is OFF !
						  fChkLemoOutUoEnableCh[14]->SetState(kButtonUp); // is OFF !
						  fChkLemoOutUoEnableCh[13]->SetState(kButtonUp); // is OFF !
						  fChkLemoOutUoEnableCh[12]->SetState(kButtonUp); // is OFF !

						  break;


					  case SIS3316TestDialog_kCM_CHECKBUTTON_IRQ_NO_61:  //  Event-Dataformat Parameter
						  this->SIS3316_Test_Calculate_MaxNofEventsEachBank();
						  break;



					 //
					case SIS3316TestDialog_kCM_CHECKBUTTON_IRQ_NO_74:  //  
						invert_parameter_has_changed_flag = 1 ;
                     break;
//
					case SIS3316TestDialog_kCM_CHECKBUTTON_IRQ_NO_84:  //  
						gain_termination_parameter_has_changed_flag = 1 ;
                     break;



				  case SIS3316TestDialog_kCM_CHECKBUTTON_IRQ_NO_100: //fChkFFT_Sum
					if (fChkFFT_Sum->IsOn() == kTRUE)  {
						fChkFFT_Db->SetEnabled(kFALSE)   ; //
						//fChkFFT_Db->SetState(kButtonUp)   ; // is Off !
						fChkFFTLogY->SetEnabled(kTRUE)   ; //
						fChkFFTLogY->SetState(kButtonUp)   ; // is Off !
					}
					else {
						//fChkFFT_Db->SetState(kButtonDown)   ; // is On !
						fChkFFTLogY->SetState(kButtonUp)   ; // is Off !
						fChkFFTLogY->SetEnabled(kFALSE)   ; //
						fChkFFT_Db->SetEnabled(kTRUE)   ; //
						if (fChkFFT_Db->IsOn() == kTRUE)  {
							fChkFFT_Sum->SetEnabled(kFALSE)   ; //
						}
						if (fB_openfCanvas3WindowFlag == kTRUE) {
							fCanvas3->SetLogy(fChkFFTLogY->IsOn() == kTRUE);
						}
					}
					  break;

                  case SIS3316TestDialog_kCM_CHECKBUTTON_IRQ_NO_101: // fChkFFTLogY
			  			if (fB_openfCanvas3WindowFlag == kTRUE) {
							fCanvas3->SetLogy(fChkFFTLogY->IsOn() == kTRUE);
						}
						else {
							fChkFFTLogY->SetState(kButtonUp)   ; // is Off !
						}
					  break;

                  case SIS3316TestDialog_kCM_CHECKBUTTON_IRQ_NO_102: // fChkFFT_Db
						if (fChkFFT_Db->IsOn() == kTRUE)  {
							fChkFFTLogY->SetState(kButtonUp)   ; // is Off !
							fChkFFTLogY->SetEnabled(kFALSE)   ; //
							fChkFFT_Sum->SetEnabled(kFALSE)   ; //
						}
						else {
							fChkFFT_Sum->SetEnabled(kTRUE)   ; //
							fChkFFTLogY->SetEnabled(kTRUE)   ; //
						}

						if (fB_openfCanvas3WindowFlag == kTRUE) {
							fCanvas3->SetLogy(fChkFFTLogY->IsOn() == kTRUE);
						}
						else {
							//fChkFFTLogY->SetState(kButtonUp)   ; // is Off !
						}
					  break;

                  default:
                     break;
               }
               break;

#ifdef not_used
	    case kCM_TAB:
               //printf("Tab item %ld activated\n", parm1);
               break;
#endif



		volatile unsigned int uint_SampleControl_BankModus ;
		unsigned int i;

		case kCM_COMBOBOX:
              //printf("kCM_COMBOBOX item %ld activated\n", parm1);
               switch (parm1) {
                  case SIS3316TestDialog_kCM_COMBOBOX_IRQ_NO_30: // Selection SampleControl_BankModus
					  	uint_SampleControl_BankModus = fCombo_SampleControl_BankModus->GetSelected();

						// Close Windows: Raw dispaly, MAW display, histogram display

						if (fB_openfCanvas1WindowFlag == kTRUE) {
							for (i = 0; i < 17; i++) {
								//delete fGraph_ch[i];
//								delete fGraph_Text_ch[i];
							}
							delete fCanvas1; //
							fB_openfCanvas1WindowFlag = kFALSE; // Setup
						}

						if (fB_openfCanvas2WindowFlag == kTRUE) {
							delete fCanvas2; //
							fB_openfCanvas2WindowFlag = kFALSE; // Setup
						}
						if (fB_openfCanvas3WindowFlag == kTRUE) {
							delete fCanvas3; //
							fB_openfCanvas3WindowFlag = kFALSE; // Setup
						}
						if (fB_openfCanvas4WindowFlag == kTRUE) {
							delete fCanvas4; //
							fB_openfCanvas4WindowFlag = kFALSE; // Setup
						}

						this->SIS3316_Test_Update_Gui_Entries();
						this->SIS3316_Test_Calculate_MaxNofEventsEachBank();

						break;



				  case SIS3316TestDialog_kCM_COMBOBOX_IRQ_NO_40: // Sample Clock configuration
							fStartB->SetEnabled(kFALSE); // dim
							fStartB->ChangeBackground(red);
							fStopB->SetEnabled(kFALSE); // dim
							fClockConfiguration->SetEnabled(kTRUE); // not dim
							fClockConfiguration->ChangeBackground(green);
							sample_clock_configuration_valid_flag = 0 ;
					  break;





				  case SIS3316TestDialog_kCM_COMBOBOX_IRQ_NO_90: //SPI ADC
						adc_input_voltage_select = fCombo_Set_ADC_SPI_Input_Voltage->GetSelected();
					  	if (gl_sis3316_adc1->adc_125MHz_flag == 0) { // 250 MHz chip AD9643
							switch (adc_input_voltage_select) {
								case 0: //SPI ADC
									data = 0x15 ; 	//  1.50V (1.75V - (11 * 0.022V) = 1,508)
									break;
								case 1: //SPI ADC
									data = 0x0 ; 	//  1.75V
									break;
								case 2: //SPI ADC
									data = 0xB ; 	//  2.00V (1.75V + (11 * 0.022V) = 1,992)
									break;
								default:
									data = 0x0 ; 	//  1.75V
									break;
							}
						}
						else { // 125 MHz chip AD9268
							switch (adc_input_voltage_select) {
								case 0: //SPI ADC
									data = 0x40 ; 	//  1.50V
									break;
								case 1: //SPI ADC
									data = 0x80 ; 	//  1.75V
									break;
								case 2: //SPI ADC
									data = 0xC0 ; 	//  2.00V
									break;
								default:
									data = 0x80 ; 	//  1.75V
									break;
							}
						}
						for (i_adc_fpga_group = 0; i_adc_fpga_group < 4; i_adc_fpga_group++) {
							for (i_adc_chip = 0; i_adc_chip < 2; i_adc_chip++) {
								gl_sis3316_adc1->adc_spi_write( i_adc_fpga_group, i_adc_chip, 0x18, data);
								gl_sis3316_adc1->adc_spi_write( i_adc_fpga_group, i_adc_chip, 0xff, 0x1);  // update
							}
						}
						break;

				  default:
                     break;
               }
			  break;

	    default:
              //printf("default item %ld activated\n", parm1);
               break;
         }
         break;

		 int ymin, ymax;
		 int xmin, xmax;

 	case kC_TEXTENTRY:
      //printf("kC_TEXTENTRY item %ld activated\n", parm1);
	  switch (GET_SUBMSG(msg)) {
	      case kTE_TEXTCHANGED:
				switch(parm1) {
					  case SIS3316TestDialog_kCM_ENTRY_IRQ_NO_11:
						  this->raw_sample_length = fNumericEntries_EventHitParameter[2]->GetIntNumber();
						  this->SIS3316_Test_Calculate_MaxNofEventsEachBank();
						  break;


					  case SIS3316TestDialog_kCM_ENTRY_IRQ_NO_20: // Raw data y-axis min
						  ymax = fNumericEntriesGraph_Yaxis[0]->GetIntNumber();
						  ymin = fNumericEntriesGraph_Yaxis[1]->GetIntNumber();
						  if (ymin >= (ymax - 1)) {
							  ymin = this->raw_graph_ymin; // take old one
						  }
						  fNumericEntriesGraph_Yaxis[1]->SetIntNumber(ymin); // Y-min
						  this->raw_graph_ymin = ymin;
						  break;

					  case SIS3316TestDialog_kCM_ENTRY_IRQ_NO_21: // Raw data y-axis max
						  ymax = fNumericEntriesGraph_Yaxis[0]->GetIntNumber();
						  ymin = fNumericEntriesGraph_Yaxis[1]->GetIntNumber();
						  if ((ymax - 1) <= (ymin)) {
							  ymax = this->raw_graph_ymax;
						  }
						  if (ymax > this->raw_graph_ymax_absolute) {
							  ymax = this->raw_graph_ymax_absolute;
						  }
						  fNumericEntriesGraph_Yaxis[0]->SetIntNumber(ymax); // Y-max
						  this->raw_graph_ymax = ymax;
						  break;



					  case SIS3316TestDialog_kCM_ENTRY_IRQ_NO_22: // Raw data x-axis min
						  xmax = fNumericEntriesGraph_Xaxis[0]->GetIntNumber();
						  xmin = fNumericEntriesGraph_Xaxis[1]->GetIntNumber();
						  if (xmin >= (xmax-1)) {
							  xmin = this->raw_graph_xmin; // take old one
						  }					  
						  fNumericEntriesGraph_Xaxis[1]->SetIntNumber(xmin); // X-min
						  this->raw_graph_xmin = xmin;
						  break;



					  case SIS3316TestDialog_kCM_ENTRY_IRQ_NO_23: // Raw data x-axis max
						  xmax = fNumericEntriesGraph_Xaxis[0]->GetIntNumber();
						  xmin = fNumericEntriesGraph_Xaxis[1]->GetIntNumber();
						  if ((xmax-1) <= (xmin)) {
							  xmax = this->raw_graph_xmax;
						  }
						  if (xmax > this->raw_graph_xmax_absolute) {
							  xmax = this->raw_graph_xmax_absolute;
						  }
						  fNumericEntriesGraph_Xaxis[0]->SetIntNumber(xmax); // X-max
 						  this->raw_graph_xmax = xmax;
						  break;


					  case SIS3316TestDialog_kCM_ENTRY_IRQ_NO_24: // Raw data x-axis min
						  xmax = fNumericEntriesHisto_Xaxis[0]->GetIntNumber();
						  xmin = fNumericEntriesHisto_Xaxis[1]->GetIntNumber();
						  if (xmin >= (xmax - 1)) {
							  xmin = this->root_histo_xmin; // take old one
						  }
						  fNumericEntriesHisto_Xaxis[1]->SetIntNumber(xmin); // X-min
						  this->root_histo_xmin = xmin;
						  break;


					  case SIS3316TestDialog_kCM_ENTRY_IRQ_NO_25: // Raw data x-axis max
						  xmax = fNumericEntriesHisto_Xaxis[0]->GetIntNumber();
						  xmin = fNumericEntriesHisto_Xaxis[1]->GetIntNumber();
						  if ((xmax - 1) <= (xmin)) {
							  xmax = this->root_histo_xmax;
						  }
						  if (xmax > this->root_histo_xmax_absolute) {
							  xmax = this->root_histo_xmax_absolute;
						  }
						  fNumericEntriesHisto_Xaxis[0]->SetIntNumber(xmax); // X-max
						  this->root_histo_xmax = xmax;
						  break;

					  
					  case SIS3316TestDialog_kCM_ENTRY_IRQ_NO_40:
						 //printf("\n pressed SIS3316TestDialog_kCM_ENTRY_IRQ_NO_40\n");
						  offset_parameter_has_changed_flag = 1 ;
						  break;



					// Lemo Out selection
					  case SIS3316TestDialog_kCM_ENTRY_IRQ_NO_60:

						// Lemo out CO
						  data = fNumericEntriesNimOutput[0]->GetIntNumber();

						  for (int i = 0; i < 32; i++) {
							  if (((data >> i) & 0x1) == 1) {
								  fChkLemoOutCoEnableCh[i]->SetState(kButtonDown); // is ON !
							  }
							  else {
								  fChkLemoOutCoEnableCh[i]->SetState(kButtonUp); // is OFF !
							  }
						  }

						  // Lemo out TO
						  data = fNumericEntriesNimOutput[1]->GetIntNumber();
						  for (int i = 0; i < 32; i++) {
							  if (((data >> i) & 0x1) == 1) {
								  fChkLemoOutToEnableCh[i]->SetState(kButtonDown); // is ON !
							  }
							  else {
								  fChkLemoOutToEnableCh[i]->SetState(kButtonUp); // is OFF !
							  }
						  }

						  // Lemo out UO
						  data = fNumericEntriesNimOutput[2]->GetIntNumber();
						  for (int i = 0; i < 32; i++) {
							  if (((data >> i) & 0x1) == 1) {
								  fChkLemoOutUoEnableCh[i]->SetState(kButtonDown); // is ON !
							  }
							  else {
								  fChkLemoOutUoEnableCh[i]->SetState(kButtonUp); // is OFF !
							  }
						  }

						  SIS3316_Test_Write_NIM_Output_Selection(); // write to registers

						// clr TO Pulse generation bit
						  data = fNumericEntriesNimOutput[1]->GetIntNumber();
						  data = data & 0x7FFFFFFF; // Bit 31 : pulse generation
						  fNumericEntriesNimOutput[1]->SetHexNumber(data);
						  fChkLemoOutToEnableCh[31]->SetState(kButtonUp); // is OFF !


						  // clr UO Pulse generation bits
						  data = fNumericEntriesNimOutput[2]->GetIntNumber();
						  data = data & 0x7FFF8FFF; // Bit 31, 14,13,12 : pulse generation
						  fNumericEntriesNimOutput[2]->SetHexNumber(data);
						  fChkLemoOutUoEnableCh[31]->SetState(kButtonUp); // is OFF !
						  fChkLemoOutUoEnableCh[14]->SetState(kButtonUp); // is OFF !
						  fChkLemoOutUoEnableCh[13]->SetState(kButtonUp); // is OFF !
						  fChkLemoOutUoEnableCh[12]->SetState(kButtonUp); // is OFF !
						  break;


					  case SIS3316TestDialog_kCM_ENTRY_IRQ_NO_61:
						  this->SIS3316_Test_Calculate_MaxNofEventsEachBank();
						  break;

						  
					  case SIS3316TestDialog_kCM_ENTRY_IRQ_NO_90:
						gl_sis3316_adc1->register_read( SIS3316_ADC_CH1_FIR_ENERGY_SETUP_REG, &data);  //
						old_energy_peaking_val             = data & 0xfff ;
						old_energy_gap_val                 = (data & 0x3ff000) >> 12  ;
						old_energy_additional_average_val  = (data & 0xc000000) >> 22 ;
						old_energy_decay_tau_factor_val    = (data & 0x3f000000) >> 24 ;
						old_energy_decay_tau_table_val     = (data & 0xc0000000) >> 30 ;


						energy_peaking_val =  fNumericEntriesEnergyPeaking->GetIntNumber(); ;
						if (gl_sis3316_adc1->adc_125MHz_flag == 0) { // SIS3316-250MHz-14bit
							if (energy_peaking_val != 0) {
								if (energy_peaking_val > old_energy_peaking_val) {
									energy_peaking_val =  (energy_peaking_val + 1) & 0xffe ; // only even
								}
								if (energy_peaking_val < old_energy_peaking_val) {
									energy_peaking_val =  (energy_peaking_val - 1) & 0xffe ; // only even
								}
							}
						}
						if (energy_peaking_val > FIR_ENERGY_MAX_PEAKING) { energy_peaking_val = FIR_ENERGY_MAX_PEAKING; } //
						if (energy_peaking_val < FIR_ENERGY_MIN_PEAKING)   { energy_peaking_val = FIR_ENERGY_MIN_PEAKING; }   //
						fNumericEntriesEnergyPeaking->SetIntNumber(energy_peaking_val );

						fNumericEntriesHistogramXaxisDivider->SetIntNumber(fNumericEntriesEnergyPeaking->GetIntNumber());

					    energy_gap_val =  fNumericEntriesEnergyGap->GetIntNumber(); ;
						if (gl_sis3316_adc1->adc_125MHz_flag == 0) { // SIS3316-250MHz-14bit
							if (energy_gap_val != 0) {
								if (energy_gap_val > old_energy_gap_val) {
									energy_gap_val =  (energy_gap_val + 1) & 0x3fe ; // only even
								}
								if (energy_gap_val < old_energy_gap_val) {
									energy_gap_val =  (energy_gap_val - 1) & 0x3fe ; // only even
								}
							}
						}
						if (energy_gap_val > FIR_ENERGY_MAX_GAP) { energy_gap_val = FIR_ENERGY_MAX_GAP; } //
						if (energy_gap_val < FIR_ENERGY_MIN_GAP)   { energy_gap_val = FIR_ENERGY_MIN_GAP; }   //
						fNumericEntriesEnergyGap->SetIntNumber(energy_gap_val );


						energy_decay_tau_table_val =  fNumericEntriesEnergyTauTable->GetIntNumber(); ;
						if (energy_decay_tau_table_val > FIR_ENERGY_MAX_TAU_TABLE) { energy_decay_tau_table_val = FIR_ENERGY_MAX_TAU_TABLE; } //
						fNumericEntriesEnergyTauTable->SetIntNumber(energy_decay_tau_table_val );

						energy_decay_tau_factor_val =  fNumericEntriesEnergyTauFactor->GetIntNumber(); ;
						if (energy_decay_tau_factor_val > FIR_ENERGY_MAX_TAU_FACTOR) { energy_decay_tau_factor_val = FIR_ENERGY_MAX_TAU_FACTOR; } //
						fNumericEntriesEnergyTauFactor->SetIntNumber(energy_decay_tau_factor_val );


						energy_additional_average_val =  fNumericEntriesEnergyAdditionalAverage->GetIntNumber(); ;
						if (energy_additional_average_val > FIR_ENERGY_MAX_ADD_AVERAGE) { energy_additional_average_val = FIR_ENERGY_MAX_ADD_AVERAGE; } //
						fNumericEntriesEnergyAdditionalAverage->SetIntNumber(energy_additional_average_val );


						// Clear FIR Energy Setup
						data = 0;
						for (i_adc_fpga = 0; i_adc_fpga < 4; i_adc_fpga++) {
							gl_sis3316_adc1->register_write((i_adc_fpga * SIS3316_FPGA_ADC_REG_OFFSET) + SIS3316_ADC_CH1_FIR_ENERGY_SETUP_REG, data);
							gl_sis3316_adc1->register_write((i_adc_fpga * SIS3316_FPGA_ADC_REG_OFFSET) + SIS3316_ADC_CH2_FIR_ENERGY_SETUP_REG, data);
							gl_sis3316_adc1->register_write((i_adc_fpga * SIS3316_FPGA_ADC_REG_OFFSET) + SIS3316_ADC_CH3_FIR_ENERGY_SETUP_REG, data);
							gl_sis3316_adc1->register_write((i_adc_fpga * SIS3316_FPGA_ADC_REG_OFFSET) + SIS3316_ADC_CH4_FIR_ENERGY_SETUP_REG, data);
						}
						// set FIR Energy Setup
						data =    ((energy_decay_tau_table_val & FIR_ENERGY_MAX_TAU_TABLE) << 30)
							   +  ((energy_decay_tau_factor_val & FIR_ENERGY_MAX_TAU_FACTOR) << 24)
							   +  ((energy_additional_average_val  & 0x3) << 22)
							   +  ((energy_gap_val  & 0x3ff) << 12)
							   +  (energy_peaking_val & 0xfff) ; //
						if ( ((this->adc_fpga_firmware_version & 0x0f00) == 0x0100) || ((this->adc_fpga_firmware_version & 0x0f00) == 0x0200)){ // Neutron/Gamma
							data =  0  ; // PSD Histogram configuration register have to be set 0
						}

						for (i_adc_fpga=0; i_adc_fpga<4; i_adc_fpga++) {
							gl_sis3316_adc1->register_write( (i_adc_fpga*SIS3316_FPGA_ADC_REG_OFFSET) + SIS3316_ADC_CH1_FIR_ENERGY_SETUP_REG, data) ;
							gl_sis3316_adc1->register_write( (i_adc_fpga*SIS3316_FPGA_ADC_REG_OFFSET) + SIS3316_ADC_CH2_FIR_ENERGY_SETUP_REG, data) ;
							gl_sis3316_adc1->register_write( (i_adc_fpga*SIS3316_FPGA_ADC_REG_OFFSET) + SIS3316_ADC_CH3_FIR_ENERGY_SETUP_REG, data) ;
							gl_sis3316_adc1->register_write( (i_adc_fpga*SIS3316_FPGA_ADC_REG_OFFSET) + SIS3316_ADC_CH4_FIR_ENERGY_SETUP_REG, data) ;
						}

						break;

					  case 120:
							unsigned int tap_delay_val;
							tap_delay_val =  fNumericEntriesTapDelay->GetIntNumber(); ;
							fNumericEntriesTapDelay->SetIntNumber(tap_delay_val); //
							gl_sis3316_adc1->register_write( SIS3316_ADC_CH1_4_INPUT_TAP_DELAY_REG, (tap_delay_val & 0x10ff)  + 0x300 ) ;
							gl_sis3316_adc1->register_write( SIS3316_ADC_CH5_8_INPUT_TAP_DELAY_REG, (tap_delay_val & 0x10ff)  + 0x300 ) ;
							gl_sis3316_adc1->register_write( SIS3316_ADC_CH9_12_INPUT_TAP_DELAY_REG, (tap_delay_val & 0x10ff)  + 0x300 ) ;
							gl_sis3316_adc1->register_write( SIS3316_ADC_CH13_16_INPUT_TAP_DELAY_REG, (tap_delay_val & 0x10ff)  + 0x300 ) ;

						   //printf("\n pressed 120\n");
						break;

				}
			break;
	  }
	  break;

      default:
	  printf("\nSIS3316TestDialog::ProcessMessage:case default parm1 = %d\n",parm1);
      break;
   }
   return kTRUE;
}

/*******************************************************/

void SIS3316TestDialog::SIS3316_Test_Write_TerminationGain(void)
{
	unsigned int i ;
	unsigned int gain_termination_reg_val ;
	  //printf("\SIS3316_Test_Write_TerminationGain\n" );

	for (i = 0; i < 16; i++) {
		gain_termination_reg_val = 0 ;
		if (fChkTerminationChannel[i]->IsOn() == kFALSE)  {
			gain_termination_reg_val = gain_termination_reg_val + 0x04 ;
		}
		if (fChkInputRange0Channel[i]->IsOn() == kFALSE)  {
			gain_termination_reg_val = gain_termination_reg_val + 0x01 ;
		}
		gl_sis3316_adc1->adc_gain_termination_ch_array[i] = gain_termination_reg_val ; //  
	}
	gl_sis3316_adc1->write_all_gain_termination_values() ; 
	gain_termination_parameter_has_changed_flag = 0 ;
}

/*******************************************************/

void SIS3316TestDialog::SIS3316_Test_Write_DacOffset(void)
{
	unsigned int i ;
	unsigned int dac_offset_reg_val ;
	 //printf("\SIS3316_Test_Write_DacOffset\n" );

	for (i = 0; i < 16; i++) {
		dac_offset_reg_val =  fNumericEntriesAnalogOffset[i]->GetIntNumber()   ;
		if(dac_offset_reg_val > 0x10000) {
			dac_offset_reg_val = 0xffff ;
		}
		if (fChkDacInrementTest->IsOn() == kTRUE) {
			dac_offset_reg_val = (dac_offset_reg_val + 0x100) & 0xffff;
		}
		fNumericEntriesAnalogOffset[i]->SetIntNumber(dac_offset_reg_val);	
		gl_sis3316_adc1->adc_dac_offset_ch_array[i] = dac_offset_reg_val ; //  
	}
	gl_sis3316_adc1->write_all_adc_dac_offsets() ; 
	offset_parameter_has_changed_flag = 0 ;
}

/*******************************************************/

void SIS3316TestDialog::SIS3316_Test_Write_NIM_Output_Selection(void)
{
	unsigned int i ;
	for (i = 0; i < 3; i++) {
		gl_sis3316_adc1->nim_output_selection_array[i] =  fNumericEntriesNimOutput[i]->GetIntNumber()   ;
	}
	gl_sis3316_adc1->write_nim_output_selection_values() ; 
}



void SIS3316TestDialog::SIS3316_Test_Update_Gui_Entries(void)
{
	if (fB_openfCanvas1WindowFlag == kTRUE) {
		delete fCanvas1; //
		fB_openfCanvas1WindowFlag = kFALSE; // Setup
	}

	if (fB_openfCanvas2WindowFlag == kTRUE) {
		delete fCanvas2; //
		fB_openfCanvas2WindowFlag = kFALSE; // Setup
	}
	if (fB_openfCanvas3WindowFlag == kTRUE) {
		delete fCanvas3; //
		fB_openfCanvas3WindowFlag = kFALSE; // Setup
	}
	if (fB_openfCanvas4WindowFlag == kTRUE) {
		delete fCanvas4; //
		fB_openfCanvas4WindowFlag = kFALSE; // Setup
	}

	// Gui: Write to File
	if (fCombo_SampleControl_BankModus->GetSelected() == 0) {
		fChkWriteDataToFile->SetState(kButtonUp); // is OFF !
		fChkWriteDataToFile->SetEnabled(kFALSE); //
		fChkWriteMultipleFiles->SetEnabled(kFALSE); //
		fTextEntryDataFilePath->SetEnabled(kFALSE); //
		fTextButtonDataFilePath->SetEnabled(kFALSE); //

	}
	else {
		fChkWriteDataToFile->SetEnabled(kTRUE); //
		if (fChkWriteDataToFile->IsOn() == kTRUE) {
			fChkWriteMultipleFiles->SetEnabled(kTRUE); //
			fTextEntryDataFilePath->SetEnabled(kTRUE); //
			fTextButtonDataFilePath->SetEnabled(kTRUE); //
		}
		else {
			fChkWriteMultipleFiles->SetEnabled(kFALSE); //
			fTextEntryDataFilePath->SetEnabled(kFALSE); //
			fTextButtonDataFilePath->SetEnabled(kFALSE); //
		}
	}


	// Gui: Sample Control
	if (fCombo_SampleControl_BankModus->GetSelected() == 0) {
		fChkNofEvents_ProBank->SetEnabled(kFALSE); // dim
		fNumericEntries_SampleControl_MaxNofEvents_ProBank->SetState(kFALSE); //
	}
	else {
		fChkNofEvents_ProBank->SetEnabled(kTRUE); // not dim
		fChkNofEvents_ProBank->SetState(this->root_chk_bank_event_nof_limit_on_flag ? kButtonDown : kButtonUp);

		if (fChkNofEvents_ProBank->IsOn() == kTRUE) {
			fNumericEntries_SampleControl_MaxNofEvents_ProBank->SetState(kTRUE); //
		}
		else {
			fNumericEntries_SampleControl_MaxNofEvents_ProBank->SetState(kFALSE); //
		}
	}



	// Gui: Raw Data Graph
	if (fCombo_SampleControl_BankModus->GetSelected() == 0) { // Single Bank
		fChkDisplayAutoZoom->SetEnabled(kTRUE); //
		if (fChkDisplayAutoZoom->IsOn() == kTRUE) {
			fNumericEntriesGraph_Yaxis[0]->SetState(kFALSE); // dim
			fNumericEntriesGraph_Yaxis[1]->SetState(kFALSE); // dim
			fChkDisplayDisableDeleteGraph->SetState(kButtonUp); // is Off !
			fChkDisplayDisableDeleteGraph->SetEnabled(kFALSE); // dim
			fNumericEntriesGraph_Xaxis[0]->SetState(kTRUE); //  not dim
			fNumericEntriesGraph_Xaxis[1]->SetState(kTRUE); // not dim
		}
		else {
			fChkDisplayDisableDeleteGraph->SetState(kButtonUp); // is Off !
			fChkDisplayDisableDeleteGraph->SetEnabled(kTRUE); // not dim
			fNumericEntriesGraph_Yaxis[0]->SetState(kTRUE); //  not dim
			fNumericEntriesGraph_Yaxis[1]->SetState(kTRUE); // not dim
			fNumericEntriesGraph_Xaxis[0]->SetState(kTRUE); //  not dim
			fNumericEntriesGraph_Xaxis[1]->SetState(kTRUE); // not dim
		}
	}
	else {
		fChkDisplayAutoZoom->SetState(kButtonUp); // is Off !
		fChkDisplayAutoZoom->SetEnabled(kFALSE); // dim
		fChkDisplayDisableDeleteGraph->SetState(kButtonUp); // is Off !
		fChkDisplayDisableDeleteGraph->SetEnabled(kFALSE); // dim
		fNumericEntriesGraph_Yaxis[0]->SetState(kTRUE); //  not dim
		fNumericEntriesGraph_Yaxis[1]->SetState(kTRUE); // not dim
		fNumericEntriesGraph_Xaxis[0]->SetState(kTRUE); //  not dim
		fNumericEntriesGraph_Xaxis[1]->SetState(kTRUE); // not dim
	}





	// Gui: SampleControl_BankModus
	if (fCombo_SampleControl_BankModus->GetSelected() == 0) { // Single Bank
		fCombo_Display_MAW->Select(1, kTRUE); // display ch1
		fCombo_Display_MAW->SetEnabled(kTRUE); //  not dim
		fCombo_Display_MAW->Select(params->uint_DisplayMAW_select, kTRUE); //  

		fCombo_Display_FFT_Ch->Select(0, kTRUE); // no display ch
		fCombo_Display_FFT_Ch->SetEnabled(kTRUE); //  not dim
		fCombo_Display_FFT_Ch->Select(params->uint_DisplayFFT_select, kTRUE); //  
		
		fCombo_Display_Histos_Build->RemoveAll();
		fCombo_Display_Histos_Build->AddEntry(entryHistoDisplayOption[0], 0);
		fCombo_Display_Histos_Build->Select(0, kTRUE); // build ADC output code
		fCombo_Display_Histos_Build->SetEnabled(kFALSE); // dim


		fNumericEntriesHistogramXaxisOffset->SetState(kFALSE); // dim
		fNumericEntriesHistogramXaxisDivider->SetState(kFALSE); // dim
		fChkHistoSum->SetEnabled(kTRUE); // not dim
		fChkHistoZoomMean->SetEnabled(kTRUE); // not dim

		fChkHistoSum->SetState(kButtonUp); // is Off !

	}
	else {
		fCombo_Display_MAW->Select(0, kTRUE); // no display
		fCombo_Display_MAW->SetEnabled(kFALSE); // dim

		fCombo_Display_FFT_Ch->Select(0, kTRUE); // no display ch
		fCombo_Display_FFT_Ch->SetEnabled(kFALSE); // dim

		fCombo_Display_Histos_Build->RemoveAll();
		fCombo_Display_Histos_Build->AddEntry(entryHistoDisplayOption[1], 1);
		fCombo_Display_Histos_Build->AddEntry(entryHistoDisplayOption[2], 2);
		fCombo_Display_Histos_Build->Select(1, kTRUE); // Energy
		fCombo_Display_Histos_Build->SetEnabled(kTRUE); //  not dim

		fNumericEntriesHistogramXaxisOffset->SetState(kTRUE); // not dim
		fNumericEntriesHistogramXaxisDivider->SetState(kTRUE); // not dim

		fChkHistoSum->SetState(kButtonDown); // is On !
		fChkHistoSum->SetEnabled(kFALSE); //   dim
		fChkHistoZoomMean->SetEnabled(kFALSE); //   dim


	}

	// update histogram x-axis divider value (Peaking Time value of Energy Filter)
	fNumericEntriesHistogramXaxisDivider->SetIntNumber(fNumericEntriesEnergyPeaking->GetIntNumber());

}

/****************************************************************************************************/

void SIS3316TestDialog::SIS3316_Test_Calculate_MaxNofEventsEachBank(void)
{
	unsigned int possibe_nof_events_pro_bank;

	unsigned int header_length;
	unsigned int sample_length;
	unsigned int maw_test_buffer_length;
	unsigned int event_length;
	unsigned int short_event_length;

	header_length = 3;


	if (fChk_EventHitParameter_DataFormatBit0->IsOn() == kTRUE) {
		header_length = header_length + 7;
	}
	if (fChk_EventHitParameter_DataFormatBit1->IsOn() == kTRUE) {
		header_length = header_length + 2;
	}
	if (fChk_EventHitParameter_DataFormatBit2->IsOn() == kTRUE) {
		header_length = header_length + 3;
	}
	if (fChk_EventHitParameter_DataFormatBit3->IsOn() == kTRUE) {
		header_length = header_length + 2;
	}

	sample_length = fNumericEntries_EventHitParameter[2]->GetIntNumber();
	maw_test_buffer_length = fNumericEntries_EventHitParameter[5]->GetIntNumber();

	event_length = (header_length + (sample_length / 2) + maw_test_buffer_length);
	short_event_length = (header_length + maw_test_buffer_length);



	if (fCombo_SampleControl_BankModus->GetSelected() == 0) { // SINGLE_EVENT_SINGLE_BANK 
		possibe_nof_events_pro_bank = 1; // 
	}
	else { // MULTI_EVENT_DOUBLE_BANK
		if (event_length > (SIS3316_ADC_MEMORY_BANK_32BIT_SIZE / 2)) { // if event length > halffull then set to 1 event
			possibe_nof_events_pro_bank = 1; // 
		}
		else {
			if (fChk_SaveRawDataFirstEventOnly->IsOn() == kTRUE) {
				possibe_nof_events_pro_bank = (((SIS3316_ADC_MEMORY_BANK_32BIT_SIZE * 3) / 4) - event_length) / short_event_length; // (3/4 memory - 1 Event ) / short_event_length
				possibe_nof_events_pro_bank = possibe_nof_events_pro_bank + 1; // 
			}
			else {
				possibe_nof_events_pro_bank = ((SIS3316_ADC_MEMORY_BANK_32BIT_SIZE * 3) / 4) / event_length; // 3/4 full
				if (possibe_nof_events_pro_bank == 0) {
					possibe_nof_events_pro_bank = 1; // 
				}
			}
		}

	}
	fNumericEntries_SampleControl_PossibleMaxNofEvents_ProBank->SetIntNumber(possibe_nof_events_pro_bank); // 

	// X-axis max
	fNumericEntriesGraph_Xaxis[0]->SetIntNumber(sample_length); // X-max
	fNumericEntriesGraph_Xaxis[0]->SetLimits((TGNumberFormat::kNELLimitMinMax), 10, sample_length); //X - max
	this->raw_graph_xmax = sample_length;
	this->raw_graph_xmax_absolute = sample_length;

	fNumericEntriesGraph_Xaxis[1]->SetIntNumber(0); // X-min
	if (sample_length > 10) { // Sample Length
		fNumericEntriesGraph_Xaxis[1]->SetLimits((TGNumberFormat::kNELLimitMinMax), 0, sample_length - 10); //X - min
	}
	else {
		fNumericEntriesGraph_Xaxis[1]->SetLimits((TGNumberFormat::kNELLimitMinMax), 0, 0); //X - min
	}

}




/*******************************************************/

void SIS3316TestDialog::SIS3316_Test_Write_Sample_Clock_Configuration(void)
{ 
	unsigned int data, return_code ;
	unsigned int fp_lvds_bus_control_value  ;
	unsigned int clock_source_choice;
	unsigned int clock_freq_choice;
	unsigned int clock_multiplier_choice;
	unsigned int clock_N1div_val ;
	unsigned int clock_HSdiv_val ;
	unsigned int iob_delay_value ;

	
	return_code = gl_sis3316_adc1->register_write( SIS3316_KEY_RESET, 0);
	return_code = gl_sis3316_adc1->register_write( SIS3316_VME_FPGA_LINK_ADC_PROT_STATUS, 0xE0E0E0E0);  // clear error Latch bits

/**************************************************************************************************************/


// enable FP-Bus Clock Master
	fp_lvds_bus_control_value = 0 ;
	if (fChkFP_BUS_ClockMaster->IsOn() == kTRUE) {
		fp_lvds_bus_control_value = fp_lvds_bus_control_value + 0x10  ;
	}
	if (fCombo_FP_BUS_ClockOutMux->GetSelected() == 1) {
		fp_lvds_bus_control_value = fp_lvds_bus_control_value + 0x20  ;
	}
	return_code = gl_sis3316_adc1->register_write( SIS3316_FP_LVDS_BUS_CONTROL, fp_lvds_bus_control_value);  //


// set Clock Multiplier
	clock_multiplier_choice = fCombo_SetClockMultiplierMode->GetSelected();
	switch (clock_multiplier_choice) {
	    case 0: // Bypass
			return_code = gl_sis3316_adc1->bypass_external_clock_multiplier() ;
			break;
	    case 1: //
			return_code = gl_sis3316_adc1->set_external_clock_multiplier(0, 5, 4, 4, 500, 1, 10 ) ; // bw=0    n1_hs=5   n1_clk1 = 4   n1_clk2 = 4   n2=500  n3=1   range 10,00 to 11,34 -> 250 to 283
			break;
	    case 2: //
			return_code = gl_sis3316_adc1->set_external_clock_multiplier(1, 5, 4, 4, 250, 1, 20 ) ; // bw=0/1  n1_hs=5   n1_clk1 = 4   n1_clk2 = 4   n2=250  n3=1   range 19,40 to 22,68 -> 242 to 283
			break;
	    case 3: //
			return_code = gl_sis3316_adc1->set_external_clock_multiplier(2, 11, 2, 2, 110, 1, 50 ) ; // bw=1/2  n1_hs=11  n1_clk1 = 2   n1_clk2 = 2   n2=110  n3=1   range 44,00 to 51,54 -> 220 to 257
			break;
	    case 4: //
			return_code = gl_sis3316_adc1->set_external_clock_multiplier(0, 4, 10, 10, 500, 1, 10 ) ; // bw=0    n1_hs=4   n1_clk1 = 10  n1_clk2 = 10  n2=500  n3=1   range 10,00 to 11,34 -> 125 to 141
			break;
	    case 5: //
			return_code = gl_sis3316_adc1->set_external_clock_multiplier(1, 5, 8, 8, 250, 1, 20 ) ; // bw=0/1  n1_hs=5   n1_clk1 = 8   n1_clk2 = 8   n2=250  n3=1   range 19,40 to 22,68 -> 121 to 141
			break;
	    case 6: //
			return_code = gl_sis3316_adc1->set_external_clock_multiplier(2, 5, 8, 8, 100, 1, 50 ) ; // bw=1/2  n1_hs=5   n1_clk1 = 8   n1_clk2 = 8   n2=100  n3=1   range 48,50 to 56,70 -> 121 to 141
			break;
	    case 7: //
			return_code = gl_sis3316_adc1->set_external_clock_multiplier(0, 11, 2, 2, 440, 1, 12 ) ; //  bw=0    n1_hs=11   n1_clk = 2   n2=440  n3=1   range 11,02 to 12,87 -> 220 to 257
			break;
	    case 8: //
			return_code = gl_sis3316_adc1->set_external_clock_multiplier(0, 10, 4, 4, 400, 1, 12); // bw=0    n1_hs=10   n1_clk = 4   n2=400  n3=1   range 11,55 to 13,50 -> 115 to 135
			break;
	}


	if (return_code != 0) {
		printf("set_external_clock_multiplier: return_code = 0x%08x     \n", return_code);
	}


// define Sample Clock
	clock_source_choice = fCombo_SampleClock_source->GetSelected();
	data = 0 ;
	switch (clock_source_choice) {
	    case 0: // Internal Clock
			data = 0 ;
			break;
	    case 1: // VXS Clock
			data = 1 ;
			break;
	    case 2: // FP-Bus Clock
			data = 2 ;
			break;
	    case 3: //External NIM Clock
			data = 3 ;
			break;
	}
	return_code = gl_sis3316_adc1->register_write( SIS3316_SAMPLE_CLOCK_DISTRIBUTION_CONTROL, data);  //



// set internal Frequency
	clock_freq_choice = fCombo_SetInternalClockFreq->GetSelected();
	if (clock_freq_choice >= 16) {
		clock_freq_choice = 0 ;
		fCombo_SetInternalClockFreq->Select(clock_freq_choice, kTRUE); //  set frequency to 250 MHz
	}
	if (gl_sis3316_adc1->adc_125MHz_flag == 1) {
		if (clock_freq_choice < 6) {
			clock_freq_choice = 6 ;
			fCombo_SetInternalClockFreq->Select(clock_freq_choice, kTRUE); //  set frequency to 125 MHz
		}
	}
	// get SI570 Oscillator Parameter from library depends of ENUM: sample_rate (0:SAMPLERATE_250MSPS, 1:SAMPLERATE_227MSPS, ..., 6:SAMPLERATE_125MSPS, .... ,15:SAMPLERATE_25MSPS)
	gl_sis3316_adc1->get_SI570_oscillator_hs_div_and_n1_div_values(clock_freq_choice, &clock_HSdiv_val, &clock_N1div_val, &this->double_clock_configure_fft_frequency);

	//printf("get_SI570_oscillator_hs_div_and_n1_div_values: clock_freq_choice =  %d   clock_N1div_val = 0x%04X   clock_HSdiv_val = 0x%04X  this->double_clock_configure_fft_frequency = %f\n\n", clock_freq_choice, clock_N1div_val, clock_HSdiv_val, this->double_clock_configure_fft_frequency);

	// new
	// get adc_fpga_iob_tap_delay value from library depends of ENUM: sample_rate (0:SAMPLERATE_250MSPS .... ,15:SAMPLERATE_25MSPS) and Module type (SIS3316-125, SIS3316-250, SIS3316-2-125, SIS3316-2-250)
	gl_sis3316_adc1->get_adc_fpga_iob_delay_value(clock_freq_choice, &iob_delay_value);
	//printf("get_adc_fpga_iob_delay_value: clock_freq_choice =  %d   iob_delay_value = 0x%04X   %d\n\n", clock_freq_choice, iob_delay_value, iob_delay_value);


	// reprogram internal Osc. 
	gl_sis3316_adc1->change_frequency_HSdiv_N1div(0, clock_HSdiv_val, clock_N1div_val) ;
	// reset DCM/PLL of the ADC-FPGAs
	gl_sis3316_adc1->reset_adc_fpga_sample_clock_PLL() ; 

	fNumericEntriesTapDelay->SetIntNumber(iob_delay_value); //
	// Calibrate and configure IOB _delay Logic
	gl_sis3316_adc1->configure_adc_fpga_iob_delays(iob_delay_value) ; 

	//enable ADC outputs (bit was cleared with Key-reset !)
	gl_sis3316_adc1->adc_spi_reg_enable_adc_outputs() ; 
	// Set NIM_Output_Selection
	SIS3316_Test_Write_NIM_Output_Selection() ;

	// Load coincidence Lookup Tables
	this->SIS3316_Test_Set_Coincidence_Lookup_Tables(); // Load examples

}


/*******************************************************/

void SIS3316TestDialog::Deactivate_Buttons(void)
{
	Pixel_t yellow;
	fClient->GetColorByName("yellow", yellow);
	Pixel_t green;
	fClient->GetColorByName("green", green);
	Pixel_t red;
	fClient->GetColorByName("red", red);
	fStartB->ChangeBackground(red);
	fStopB->ChangeBackground(red);
	fClockConfiguration->ChangeBackground(red);
	fTab->SetTab(0); // set active
	fStartB->SetEnabled(kFALSE); // dim
	fStopB->SetEnabled(kFALSE); // dim
	fClockConfiguration->SetEnabled(kFALSE); //  dim

}


void SIS3316TestDialog::Activate_CLK_Configuration_Button(void)
{
	Pixel_t yellow;
	fClient->GetColorByName("yellow", yellow);
	Pixel_t green;
	fClient->GetColorByName("green", green);
	Pixel_t red;
	fClient->GetColorByName("red", red);
	fStartB->ChangeBackground(red);
	fStopB->ChangeBackground(red);
	fStartB->SetEnabled(kFALSE); // dim
	fStopB->SetEnabled(kFALSE); // dim
	fClockConfiguration->SetEnabled(kTRUE); // not dim
	fClockConfiguration->ChangeBackground(green);
	fTab->SetTab(7); // set active
}


/*******************************************************/

 

int SIS3316TestDialog::SIS3316_Test_Set_Coincidence_Lookup_Tables(void)
{
	unsigned int i, j;
	unsigned int pollcounter = 0;
	unsigned int data;
	unsigned int table_address;
	unsigned int table_data;
	unsigned int *table_data_arrary;
	unsigned int sum;
	unsigned int coincidence_example_choice;



	coincidence_example_choice = fCombo_CoincidenceLookupTableMode->GetSelected();
	if (coincidence_example_choice == 0) { // disable "Load Coincidence Lookup Table"
		return 0;
	}

	//  Clear Trigger Coincidence Lookup Tables (clear both tables)
	gl_sis3316_adc1->register_write(SIS3316_LOOKUP_TABLE_CONTROL_REG, 0x80001f1f); //clear Lookup table , takes appr. 525 us // pulse length 32 sample clocks


	// wait for "clear Lookup table finished"
	do {
		gl_sis3316_adc1->register_read(SIS3316_LOOKUP_TABLE_CONTROL_REG, &data); //poll on busy
		pollcounter++;
	} while ((pollcounter < 10000) && ((data & 0x80000000) == 0x80000000));
	//printf("clear Lookup table: pollcounter = %d\n\n", pollcounter);
	if (pollcounter >= 10000) {
		printf("ERROR: clear Lookup table: pollcounter = %d\n\n", pollcounter);
		return -1;
	}


	if (coincidence_example_choice == 1) { // disable "Load Coincidence Lookup Table"
		// Example 1
		// Coincidence validation signal = 1
		//	if ((ch 10 or ch 9) and
		//		(ch 8 or ch 7 or ch 6 or ch 5 or ch 4 or ch 3 or ch 2 or ch 1))
		// ch 16 or ch 11 are ignored

		table_data = 1;  // Lookup Table 1 Coincidence validation bit ; 
		table_address = 0x101; 
		gl_sis3316_adc1->register_write(SIS3316_LOOKUP_TABLE_ADDR_REG, table_address); // 
		for (i = 0x001; i < 0x0FF; i++) { // writes from address 0x101 to 0x1FF -->   (Ch 9) and (ch 8 or ch 7 or ch 6 or ch 5 or ch 4 or ch 3 or ch 2 or ch 1)
			gl_sis3316_adc1->register_write(SIS3316_LOOKUP_TABLE_DATA_REG, table_data); // 
		}
		table_address = 0x201;
		gl_sis3316_adc1->register_write(SIS3316_LOOKUP_TABLE_ADDR_REG, table_address); // 
		for (i = 0x001; i < 0x0FF; i++) { // writes from address 0x201 to 0x2FF -->   (Ch 10) and (ch 8 or ch 7 or ch 6 or ch 5 or ch 4 or ch 3 or ch 2 or ch 1)
			gl_sis3316_adc1->register_write(SIS3316_LOOKUP_TABLE_DATA_REG, table_data); // 
		}

		table_address = 0x301;
		gl_sis3316_adc1->register_write(SIS3316_LOOKUP_TABLE_ADDR_REG, table_address); // 
		for (i = 0x001; i < 0x0FF; i++) { // writes from address 0x301 to 0x3FF -->   (Ch 10 and Ch 9) and (ch 8 or ch 7 or ch 6 or ch 5 or ch 4 or ch 3 or ch 2 or ch 1)
			gl_sis3316_adc1->register_write(SIS3316_LOOKUP_TABLE_DATA_REG, table_data); // 
		}
		table_address = 0x03FF0000;// use only Trigger lines from channel 1 to 10   
		gl_sis3316_adc1->register_write(SIS3316_LOOKUP_TABLE_ADDR_REG, table_address); //     

		return 0;
	}

	table_data_arrary = (unsigned int*)malloc(0x10000 * 4);
	if (table_data_arrary == NULL) {
		printf("Error allocating table_data_arrary !\n");
	}

	if (coincidence_example_choice == 2) { //  
		// Example 2
		//	Coincidence validation signal = 1:    if at least 2 channels of ch1 to 16 are triggering 
		//	Coincidence validation signal = 2:    if at least 3 channels of ch1 to 16 are triggering 
		// prepare lookup table
		for (i = 0; i < 0x10000; i++) {
			sum = 0;
			for (j = 0; j < 16; j++) { //  
				if (((i >> j) & 0x1) == 1) {
					sum++;
				}
			}
			table_data_arrary[i] = 0;
			if (sum > 1) { // at least 2 triggers for table 1
				table_data_arrary[i] = table_data_arrary[i] + 1;
			}
			if (sum > 2) { // at least 3 triggers for table 2
				table_data_arrary[i] = table_data_arrary[i] + 2;
			}
		}

		table_address = 0x0;//  
		gl_sis3316_adc1->register_write(SIS3316_LOOKUP_TABLE_ADDR_REG, table_address); // 
		for (i = 0; i < 0x10000; i++) {
			gl_sis3316_adc1->register_write(SIS3316_LOOKUP_TABLE_DATA_REG, table_data_arrary[i]); // 
		}

		table_address = 0xFFFF0000;// use all Trigger lines from channel 1 to 16   
		gl_sis3316_adc1->register_write(SIS3316_LOOKUP_TABLE_ADDR_REG, table_address); // 


		return 0;
	}
	free(table_data_arrary);
	return 0;
}

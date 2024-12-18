//
/***************************************************************************/
/*                                                                         */
/*  Filename: sis3316_root_gui_test1.h                                     */
/*                                                                         */
/*  Funktion:                                                              */
/*                                                                         */
/*  Autor:                TH                                               */
/*  date:                 27.03.2015                                       */
/*  last modification:    03.11.2021                                       */
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
/*  � 2021                                                                 */
/*                                                                         */
/***************************************************************************/
#ifndef _SIS3316_ROOT_GUI_TEST1_
#define _SIS3316_ROOT_GUI_TEST1_


#include "sis3316_root_gui_monitor_size.h"
#include "rootIncludes.h"
#include "fftw3.h"
#include "get_configuration_parameters.h"





#define SIS3316_CHANNEL_COUNT	16


#define MAX_ROOT_PLOT_LENGTH				0x100000		// 1 Msample

#define MAX_NUMBER_LWORDS_64MBYTE			0x1000000       /* 16M x 32-bit words ->  64MByte */

#define SINGLE_EVENT_CH_BUFFER_LENGTH		(MAX_NUMBER_LWORDS_64MBYTE/SIS3316_CHANNEL_COUNT)		// 4MByte -> 2 Msample -> use max. 1Msample

//#define ADC_BUFFER_LENGTH                   0x100000		// 1 Msamples / 2MByte



//#define MAX_ROOT_PLOT_LENGTH 0x200000 // 2.097.152
//#define MAX_SAMPLE_LENGTH 0x10000-2 //
#define MAX_SAMPLE_LENGTH	MAX_ROOT_PLOT_LENGTH //  new 27.5.2016


#define MAX_ROOT_PLOT_MAW_LENGTH 2048 //

//#define MAX_PRETRIGGER_DELAY 2042 // ADC FPGA Version A006 and lower
#define MAX_PRETRIGGER_DELAY 16378 // ADC FPGA Version A007 and higher


#define WRITE_DTATA_TO_FILE_MAX_32BIT_WORDS	 0x10000000   // 256M 32-bit words -> 1GByte	




#define DefineCanvasBackgroundColor   10
#define DefineChannel_1_Color          2
#define DefineChannel_2_Color          3
#define DefineChannel_3_Color          4
#define DefineChannel_4_Color          6
#define DefineChannel_5_Color          7
#define DefineChannel_6_Color          8
#define DefineChannel_7_Color          9
#define DefineChannel_8_Color         28
#define DefineChannel_9_Color         30
#define DefineChannel_10_Color        40
#define DefineChannel_11_Color        41
#define DefineChannel_12_Color        42
#define DefineChannel_13_Color        43
#define DefineChannel_14_Color        44
#define DefineChannel_15_Color        45
#define DefineChannel_16_Color        46

int SIS3316_WriteBankChannelHeaderToDataFile (FILE *file_data_ptr, unsigned int indentifier, unsigned int bank_loop_no, unsigned int channel_no, unsigned int nof_events, unsigned int event_length, unsigned int maw_length, unsigned int reserved);
int SIS3316_WriteBankChannelEventBufferToDataFile (FILE *file_data_ptr, unsigned int* memory_data_array, unsigned int nof_write_length_lwords);


/*************************************************************************************************************************/
/*************************************************************************************************************************/
/*************************************************************************************************************************/
/*************************************************************************************************************************/
/*************************************************************************************************************************/

#define SIS3316Test_MAX_TABLE_TABS   10
enum ETest1CommandIdentifiers {
   TEST1_FILE_EXIT,
   M_LOAD_CONFIGURATION_DLG,
   M_SAVE_CONFIGURATION_DLG,
   TEST1_HELP_ABOUT 
};

class SIS3316TestDialog : public TGTransientFrame {

private:
	Bool_t *fBSetup;			// shows if setup is open
	Bool_t *fBTest1_Run_Busy;	// shows if test is running

 
   TGMenuBar          *fMenuBar;
   TGPopupMenu        *fMenuFile, *fMenuConfiguration, *fMenuHelp;
   TGLayoutHints      *fMenuBarLayout, *fMenuBarItemLayout, *fMenuBarHelpLayout;
   
	TGCompositeFrame   *fTGHor_tab1a, *fTGHor_tab1b;
	TGHorizontalFrame	*fTGHor_tab1a_grp1b_sub[4];
	TGHorizontalFrame	*fTGHor_tab1a_grp1c_sub[4];
	TGHorizontalFrame	*fTGHor_tab1a_grp1d_sub[4];
	TGHorizontalFrame	*fTGHor_tab1a_grp1e_sub[4];
	TGHorizontalFrame	*fTGHor_tab1a_grp1f_sub[4];
	TGHorizontalFrame	*fTGHor_tab1a_grp1g_sub[4];
	TGHorizontalFrame	*fTGHor_tab1a_grp1h_sub[4];

	TGHorizontalFrame	*fFrame1_main;
	

//	TGCompositeFrame   *fFrame2;
	TGVerticalFrame		*fFrame1;
	TGVerticalFrame		*fFrame2;
	//TGHorizontalFrame* fFrame1, * fTGHor_frame1, * fTGHor_frame1a, * fTGHor_frame2, * fTGHor_tab4a;
	TGVerticalFrame		*fTGVer_frame1;

	TGHorizontalFrame	*fTGHor_frame1, *fTGHor_frame1a, *fTGHor_frame2, *fTGHor_tab4a;
	TGHorizontalFrame   *fTGHor_tab3_2_sub[20]  ;
	TGHorizontalFrame   *fTGHor_tab4a_1a_sub[16]  ;
	TGHorizontalFrame   *fTGHor_tab4a_1b_sub[16]  ;

	TGCompositeFrame	*fFVer0;

	TGCompositeFrame    *fF0,  *fF1, *fF1A, *fF1B, *fF1C, *fF2, *fF3, *fF4, *fF4A, * fF5, *fF5AA, *fF5A, *fF5B, *fF5Br, *fF5Br1, *fF5C, *fF5D , *fF5E   ;
   TGCompositeFrame    *fF_tab1, *fF_tab1a, *fF_tab2, *fF_tab2b, *fF_tab3, *fF_tab3a, *fF_tab4, *fF_tab5, *fF_tab6, *fF_tab7, *fF_tab8;
   TGGroupFrame			*fGrp1, *fGrp1a;
   TGGroupFrame			*fF_tab1_fGrp0, *fF_tab1_fGrp1, *fF_tab1_fGrp1A, *fF_tab1_fGrp1B, *fF_tab1_fGrp2, *fF_tab1_fGrp3 ; //* fF_tab1_fGrp4, * fF_tab1_fGrp4b
   TGGroupFrame			*fF_tab1a_fGrp1a, *fF_tab1a_fGrp1ar, *fF_tab1a_fGrp1ar1, *fF_tab1a_fGrp1b, *fF_tab1a_fGrp1c, *fF_tab1a_fGrp1d, *fF_tab1a_fGrp1e, *fF_tab1a_fGrp1f, *fF_tab1a_fGrp1g, *fF_tab1a_fGrp1h;
   
   TGGroupFrame			*fF_tab2_fGrp1, *fF_tab2_fGrp2, *fF_tab2_fGrp3, *fF_tab2_fGrp4, *fF_tab2_fGrp5;
   TGGroupFrame			*fF_tab2b_fGrp1 ;
   TGGroupFrame			*fF_tab3_fGrp1, *fF_tab3_fGrp2, *fF_tab3_fGrp3, *fF_tab3_fGrp4;
   TGGroupFrame			*fF_tab3a_fGrp1, *fF_tab3a_fGrp2 ;
   TGGroupFrame			*fF_tab4_fGrp1, *fF_tab4_fGrp1A, *fF_tab4_fGrp1B, *fF_tab4_fGrp2, *fF_tab4_fGrp3, *fF_tab4_fGrp4;
   TGGroupFrame			*fF_tab5_fGrp1, *fF_tab5_fGrp1A, *fF_tab5_fGrp2, *fF_tab5_fGrp3;
   TGGroupFrame			*fF_tab6_fGrp1, *fF_tab6_fGrp2, *fF_tab6_fGrp3;
 //  TGGroupFrame			*fF_tab6_fGrp4, *fF_tab6_fGrp5, *fF_tab6_fGrp6;

   TGHorizontalFrame	*fF_tab6_fHor1, *fF_tab6_fHor2, *fF_tab6_fHor3;

   TGHorizontalFrame* fTGHor_tab6_1_sub[17];
   TGHorizontalFrame* fTGHor_tab6_2_sub[17];
   TGHorizontalFrame* fTGHor_tab6_3_sub[17];

   TGTextView			*fTextView; 


 	TGTabElement		*tabel_tab[SIS3316Test_MAX_TABLE_TABS] ;
	unsigned int		sis3316Test1_nof_valid_tabel_tabs ;

   TGHorizontalFrame    *fTGHorizontalFrame;
   TGVerticalFrame		*fTGVerticalFrame;

   TGHorizontalFrame    *fF[16];
   TGVerticalFrame		*fVF[16];
   TGGroupFrame        *fF6, *fF7;
   TGButton            *fStartB, *fStopB, *fClockConfiguration ;
	Pixel_t				fClockConfiguration_background_color; 

	TGTextEntry			*fTextEntryDataFilePath;
	TGTextButton		*fTextButtonDataFilePath;

   TGButton            *fChkFP_BUS_ClockMaster;
   TGButton				*fChkExternalTriggerFunc, *fChkKeyTrigger, *fChkLemoInTiEnable, *fChkExternalTriggerDisableWithBusyEnable;
   TGButton				*fChkFeedbackInternalTriggerEnable, *fChkFeedbackCoincidence1TriggerEnable;


   TGButton				*fChkExternalTriggerFuncAsVeto, *fChkLocalVetoFuncAsVeto, *fChkLemoInUiAsVetoEnable;

   TGHorizontalFrame	*fF_tab1a_fGrp1ar_Veto_Delay;
   
   TGNumberEntry		*fNumericEntries_VetoDelay;

   TGLabel				*fLabel_VetoDelay;

   TGHorizontalFrame	*fF_tab1a_fGrp1ar_InternalDelay;
   TGNumberEntry		*fNumericEntries_InternalDelay;
   TGLabel				*fLabel_InternalDelay;
   
   TGButton				*fChkDisplayAutoZoom;
   TGButton				*fChkDisplayDisableDeleteGraph;
   TGButton            *fDisplayEnableCh_Set, *fDisplayEnableCh_Clr;

   
   TGButton				*fChkDisplayStatisticCounters;

   TGButton            *fChkDisplayAdc[16];
   TGButton            *fChkHistoSum;
   TGButton            *fChkHistoZoomMean;
   TGButton            *fChkHistoGaussFit;
   TGButton            *fChkFFT_Db, *fChkFFT_AutoScale, *fChkFFT_Sum, *fChkFFTLogY;


   TGButton            *fInvertChannel_Set, *fInvertChannel_Clr;
   TGButton            *fChkInvertChannel[16];

   TGLabel			   *fLabel_TerminationChannel ;
   TGButton            *fTerminationChannel_Set, *fTerminationChannel_Clr;
   TGButton            *fChkTerminationChannel[16];

   TGLabel			   *fLabel_InputRange0Channel ;
   TGButton            *fInputRange0Channel_Set, *fInputRange0Channel_Clr;
   TGButton            *fChkInputRange0Channel[16];

   TGButton            *fChkTriggerHeSuppressMode;
   TGButton            *fChkTriggerEnableCh[16+4];
   TGButton			   *fTriggerEnableCh_Set, *fTriggerEnableCh_Clr;

   TGButton				*fIntFeedbackTriggerEnableCh_Set, *fIntFeedbackTriggerEnableCh_Clr;
   TGButton				*fChkIntFeedbackTriggerEnableCh[16];


   TGButton            *fExtTriggerEnableCh_Set, *fExtTriggerEnableCh_Clr;
   TGButton			   *fChkExtTriggerEnableCh[16];

   TGButton				*fIntTriggerEnableCh_Set, *fIntTriggerEnableCh_Clr;
   TGButton				*fChkIntTriggerEnableCh[16];

   TGButton				*fIntSumTriggerEnableCh_Set, *fIntSumTriggerEnableCh_Clr;
   TGButton				*fChkIntSumTriggerEnableCh[16];

   TGButton				*fIntPileupTriggerEnableCh_Set, *fIntPileupTriggerEnableCh_Clr;
   TGButton				*fChkIntPileupTriggerEnableCh[16];

   TGButton				*fExtGateEnableCh_Set, *fExtGateEnableCh_Clr;
   TGButton				*fChkExtGateEnableCh[16];

   TGButton				*fExtVetoEnableCh_Set, *fExtVetoEnableCh_Clr;
   TGButton				*fChkExtVetoEnableCh[16];

   TGButton				*fChkLemoOutCoEnableCh[32];
   TGButton				*fChkLemoOutToEnableCh[32];
   TGButton				*fChkLemoOutUoEnableCh[32];


	TGComboBox          *fCombo_InternalTriggerCfdSelection;

    TGButton            *fChkInput_50Ohm, *fChkInput_5V_Range, *fChkDacInrementTest;

	TGButton			*fChkStopAfterTime;
	TGNumberEntry		*fNumericEntriesStopAfterTime;
	TGTextEntry			*fTextEntryTimeSecCounterView;

	TGNumberEntry		*fNumericEntriesTimeSecCounterView;
	TGLabel				*fLabel_TimeSecCounterView;

	TGNumberEntry		*fNumericEntriesBankLoopCounterView;
	TGLabel				*fLabel_BankLoopCounterView;

	
	TGNumberEntry		*fNumericEntriesTemperatureView;
	TGLabel				*fLabel_TemperatureView;


	TGButton            *fChkStopAfterBanks;
	TGNumberEntry       *fNumericEntriesStopAfterBanks;

	TGButton            *fChkWriteDataToFile, *fChkWriteMultipleFiles;
	TGFileInfo			fileInfoConfFile;
	TGFileInfo			fileInfoDataFile;

	TGButton            *fChkNofEvents_ProBank;
	TGNumberEntry		*fNumericEntries_SampleControl_NofEvents_ProBank;
	TGNumberEntry		*fNumericEntries_SampleControl_PossibleMaxNofEvents_ProBank;
	TGNumberEntry       *fNumericEntries_SampleControl_MaxNofEvents_ProBank;

	TGButton			*fChk_SuppressEventsIfAddrThresFlag;

	TGButton            *fChk_EventHitParameter_DataFormatBit0, *fChk_EventHitParameter_DataFormatBit1, *fChk_EventHitParameter_DataFormatBit2, *fChk_EventHitParameter_DataFormatBit3;
	TGButton			*fChk_SaveRawDataFirstEventOnly;
	
 
   TGPictureButton     *fPicBut1;
   TGCheckButton       *fCheck1;
   TGComboBox          *fCombo;
   TGTab               *fTab;
   TGNumberEntry       *fNumericEntries_EventHitParameter[9];
   TGLabel              *fLabel[16];
   TGLayoutHints       *fL5;

   Bool_t               fSIS3316_Test1_Run_Cmd;
   TGraph               *fGraph_ch[17];
   TLatex              	*fGraph_Text_ch[17];
   TGraph               *fGraph_maw;
   TH1I                	*iHistoAdc[16];
   TGraph               *fGraph_fft[17];

   TGComboBox          *fCombo_SampleControl_BankModus;
   TGComboBox          *fCombo_FP_BUS_ClockOutMux;

   TGComboBox          *fCombo_SampleClock_source;
   TGComboBox          *fCombo_SetInternalClockFreq;
   TGComboBox          *fCombo_SetClockMultiplierMode;

   TGComboBox			*fCombo_CoincidenceLookupTableMode;


   TGComboBox          *fCombo_SetSelectMAW_TestBuffer;


   TGComboBox          *fCombo_Display_MAW;
   TGComboBox          *fCombo_Display_Histos_Ch;
   TGComboBox          *fCombo_Display_Histos_Build;
   TGComboBox          *fCombo_Display_FFT_Ch;
   TGComboBox          *fCombo_Display_FFT_Window;

   TGComboBox          *fCombo_Set_ADC_SPI_Input_Voltage;
   TGLabel				*fLabel_fCombo_Set_ADC_SPI_text[4] ;
 
  TGComboBox          *fCombo_InternalTriggerToVMESelection;
  TGComboBox          *fCombo_InternalHeTriggerToVMESelection;
 


   TCanvas              *fCanvas1;
   TCanvas              *fCanvas2;
   TCanvas              *fCanvas3;
   TCanvas			*fCanvas4;
   TCanvas			*fCanvas5;
   TPaveText		*histo_pave_text[16];

   	
	
	TGNumberEntry       *fNumericEntriesHistogramXaxisOffset;
	TGNumberEntry       *fNumericEntriesHistogramXaxisDivider;

	TGNumberEntry		*fNumericEntriesGraph_Yaxis[2];
	TGNumberEntry		*fNumericEntriesGraph_Xaxis[2];


	TGNumberEntry		*fNumericEntriesHisto_Xaxis[2];
	


	TGNumberEntry       *fNumericEntriesAnalogOffset[16];
   TGNumberEntry       *fNumericEntriesTapDelay;

   TGNumberEntry       *fNumericEntriesTriggerPulse_length;
   TGNumberEntry       *fNumericEntriesTriggerGap;
   TGNumberEntry       *fNumericEntriesTriggerPeaking;
   TGNumberEntry       *fNumericEntriesTriggerThreshold;
   TGNumberEntry       *fNumericEntriesHeTriggerThreshold;
   TGNumberEntry       *fNumericEntriesPileup_length;
   TGNumberEntry       *fNumericEntriesRepileup_length;


   TGNumberEntry       *fNumericEntriesEnergyGap;
   TGNumberEntry       *fNumericEntriesEnergyPeaking;
   TGNumberEntry       *fNumericEntriesEnergyTauTable;
   TGNumberEntry       *fNumericEntriesEnergyTauFactor;
   TGNumberEntry       *fNumericEntriesEnergyAdditionalAverage;
   TGNumberEntry       *fNumericEntriesEnergyPickupValueIndex;

	TGNumberEntry       *fNumericEntriesAccumulatorStartIndex[8];
	TGNumberEntry       *fNumericEntriesAccumulatorLength[8];
	TGLabel				*fLabel_AccumulatorStartIndex_text[8] ;
	TGLabel				*fLabel_AccumulatorLength_text[8] ;

   TGNumberEntry       *fNumericEntriesNimOutput[3];

   
    Pixel_t tab_color_not_active  ;
    Pixel_t tab_color_active      ;


	void SIS3316_Test1();
	void SIS3316_Test_running_dim_widgets(bool dim_state) ;

	void SIS3316_Test_Write_TerminationGain(void);
	void SIS3316_Test_Write_DacOffset(void);
	void SIS3316_Test_Write_NIM_Output_Selection(void);

	void SIS3316_Test_Update_Gui_Entries(void);
	void SIS3316_Test_Calculate_MaxNofEventsEachBank(void);
	

	void SIS3316_Test_Write_Sample_Clock_Configuration();

	void LoadConfigurationFile();
	void SaveConfigurationFile();

	sis3316_get_configuration_parameters *params;

	void setGuiParameters(sis3316_get_configuration_parameters *params);
	void setAdcParameters(sis3316_get_configuration_parameters *params);
	void getGuiParameters(sis3316_get_configuration_parameters *params);
	void getAdcParameters(sis3316_get_configuration_parameters *params);

	unsigned int sample_clock_configuration_valid_flag = 0 ;
	unsigned int adc_fpga_firmware_version = 0 ;
	double  double_clock_configure_fft_frequency ;

	unsigned int invert_parameter_has_changed_flag = 0;
	unsigned int gain_termination_parameter_has_changed_flag = 0;
	unsigned int offset_parameter_has_changed_flag = 0;

	unsigned int root_chk_bank_event_nof_limit_on_flag ;

 
	int raw_graph_ymin, raw_graph_ymax;
	int raw_graph_ymax_absolute;

	int raw_graph_xmin, raw_graph_xmax;
	int raw_graph_xmax_absolute;

	int raw_sample_length;

	Int_t* root_gr_x;
	Int_t* root_gr_y;

	unsigned int* dma_data_buffer;
	unsigned short* ushort_adc_buffer_array_ptr[16]; //

	Int_t* root_gr_maw_x;
	Int_t* root_gr_maw_y;


	float* root_float_fft_x;
	float* root_float_fft_y;
	float* root_float_fft_y1;
	double* root_double_window_weight;
	double* root_double_fft_spectrum;
	int* root_int_save_adc_buffer;

	int root_histo_xmin, root_histo_xmax;
	int root_histo_xmax_absolute;

	int root_histo_length;

	char char_TextView[256];

	unsigned int uint_test_vme_base_addr;
	char char_test_config_file[512];




protected:
	Bool_t fB_openfCanvas1WindowFlag; // shows if Canvas1 window is open , Raw Data
	Bool_t fB_openfCanvas2WindowFlag; // shows if Canvas2 window is open , Histogram 
	Bool_t fB_openfCanvas3WindowFlag; // shows if Canvas3 window is open , FFT
	Bool_t fB_openfCanvas4WindowFlag; // shows if Canvas4 window is open , FIR Filter MAW


public:
   SIS3316TestDialog(const TGWindow *p, const TGWindow *main, UInt_t w, UInt_t h, Bool_t *b, Bool_t *run_flag, unsigned int uint_main_vme_base_addr, char* char_main_config_file,
	                 UInt_t options = kVerticalFrame);
   virtual ~SIS3316TestDialog();

   virtual void CloseWindow();
   virtual Bool_t ProcessMessage(Long_t msg, Long_t parm1, Long_t parm2);

   void Deactivate_Buttons();
   void Activate_CLK_Configuration_Button();
   int SIS3316_Test_Set_Coincidence_Lookup_Tables();
   
};




/*******************************************************************************************************************************/
/***********************       SIS3316TestDialog                                       *****************************************/
	  
#define SIS3316TestDialog_kCM_BUTTON_IRQ_NO_4			4
#define SIS3316TestDialog_kCM_BUTTON_IRQ_NO_5			5
#define SIS3316TestDialog_kCM_BUTTON_IRQ_NO_6			6

#define SIS3316TestDialog_kCM_BUTTON_EXT_IRQ_NO_10			10
#define SIS3316TestDialog_kCM_BUTTON_EXT_IRQ_NO_11			11

#define SIS3316TestDialog_kCM_BUTTON_INT_IRQ_NO_12			12
#define SIS3316TestDialog_kCM_BUTTON_INT_IRQ_NO_13			13

#define SIS3316TestDialog_kCM_BUTTON_INTSUM_IRQ_NO_14		14
#define SIS3316TestDialog_kCM_BUTTON_INTSUM_IRQ_NO_15		15

#define SIS3316TestDialog_kCM_BUTTON_INTPILE_IRQ_NO_16		16
#define SIS3316TestDialog_kCM_BUTTON_INTPILE_IRQ_NO_17		17

#define SIS3316TestDialog_kCM_BUTTON_INTFEEDBACK_IRQ_NO_18	18
#define SIS3316TestDialog_kCM_BUTTON_INTFEEDBACK_IRQ_NO_19	19

#define SIS3316TestDialog_kCM_BUTTON_EXTGATE_IRQ_NO_20		20
#define SIS3316TestDialog_kCM_BUTTON_EXTGATE_IRQ_NO_21		21

#define SIS3316TestDialog_kCM_BUTTON_EXTVETO_IRQ_NO_22		22
#define SIS3316TestDialog_kCM_BUTTON_EXTVETO_IRQ_NO_23		23


#define SIS3316TestDialog_kCM_BUTTON_IRQ_NO_30			30
#define SIS3316TestDialog_kCM_BUTTON_IRQ_NO_31			31

#define SIS3316TestDialog_kCM_BUTTON_IRQ_NO_42			42

#define SIS3316TestDialog_kCM_BUTTON_IRQ_NO_60			60


#define SIS3316TestDialog_kCM_BUTTON_IRQ_NO_70			70
#define SIS3316TestDialog_kCM_BUTTON_IRQ_NO_71			71

#define SIS3316TestDialog_kCM_BUTTON_IRQ_NO_80			80
#define SIS3316TestDialog_kCM_BUTTON_IRQ_NO_81			81
#define SIS3316TestDialog_kCM_BUTTON_IRQ_NO_82			82
#define SIS3316TestDialog_kCM_BUTTON_IRQ_NO_83			83


#define SIS3316TestDialog_kCM_CHECKBUTTON_IRQ_NO_9		9
#define SIS3316TestDialog_kCM_CHECKBUTTON_IRQ_NO_10		10
#define SIS3316TestDialog_kCM_CHECKBUTTON_IRQ_NO_11		11
#define SIS3316TestDialog_kCM_CHECKBUTTON_IRQ_NO_12		12

//#define SIS3316TestDialog_kCM_CHECKBUTTON_IRQ_NO_20		20
#define SIS3316TestDialog_kCM_CHECKBUTTON_IRQ_NO_21		21

#define SIS3316TestDialog_kCM_CHECKBUTTON_IRQ_NO_40		40

#define SIS3316TestDialog_kCM_CHECKBUTTON_IRQ_NO_60		60
#define SIS3316TestDialog_kCM_CHECKBUTTON_IRQ_NO_61		61

#define SIS3316TestDialog_kCM_CHECKBUTTON_IRQ_NO_74		74
#define SIS3316TestDialog_kCM_CHECKBUTTON_IRQ_NO_84		84

#define SIS3316TestDialog_kCM_COMBOBOX_IRQ_NO_30		30   
#define SIS3316TestDialog_kCM_COMBOBOX_IRQ_NO_40		40   
#define SIS3316TestDialog_kCM_COMBOBOX_IRQ_NO_90		90   

#define SIS3316TestDialog_kCM_CHECKBUTTON_IRQ_NO_100	100   
#define SIS3316TestDialog_kCM_CHECKBUTTON_IRQ_NO_101	101   
#define SIS3316TestDialog_kCM_CHECKBUTTON_IRQ_NO_102	102   



#define SIS3316TestDialog_kCM_ENTRY_IRQ_NO_11		    11  

#define SIS3316TestDialog_kCM_ENTRY_IRQ_NO_20		    20  
#define SIS3316TestDialog_kCM_ENTRY_IRQ_NO_21		    21  
#define SIS3316TestDialog_kCM_ENTRY_IRQ_NO_22		    22  
#define SIS3316TestDialog_kCM_ENTRY_IRQ_NO_23		    23  
#define SIS3316TestDialog_kCM_ENTRY_IRQ_NO_24		    24  
#define SIS3316TestDialog_kCM_ENTRY_IRQ_NO_25		    25  


#define SIS3316TestDialog_kCM_ENTRY_IRQ_NO_40		    40  
#define SIS3316TestDialog_kCM_ENTRY_IRQ_NO_60		    60  
#define SIS3316TestDialog_kCM_ENTRY_IRQ_NO_61		    61 
#define SIS3316TestDialog_kCM_ENTRY_IRQ_NO_90		    90


//const char *const SIS3316TestDialog::entrySampleControl_BankModus[2] = {
const char *const entrySampleControl_BankModus[2] = {
   "Single Event/Hit - Single Bank Mode",
   "Multi  Event/Hit - Double Bank Mode "
};




const char *const entryFP_BUS_ClockOutMux[2] = {
   "Use Internal Programmable Clock Oscillator",
   "Use External NIM Clock (via Clock Multiplier) "
};

const char *const entryClock_source[4] = {
   "Use Internal Programmable Clock Oscillator",
   "Use VXS Sample Clock",
   "Use FP-Bus Sample Clock",
   "Use External NIM Clock (via Clock Multiplier) "
};


const char *const entryClock_freq[16] = {
   "Set Internal Sample Clock to 250.000 MHz",
   "Set Internal Sample Clock to 227.273 MHz",
   "Set Internal Sample Clock to 208,333 MHz",
   "Set Internal Sample Clock to 178,571 MHz",
   "Set Internal Sample Clock to 166.667 MHz",
   "Set Internal Sample Clock to 138.889 MHz",
   "Set Internal Sample Clock to 125.000 MHz",
   "Set Internal Sample Clock to 119.048 MHz",
   "Set Internal Sample Clock to 113.636 MHz",
   "Set Internal Sample Clock to 104.167 MHz",
   "Set Internal Sample Clock to 100.000 MHz",
   "Set Internal Sample Clock to  83.333 MHz",
   "Set Internal Sample Clock to  71.429 MHz",
   "Set Internal Sample Clock to  62.500 MHz",
   "Set Internal Sample Clock to  50.000 MHz",
   "Set Internal Sample Clock to  25.000 MHz"
};

const char *const entryClock_multiplier_modes[9] = {
   "Set Clock Multiplier: Bypass",  //
   "Set Clock Multiplier: 10 MHz to 250 MHz",  // bw=0    n1_hs=5   n1_clk = 4   n2=500  n3=1   range 10,00 to 11,34 -> 250 to 283
   "Set Clock Multiplier: 20 MHz to 250 MHz",  // bw=0/1  n1_hs=5   n1_clk = 4   n2=250  n3=1   range 19,40 to 22,68 -> 242 to 283
   "Set Clock Multiplier: 50 MHz to 250 MHz",  // bw=1/2  n1_hs=11  n1_clk = 1   n2=110  n3=1   range 44,00 to 51,54 -> 220 to 257
   "Set Clock Multiplier: 10 MHz to 125 MHz",  // bw=0    n1_hs=4   n1_clk = 10  n2=500  n3=1   range 10,00 to 11,34 -> 125 to 141
   "Set Clock Multiplier: 20 MHz to 125 MHz",  // bw=0/1  n1_hs=5   n1_clk = 8   n2=250  n3=1   range 19,40 to 22,68 -> 121 to 141
   "Set Clock Multiplier: 50 MHz to 125 MHz",   // bw=1/2  n1_hs=5   n1_clk = 8   n2=100  n3=1   range 48,50 to 56,70 -> 121 to 141
   "Set Clock Multiplier: 12.5 MHz to 250 MHz",  // bw=0    n1_hs=11   n1_clk = 2   n2=440  n3=1   range 11,02 to 12,87 -> 220 to 257
   "Set Clock Multiplier: 12.5 MHz to 125 MHz"   // bw=0    n1_hs=7   n1_clk = 6   n2=420  n3=1   range 11,55 to 13,50 -> 115 to 135
};


const char* const entryCoincidenceLookupTableMode[3] = {
   "Disable Coincidence Lookup Tables",
   "Load Example 1 into Lookup Table 1",
   "Load Example 2 into Lookup Table 1/2" 
};



const char *const entryClock_SelectMAW_TestBuffer[2] = {
   "Select Trigger MAW",  //
   "Select Energy  MAW"   //
};



const char *const numlabel[9] = {
   "Pre Trigger Delay",
   "Raw Data Sample Start Index",
   "Raw Data Sample Length",
   "MAW Test Buffer Pre Trigger Delay",
   "MAW Test Start Index",
   "MAW Test Buffer Length",
   "Info: Event Length (32-bit words)",
   "Info: Active Trigger Gate Length",
   "Info: Address Threshold"
};

const char *const entryInternalTriggerCfdSelection[3] = {
   "Select -CFD function disable-",
   "Select -CFD function enable with Zero crossing-",
   "Select -CFD function enable with 50% crossing-"
};

 
const char *const entryInternalTriggerToVMESelection[3] = {
   "Select -Internal Trigger-              as -Internal Trigger- to VME FPGA",
   "Select -Internal HE-Trigger-       as -Internal Trigger- to VME FPGA",
   "Select -Pileup detection pulse- as -Internal Trigger- to VME FPGA"
};
const char *const entryInternalHeTriggerToVMESelection[2] = {
   "Select -Internal HE-Trigger-        as -Internal HE-Trigger- to VME FPGA",
   "Select -Pileup detection pulse- as -Internal HE-Trigger- to VME FPGA"
};


const Double_t numinit[8] = {
   1, 2, 3, 4, 5, 6,
   7,
   8
};


// graph
const char *const chkDisAdcLabel[16] = {
   "Ch 1",
   "Ch 2",
   "Ch 3",
   "Ch 4",
   "Ch 5",
   "Ch 6",
   "Ch 7",
   "Ch 8",
   "Ch 9",
   "Ch 10",
   "Ch 11",
   "Ch 12",
   "Ch 13",
   "Ch 14",
   "Ch 15",
   "Ch 16"
};



// MAW (Moving Average Window)
const char *const entryMawLabel[17] = {
   "No MAW",
   "Display MAW Ch 1",
   "Display MAW Ch 2",
   "Display MAW Ch 3",
   "Display MAW Ch 4",
   "Display MAW Ch 5",
   "Display MAW Ch 6",
   "Display MAW Ch 7",
   "Display MAW Ch 8",
   "Display MAW Ch 9",
   "Display MAW Ch 10",
   "Display MAW Ch 11",
   "Display MAW Ch 12",
   "Display MAW Ch 13",
   "Display MAW Ch 14",
   "Display MAW Ch 15",
   "Display MAW Ch 16"
};

// graph
const char *const AdcHistogramLabel[16] = {
   "Histogram Ch 1",
   "Histogram Ch 2",
   "Histogram Ch 3",
   "Histogram Ch 4",
   "Histogram Ch 5",
   "Histogram Ch 6",
   "Histogram Ch 7",
   "Histogram Ch 8",
   "Histogram Ch 9",
   "Histogram Ch 10",
   "Histogram Ch 11",
   "Histogram Ch 12",
   "Histogram Ch 13",
   "Histogram Ch 14",
   "Histogram Ch 15",
   "Histogram Ch 16"
};




// histogram build option
const char *const entryHistoDisplayOption[3] = {
	"Histogramming: ADC Output Code Histogram",
	"Histogramming: Energy FIR Filter  ",
	"Histogramming: Accumulator (reserved)"
//   "Read internal FIR Energy Histogram"
};


// histogram channel
const char *const entryHistoLabel[19] = {
   "No Histogramming",
   "Histogramming but not display",
   "Display Histogram Ch 1",
   "Display Histogram Ch 2",
   "Display Histogram Ch 3",
   "Display Histogram Ch 4",
   "Display Histogram Ch 5",
   "Display Histogram Ch 6",
   "Display Histogram Ch 7",
   "Display Histogram Ch 8",
   "Display Histogram Ch 9",
   "Display Histogram Ch 10",
   "Display Histogram Ch 11",
   "Display Histogram Ch 12",
   "Display Histogram Ch 13",
   "Display Histogram Ch 14",
   "Display Histogram Ch 15",
   "Display Histogram Ch 16",
   "Display Histogram Ch 1-16"
};


const char *const entryDisplayFFTLabel[17] = {
   "No FFT",
   "Display FFT Ch 1",
   "Display FFT Ch 2",
   "Display FFT Ch 3",
   "Display FFT Ch 4",
   "Display FFT Ch 5",
   "Display FFT Ch 6",
   "Display FFT Ch 7",
   "Display FFT Ch 8",
   "Display FFT Ch 9",
   "Display FFT Ch 10",
   "Display FFT Ch 11",
   "Display FFT Ch 12",
   "Display FFT Ch 13",
   "Display FFT Ch 14",
   "Display FFT Ch 15",
   "Display FFT Ch 16"
};


const char *const entryDisplayFFTWindowLabel[6] = {
   "Rectangular window (no window)",
   "Hamming window",
   "Hann window",
   "Blackmann window",
   "Blackmann-Harris window",
   "No"
};


const char *const accuStartIndexlabel[8] = {
   "Start Index of Accumulator 1",
   "Start Index of Accumulator 2",
   "Start Index of Accumulator 3",
   "Start Index of Accumulator 4",
   "Start Index of Accumulator 5",
   "Start Index of Accumulator 6",
   "Start Index of Accumulator 7",
   "Start Index of Accumulator 8"
};

const char *const accuLengthlabel[8] = {
   "Length of Accumulator 1",
   "Length of Accumulator 2",
   "Length of Accumulator 3",
   "Length of Accumulator 4",
   "Length of Accumulator 5",
   "Length of Accumulator 6",
   "Length of Accumulator 7",
   "Length of Accumulator 8"
};


const char *const entryADC_SPI_InputVoltage[3] = {
   "ADC chip full scale 1.50V input range",
   "ADC chip full scale 1.75V input range",
   "ADC chip full scale 2.00V input range"
};




// graph
const char *const chkTriggerEnableChLabel[16+4] = {
   "Ch 1",
   "Ch 2",
   "Ch 3",
   "Ch 4",
   "Ch 5",
   "Ch 6",
   "Ch 7",
   "Ch 8",
   "Ch 9",
   "Ch 10",
   "Ch 11",
   "Ch 12",
   "Ch 13",
   "Ch 14",
   "Ch 15",
   "Ch 16",
   "Ch_Sum 1 to 4",
   "Ch_Sum 5 to 8",
   "Ch_Sum 9 to 12",
   "Ch_Sum 13 to 16"
};



const char *const chkChLabel[16] = {
   "Ch 1",
   "Ch 2",
   "Ch 3",
   "Ch 4",
   "Ch 5",
   "Ch 6",
   "Ch 7",
   "Ch 8",
   "Ch 9",
   "Ch 10",
   "Ch 11",
   "Ch 12",
   "Ch 13",
   "Ch 14",
   "Ch 15",
   "Ch 16"
};


const char* const chkLemoOutCoLabel[32] = {
   " 0 Sample Clock",
   " 1 reserved  ",
   " 2 reserved  ",
   " 3 reserved  ",
   " 4 reserved  ",
   " 5 reserved  ",
   " 6 reserved  ",
   " 7 reserved  ",
   " 8 reserved  ",
   " 9 reserved ",
   "10 reserved",
   "11 reserved ",
   "12 reserved ",
   "13 reserved",
   "14 reserved ",
   "15 reserved",
   "16 High Energy Trigger pulse ch1-4",
   "17 High Energy Trigger pulse ch5-8",
   "18 High Energy Trigger pulse ch9-12",
   "19 High Energy Trigger pulse ch13-16",
   "20 reserved ",
   "21 reserved",
   "22 Sample Logic Bank2 flag",
   "23 Sample Logic Bankx Armed",
   "24 reserved",
   "25 reserved",
   "26 reserved",
   "27 reserved",
   "28 reserved",
   "29 reserved ",
   "30 Set      ",
   "31 reserved" 
};

const char* const chkLemoOutToLabel[32] = {
   " 0 Internal Trigger ch 1",
   " 1 Internal Trigger ch 2",
   " 2 Internal Trigger ch 3",
   " 3 Internal Trigger ch 4",
   " 4 Internal Trigger ch 5",
   " 5 Internal Trigger ch 6",
   " 6 Internal Trigger ch 7",
   " 7 Internal Trigger ch 8",
   " 8 Internal Trigger ch 9",
   " 9 Internal Trigger ch 10",
   "10 Internal Trigger ch 11",
   "11 Internal Trigger ch 12",
   "12 Internal Trigger ch 13",
   "13 Internal Trigger ch 14",
   "14 Internal Trigger ch 15",
   "15 Internal Trigger ch 16",
   "16 SUM-Trigger stretched pulse ch1-4",
   "17 SUM-Trigger stretched pulse ch5-8",
   "18 SUM-Trigger stretched pulse ch9-12",
   "19 SUM-Trigger stretched pulse ch13-16",
   "20 Sample Bank Swap Control with NIM Input TI/UI ",
   "21 Sample Logic Bankx Armed",
   "22 Sample Logic Bank2 flag",
   "23 reserved",
   "24 Coincidence Lookup Table 1 output pulse",
   "25 External Trigger to ADC FPGA 16 clocks",
   "26 External Trigger to ADC FPGA",
   "27 External Veto/Gate to ADC FPGA",
   "28 External Timestamp Clear to ADC FPGA",
   "29 reserved ",
   "30 Set      ",
   "31 Generate Pulse"
};

const char* const chkLemoOutUoLabel[32] = {
   " 0 reserved",
   " 1 Sample Logic Bankx Armed ",
   " 2 Sample Logic Busy (Ready)  ",
   " 3 Address Threshold Flag  ",
   " 4 Sample Event Active Flag  ",
   " 5 reserved  ",
   " 6 reserved  ",
   " 7 External Timestamp Clear  ",
   " 8 Sample Logic Ready  ",
   " 9 Sample Logic Not Ready ",
   "10 reserved",
   "11 reserved",
   "12 Generate Pulse (length of 4 sample clocks) ",
   "13 Generate Pulse (length of 8 sample clocks)",
   "14 Generate Pulse (length of 12 sample clocks) ",
   "15 reserved",
   "16 High Energy Trigger pulse ch1-4",
   "17 High Energy Trigger pulse ch5-8",
   "18 High Energy Trigger pulse ch9-12",
   "19 High Energy Trigger pulse ch13-16",
   "20  Sample Bank Swap Control with NIM Input TI/UI ",
   "21 Sample Logic Bankx Armed",
   "22 Sample Logic Bank2 flag",
   "23 reserved",
   "24 Coincidence Lookup Table 2 output pulse",
   "25 Prescaler Output Pulse",
   "26 External Trigger to ADC FPGA",
   "27 External Veto/Gate to ADC FPGA",
   "28 External Timestamp Clear to ADC FPGA",
   "29 reserved ",
   "30 Set      ",
   "31 Generate Pulse"
};


#endif




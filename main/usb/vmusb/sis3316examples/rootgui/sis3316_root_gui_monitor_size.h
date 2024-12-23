//
/***************************************************************************/
/*                                                                         */
/*  Filename: sis3316_root_gui_monitor_size.h                              */
/*                                                                         */
/*  Funktion:                                                              */
/*                                                                         */
/*  Autor:                TH                                               */
/*  date:                 03.11.2021                                       */
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
#ifndef _SIS3316_ROOT_GUI_MONITOR_SIZE_
#define _SIS3316_ROOT_GUI_MONITOR_SIZE_



//#define MONITOR_1920X1200
#define MONITOR_1920X1080



#ifdef MONITOR_1920X1200

	#define MAIN_WINDOW_WIDTH							490
	#define MAIN_WINDOW_HIGH							1000
	#define MAIN_WINDOW_POSTION_X						0
	#define MAIN_WINDOW_POSTION_Y						25


	#define SIS3316_TEST_WINDOW_WIDTH_RUN_SINGLE			470
	#define SIS3316_TEST_WINDOW_WIDTH_RUN_MULTI				570

	#define SIS3316_TEST_WINDOW_WIDTH						790
	#define SIS3316_TEST_WINDOW_HIGH						1100
	#define SIS3316_TEST_WINDOW_POSTION_X					20
	#define SIS3316_TEST_WINDOW_POSTION_Y					50


	#define SIS3316_RAW_DATA_WINDOW_WIDTH_SINGLE			800
	#define SIS3316_RAW_DATA_WINDOW_HIGH_SINGLE				540
	#define SIS3316_RAW_DATA_WINDOW_POSTION_X_SINGLE		490
	#define SIS3316_RAW_DATA_WINDOW_POSTION_Y_SINGLE		50

	#define SIS3316_RAW_DATA_WINDOW_WIDTH_MULTI				1300
	#define SIS3316_RAW_DATA_WINDOW_HIGH_MULTI				540
	#define SIS3316_RAW_DATA_WINDOW_POSTION_X_MULTI			590
	#define SIS3316_RAW_DATA_WINDOW_POSTION_Y_MULTI			50


	#define SIS3316_HISTOGRAM_WINDOW_WIDTH_SINGLE			620
	#define SIS3316_HISTOGRAM_WINDOW_HIGH_SINGLE			1100
	#define SIS3316_HISTOGRAM_WINDOW_POSTION_X_SINGLE		1290
	#define SIS3316_HISTOGRAM_WINDOW_POSTION_Y_SINGLE		50

	#define SIS3316_HISTOGRAM_WINDOW_WIDTH_MULTI			1300
	#define SIS3316_HISTOGRAM_WINDOW_HIGH_MULTI				530
	#define SIS3316_HISTOGRAM_WINDOW_POSTION_X_MULTI		590
	#define SIS3316_HISTOGRAM_WINDOW_POSTION_Y_MULTI		620


	#define SIS3316_MAW_WINDOW_WIDTH						800
	#define SIS3316_MAW_DATA_WINDOW_HIGH					530
	#define SIS3316_MAW_DATA_WINDOW_POSTION_X				490
	#define SIS3316_MAW_DATA_WINDOW_POSTION_Y				620

	#define SIS3316_FFT_WINDOW_WIDTH						800
	#define SIS3316_FFT_WINDOW_HIGH							530
	#define SIS3316_FFT_WINDOW_POSTION_X					490
	#define SIS3316_FFT_WINDOW_POSTION_Y					620

#endif



#ifdef MONITOR_1920X1080

	#define MAIN_WINDOW_WIDTH							480
	#define MAIN_WINDOW_HIGH							1000
	#define MAIN_WINDOW_POSTION_X						50
	#define MAIN_WINDOW_POSTION_Y						25




	#define SIS3316_TEST_WINDOW_WIDTH_RUN_SINGLE			470
	#define SIS3316_TEST_WINDOW_WIDTH_RUN_MULTI				570

	#define SIS3316_TEST_WINDOW_WIDTH						790
	#define SIS3316_TEST_WINDOW_HIGH						1000
	#define SIS3316_TEST_WINDOW_POSTION_X					80
	#define SIS3316_TEST_WINDOW_POSTION_Y					25


	#define SIS3316_RAW_DATA_WINDOW_WIDTH_SINGLE			700
	#define SIS3316_RAW_DATA_WINDOW_HIGH_SINGLE				485
	#define SIS3316_RAW_DATA_WINDOW_POSTION_X_SINGLE		550
	#define SIS3316_RAW_DATA_WINDOW_POSTION_Y_SINGLE		25

	#define SIS3316_RAW_DATA_WINDOW_WIDTH_MULTI				1200
	#define SIS3316_RAW_DATA_WINDOW_HIGH_MULTI				485
	#define SIS3316_RAW_DATA_WINDOW_POSTION_X_MULTI			650
	#define SIS3316_RAW_DATA_WINDOW_POSTION_Y_MULTI			25


	#define SIS3316_HISTOGRAM_WINDOW_WIDTH_SINGLE			640
	#define SIS3316_HISTOGRAM_WINDOW_HIGH_SINGLE			1000
	#define SIS3316_HISTOGRAM_WINDOW_POSTION_X_SINGLE		1250
	#define SIS3316_HISTOGRAM_WINDOW_POSTION_Y_SINGLE		25

	#define SIS3316_HISTOGRAM_WINDOW_WIDTH_MULTI			1200
	#define SIS3316_HISTOGRAM_WINDOW_HIGH_MULTI				485
	#define SIS3316_HISTOGRAM_WINDOW_POSTION_X_MULTI		650
	#define SIS3316_HISTOGRAM_WINDOW_POSTION_Y_MULTI		540


	#define SIS3316_MAW_WINDOW_WIDTH						700
	#define SIS3316_MAW_DATA_WINDOW_HIGH					485
	#define SIS3316_MAW_DATA_WINDOW_POSTION_X				550
	#define SIS3316_MAW_DATA_WINDOW_POSTION_Y				540

	#define SIS3316_FFT_WINDOW_WIDTH						700
	#define SIS3316_FFT_WINDOW_HIGH							485
	#define SIS3316_FFT_WINDOW_POSTION_X					550
	#define SIS3316_FFT_WINDOW_POSTION_Y					540

#endif



#endif




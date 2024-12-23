/***************************************************************************/
/*                                                                         */
/*  Filename: sis3316_cern_root_class.h                                    */
/*                                                                         */
/*  Funktion:                                                              */
/*                                                                         */
/*  Autor:                TH                                               */
/*  date:                 25.07.2012                                       */
/*  last modification:    19.08.2014                                       */
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
/*  © 2014                                                                 */
/*                                                                         */
/***************************************************************************/
#ifndef _SIS_CERN_ROOT_CLASS_
#define _SIS_CERN_ROOT_CLASS_


#include "TApplication.h"
#include <TGLabel.h>

#include "TObject.h"
#include "TH1.h"
#include "TH1D.h"
#include "TH1F.h"
#include "TH2D.h"
#include "TGraph.h"
#include "TGraph2D.h"
#include "TMultiGraph.h"
#include "TMath.h"
#include "TCanvas.h"
//#include "TRandom.h"
//#include "TThread.h"
#include <TSystem.h>
#include "TLatex.h"
#include <TGFrame.h>
#include "TPaveLabel.h"
#include "TFrame.h"
#include "TGNumberEntry.h"
#include "TRootEmbeddedCanvas.h"
#include <TGProgressBar.h>
#include <TStyle.h>

#define GRAPH_MAX_DRAW_LENGTH  0x10000 // 64K
#define GRAPH_MAX_MAW_LENGTH  0x10000 //  

//#define GRAPH_MAX_DRAW_EXTENDED_LENGTH  0x2000000 // 32M 
//#define GRAPH_MAX_DRAW_EXTENDED_LENGTH  0x1000000 // 16M 
#define GRAPH_MAX_DRAW_EXTENDED_LENGTH  0x400000 // 4M 
 



class sis_root_graph{
private:
	TCanvas *c1 ;
	TGraph *gr1;
	TLatex *gr1_text;
	int *gr_x , *gr_y ;
	float *float_gr_x , *float_gr_y ;
	int max_y_axix_value ;


public:
	int CanvasBackgroundColor ; // white
	int Channel_1_Color  ; //  
	int Channel_2_Color ; //  
	int Channel_3_Color  ; //  
	int Channel_4_Color  ; //  
	int Channel_5_Color  ; //  
	int Channel_6_Color  ; //  
	int Channel_7_Color  ; //  
	int Channel_8_Color  ; //  
	int Channel_9_Color  ; //  
	int Channel_10_Color  ; //  
	int Channel_11_Color  ; //  
	int Channel_12_Color  ; //  
	int Channel_13_Color  ; //  
	int Channel_14_Color  ; //  
	int Channel_15_Color  ; //  
	int Channel_16_Color  ; //  

public:
	sis_root_graph (char* graph_text, int graph_x, int graph_y, int graph_x_size, int graph_y_size );
	~sis_root_graph(void);
	void sis3316_set_14bit_Yaxis (void);
	void sis3316_set_16bit_Yaxis (void);
	void sis3316_draw_XYaxis (unsigned int plot_length);
	void sis3316_draw_chN (unsigned int plot_length, unsigned int* ch_rawdata_buffer, unsigned int ch_no );
	void sis3316_draw_chN_with_float_x_correction (unsigned int plot_length, unsigned int* ch_rawdata_buffer, unsigned int ch_no, float x_correction );

};





class sis_root_graph_maw{
private:
	TCanvas *c1 ;
	TGraph *gr1;
	TLatex *gr1_text;
	int gr_x[GRAPH_MAX_MAW_LENGTH] , gr_y[GRAPH_MAX_MAW_LENGTH] ;

public:
	int CanvasBackgroundColor ; // white
	int Channel_1_Color  ; //  
	int Channel_2_Color ; //  
	int Channel_3_Color  ; //  
	int Channel_4_Color  ; //  
	int Channel_5_Color  ; //  
	int Channel_6_Color  ; //  
	int Channel_7_Color  ; //  
	int Channel_8_Color  ; //  
	int Channel_9_Color  ; //  
	int Channel_10_Color  ; //  
	int Channel_11_Color  ; //  
	int Channel_12_Color  ; //  
	int Channel_13_Color  ; //  
	int Channel_14_Color  ; //  
	int Channel_15_Color  ; //  
	int Channel_16_Color  ; //  

public:
	sis_root_graph_maw (char* graph_text, int graph_x, int graph_y, int graph_x_size, int graph_y_size );
	void sis3316_draw_mawN (unsigned int plot_length, unsigned int* maw_data_buffer, unsigned int ch_no );
	void sis3316_draw_trigger_and_energy_mawN (unsigned int plot_length, unsigned int* maw_data_buffer, unsigned int ch_no, unsigned int uint_trigger_maw_offset );
	void sis3316_draw_signedInt_buffer (unsigned int plot_length, signed int* signed_data_buffer, unsigned int ch_no );

	
};


/***************************************************************************************************************/

class sis_root_intensity_graph{
private:
	TCanvas *c1 ;
	TH2 *h_intensity_graph;
	//TGraph2D *h_intensity_graph;
	TLatex *gr1_text;
	//int gr_x[GRAPH_MAX_DRAW_LENGTH] , gr_y[GRAPH_MAX_DRAW_LENGTH] ;
	//float float_gr_x[GRAPH_MAX_DRAW_LENGTH] , float_gr_y[GRAPH_MAX_DRAW_LENGTH] ;



public:
	int CanvasBackgroundColor ; // white

public:
sis_root_intensity_graph(char* graph_text, int graph_x, int graph_y, int graph_x_size, int graph_y_size,
													int nbinsx,Double_t xlow,Double_t xup,
													int nbinsy,Double_t ylow,Double_t yup);
void sis3316_add_intensity_with_float_x_correction (unsigned int plot_length, unsigned int* ch_rawdata_buffer, float x_correction );
void sis3316_draw_intensity_with_float_x_correction (void);

};



/***************************************************************************************************************/

class sis_root_channel_delay_histograms{
private:
	TCanvas *c1 ;
	TH1F   	*h_ch_delay_histogram[3];
	//int gr_x[GRAPH_MAX_DRAW_LENGTH] , gr_y[GRAPH_MAX_DRAW_LENGTH] ;
	//float float_gr_x[GRAPH_MAX_DRAW_LENGTH] , float_gr_y[GRAPH_MAX_DRAW_LENGTH] ;

	static const char *const ChannelDelayHistogramLabel[3];


public:
	int CanvasBackgroundColor ; // white

public:
sis_root_channel_delay_histograms(char* histogram_text, int histogram_x, int histogram_y, int histogram_x_size, int histogram_y_size,char* histogram1_text, char* histogram2_text, char* histogram3_text);
void sis3316_increment_delay_histogram (unsigned int delay_histogram_i, float ns_value );
void sis3316_increment_delay_draw (void );

	//sis_root_intensity_graph::sis_root_intensity_graph(char* graph_text, int graph_x, int graph_y, int graph_x_size, int graph_y_size);
//void sis3316_add_intensity_with_float_x_correction (unsigned int plot_length, unsigned int* ch_rawdata_buffer, float x_correction );
//void sis_root_intensity_graph::sis3316_draw_intensity_with_float_x_correction (unsigned int plot_length );

};




/***************************************************************************************************************/

class sis_root_channel_nG_histogram{
private:
	TCanvas *c1 ;
	TH1F   	*h_ch_nG_histogram;
	//int gr_x[GRAPH_MAX_DRAW_LENGTH] , gr_y[GRAPH_MAX_DRAW_LENGTH] ;
	//float float_gr_x[GRAPH_MAX_DRAW_LENGTH] , float_gr_y[GRAPH_MAX_DRAW_LENGTH] ;


public:
	int CanvasBackgroundColor ; // white

public:
sis_root_channel_nG_histogram(char* histogram_text, int histogram_x, int histogram_y, int histogram_x_size, int histogram_y_size);
void sis3316_increment_nG_histogram (float ns_value );
void sis3316_increment_nG_draw (void );


};



/***************************************************************************************************************/

class sis_root_channel_tof_histogram{
private:
	TCanvas *c1 ;
	TH1F   	*h_ch_tof_histogram;
	//int gr_x[GRAPH_MAX_DRAW_LENGTH] , gr_y[GRAPH_MAX_DRAW_LENGTH] ;
	//float float_gr_x[GRAPH_MAX_DRAW_LENGTH] , float_gr_y[GRAPH_MAX_DRAW_LENGTH] ;


public:
	int CanvasBackgroundColor ; // white

public:
sis_root_channel_tof_histogram(char* histogram_text, int histogram_x, int histogram_y, int histogram_x_size, int histogram_y_size);
void sis3316_increment_tof_histogram (float ns_value );
void sis3316_increment_tof_draw (void );


};



/***************************************************************************************************************/

class sis_root_channel_energy_histogram{
private:
	TCanvas *c1 ;
	TH1F   	*h_ch_energy_histogram;
	float *float_gr_x , *float_gr_y ;


public:
	int CanvasBackgroundColor ; // white

public:
sis_root_channel_energy_histogram(char* histogram_text, int histogram_x, int histogram_y, int histogram_x_size, int histogram_y_size);
~sis_root_channel_energy_histogram(void);

void sis3316_increment_energy_histogram (float energy_value );
void sis3316_draw_energy_histogram (void );



};

/***************************************************************************************************************/

class sis_root_channel_fpga_energy_histogram{
private:
	TCanvas *c1 ;
	TH1F   	*h_ch_energy_histogram;
	//float float_gr_x[GRAPH_MAX_DRAW_LENGTH] , float_gr_y[GRAPH_MAX_DRAW_LENGTH] ;
	float *float_gr_x , *float_gr_y ;


public:
	int CanvasBackgroundColor ; // white

public:
sis_root_channel_fpga_energy_histogram(char* histogram_text, int histogram_x, int histogram_y, int histogram_x_size, int histogram_y_size);
~sis_root_channel_fpga_energy_histogram(void);

void sis3316_write_histogram (unsigned int length, unsigned int* uint_histogram_array );
void sis3316_draw_energy_histogram (void );

};



/***************************************************************************************************************/


 

class sis_root_canvas_with_TH1F_histogram{
private:
	TCanvas *c1 ;
	TH1F   	*h_ch_histogram[4];
	char temp_histogram_text[128] ;

public:
	int CanvasBackgroundColor ; // white
	int nof_TH1F_histograms ; //  

public:

sis_root_canvas_with_TH1F_histogram(char* canvas_text, int canvas_x_pos, int canvas_y_pos, int canvas_x_size, int canvas_y_size,
																		int nof_histograms, char* histogram_text, int histogram_bins_x, double histogram_x_low, double histogram_x_up) ;

~sis_root_canvas_with_TH1F_histogram(void);

void sis3316_increment_histogram (int histogram_index, float increment_index );
void sis3316_draw_histogram (void );




};




	




class sis_root_extended_graph{
private:
	TCanvas *c1 ;
	TGraph *gr1;
	TLatex *gr1_text;
	int *gr_x , *gr_y ;
	int max_y_axix_value ;



public:
	int CanvasBackgroundColor ; // white
	int Channel_1_Color  ; //  
	int Channel_2_Color ; //  
	int Channel_3_Color  ; //  
	int Channel_4_Color  ; //  
	int Channel_5_Color  ; //  
	int Channel_6_Color  ; //  
	int Channel_7_Color  ; //  
	int Channel_8_Color  ; //  
	int Channel_9_Color  ; //  
	int Channel_10_Color  ; //  
	int Channel_11_Color  ; //  
	int Channel_12_Color  ; //  
	int Channel_13_Color  ; //  
	int Channel_14_Color  ; //  
	int Channel_15_Color  ; //  
	int Channel_16_Color  ; //  

public:
	sis_root_extended_graph (char* graph_text, int graph_x, int graph_y, int graph_x_size, int graph_y_size );
	~sis_root_extended_graph(void);
	void sis3316_set_14bit_Yaxis (void);
	void sis3316_set_16bit_Yaxis (void);
	void sis3316_draw_XYaxis (unsigned int plot_length);
	void sis3316_draw_chN (unsigned int plot_length, unsigned int* ch_rawdata_buffer, unsigned int ch_no );

};




#endif



 

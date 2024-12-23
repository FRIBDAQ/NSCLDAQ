/***************************************************************************/
/*                                                                         */
/*  Filename: sis3316_cern_root_class.cpp                                  */
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
//#include "stdafx.h"
#include <iostream>
#include <cstdio>
#include "sis3316_cern_root_class.h"

using namespace std;


sis_root_graph::sis_root_graph(char* graph_text, int graph_x, int graph_y, int graph_x_size, int graph_y_size)
{

int i;
	CanvasBackgroundColor = 10 ; // white
	Channel_1_Color  = 2 ; //  
	Channel_2_Color  = 3 ; //  
	Channel_3_Color  = 4 ; //  
	Channel_4_Color  = 6 ; //  
	Channel_5_Color  = 7 ; //  
	Channel_5_Color  = 2 ; //  
	Channel_6_Color  = 8 ; //  
	Channel_7_Color  = 9 ; //  
	Channel_8_Color  = 28 ; //  
	Channel_9_Color  = 41 ; //  
	Channel_10_Color  = 42 ; //  
	Channel_11_Color  = 43 ; //  
	Channel_12_Color  = 44 ; //  
	Channel_13_Color  = 45 ; //  
	Channel_14_Color  = 46 ; //  
	Channel_15_Color  = 47 ; //  
	Channel_16_Color  = 1 ; //  

	max_y_axix_value  = 0xffff;// 16 bit;
	max_y_axix_value  = 0x3fff;// 14 bit;

	gr_x = (int*) malloc(sizeof(int)*GRAPH_MAX_DRAW_LENGTH); //  
	gr_y = (int*) malloc(sizeof(int)*GRAPH_MAX_DRAW_LENGTH); //  
	float_gr_x = (float*) malloc(sizeof(float)*GRAPH_MAX_DRAW_LENGTH); //  
	float_gr_y = (float*) malloc(sizeof(float)*GRAPH_MAX_DRAW_LENGTH); //  



	for(i=0;i<GRAPH_MAX_DRAW_LENGTH;i++) {
		this->gr_x[i] = i ;
		this->gr_y[i] = 0 ;
		this->float_gr_x[i] = float(i);
	}

	//c1 = new TCanvas("c1", graph_text, graph_x, graph_y, graph_x_size, graph_y_size);
	this->c1 = new TCanvas(graph_text, graph_text, graph_x, graph_y, graph_x_size, graph_y_size);
	this->c1->SetGrid();
	this->c1->SetFillColor(CanvasBackgroundColor);
	this->c1->Divide(1); 
	this->c1->cd();

	this->gr1 = new TGraph( GRAPH_MAX_DRAW_LENGTH, this->gr_x, this->gr_y);  
	//this->gr1->SetTitle(graph_text);
	this->gr1->SetLineColor(Channel_1_Color);
	this->gr1_text = new TLatex(10,10,"Ch x");


	this->c1->Update();
}
/**************************************************************************************/
sis_root_graph::~sis_root_graph(void)
{
	free(gr_x);
	free(gr_y);
	free(float_gr_x);
	free(float_gr_y);
	delete this->c1 ;
} 

/**************************************************************************************/
void sis_root_graph::sis3316_set_14bit_Yaxis (void)
{
		this->max_y_axix_value  = 0x3fff ; // 14 bit;
}

void sis_root_graph::sis3316_set_16bit_Yaxis (void)
{
		this->max_y_axix_value  = 0xffff ; // 16 bit
}

/**************************************************************************************/

void sis_root_graph::sis3316_draw_XYaxis (unsigned int plot_length)
{
	unsigned int iBin ;
	unsigned int draw_length;

	this->c1->cd();
	draw_length = plot_length; // 
	if(draw_length > GRAPH_MAX_DRAW_LENGTH) {
		draw_length = GRAPH_MAX_DRAW_LENGTH ;
	}

	for(iBin=0; iBin<draw_length;iBin++) {
		this->gr_y[iBin] = max_y_axix_value; 
	}
	this->gr_y[draw_length/2] = 0; 

	this->c1->Clear();
	this->gr1->SetLineColor(CanvasBackgroundColor);
	this->gr1->DrawGraph(draw_length, this->gr_x, this->gr_y, "AL");   
	this->c1->Update();
	return  ;
}

void sis_root_graph::sis3316_draw_chN (unsigned int plot_length, unsigned int* ch_rawdata_buffer, unsigned int ch_no )
{
	unsigned int i, iBin;
	unsigned int module_ch_no;
	unsigned int draw_length;
	int Channel_Color;
	char Channel_Text[80];
	unsigned short* ushort_adc_buffer_ptr;
	ushort_adc_buffer_ptr = (unsigned short*) ch_rawdata_buffer ;

	sprintf(Channel_Text,"Ch %d", ch_no+1);
	module_ch_no = ch_no & 0xf ;
	switch (module_ch_no) {
		case 0: //  
		  Channel_Color = Channel_1_Color;
		  //strcpy(Channel_Text, "Ch 1");
		  break;
		case 1: //  
		  Channel_Color = Channel_2_Color;
		  //strcpy(Channel_Text, "Ch 2");
		  break;
		case 2: //  
		  Channel_Color = Channel_3_Color;
		  //strcpy(Channel_Text, "Ch 3");
		  break;
		 case 3: //  
		  Channel_Color = Channel_4_Color;
		  //strcpy(Channel_Text, "Ch 4");
		  break;
		case 4: //  
		  Channel_Color = Channel_5_Color;
		  //strcpy(Channel_Text, "Ch 5");
		  break;
		case 5: //  
		  Channel_Color = Channel_6_Color;
		  //strcpy(Channel_Text, "Ch 6");
		  break;
		case 6: //  
		  Channel_Color = Channel_7_Color;
		  //strcpy(Channel_Text, "Ch 7");
		  break;
		 case 7: //  
		  Channel_Color = Channel_8_Color;
		  //strcpy(Channel_Text, "Ch 8");
		  break;
		 case 8: //  
		  Channel_Color = Channel_9_Color;
		  //strcpy(Channel_Text, "Ch 9");
		  break;
		 case 9: //  
		  Channel_Color = Channel_10_Color;
		  //strcpy(Channel_Text, "Ch 10");
		  break;
		 case 10: //  
		  Channel_Color = Channel_11_Color;
		  //strcpy(Channel_Text, "Ch 11");
		  break;
		 case 11: //  
		  Channel_Color = Channel_12_Color;
		  //strcpy(Channel_Text, "Ch 12");
		  break;
		 case 12: //  
		  Channel_Color = Channel_13_Color;
		  //strcpy(Channel_Text, "Ch 13");
		  break;
		 case 13: //  
		  Channel_Color = Channel_14_Color;
		  //strcpy(Channel_Text, "Ch 14");
		  break;
		 case 14: //  
		  Channel_Color = Channel_15_Color;
		  //strcpy(Channel_Text, "Ch 15");
		  break;
		 case 15: //  
		  Channel_Color = Channel_16_Color;
		  //strcpy(Channel_Text, "Ch 16");
		  break;
		 default: 
		  Channel_Color = Channel_1_Color;
		  //strcpy(Channel_Text, "Ch x");
		  break;
	}

	this->c1->cd();
	draw_length = plot_length; //                ;
	if(draw_length > GRAPH_MAX_DRAW_LENGTH) {
		draw_length = GRAPH_MAX_DRAW_LENGTH ;
	}
	this->gr1->SetLineColor(Channel_Color);
	this->gr1_text->SetTextColor(Channel_Color);
	//this->gr1_text->DrawLatex(draw_length+5,(1000-(ch_no*100)),Channel_Text);
	this->gr1_text->DrawLatex(draw_length+5,(400+(module_ch_no*1000)),Channel_Text);

	iBin= 0;
	for(i=0;i<draw_length;i++) {
		this->gr_y[i] = (int) (ushort_adc_buffer_ptr[i]) ;
	}
	//this->c1->Clear();
	this->gr1->DrawGraph(draw_length, this->gr_x, this->gr_y, "L");   
	this->c1->Update();
	return  ;
}


void sis_root_graph::sis3316_draw_chN_with_float_x_correction (unsigned int plot_length, unsigned int* ch_rawdata_buffer, unsigned int ch_no, float x_correction )
{
	unsigned int i, iBin;
	unsigned int draw_length;
	int Channel_Color;
	char Channel_Text[80];
	unsigned short* ushort_adc_buffer_ptr;
	ushort_adc_buffer_ptr = (unsigned short*) ch_rawdata_buffer ;

	switch (ch_no) {
		case 0: //  
		  Channel_Color = Channel_1_Color;
		  strcpy(Channel_Text, "Ch 1");
		  break;
		case 1: //  
		  Channel_Color = Channel_2_Color;
		  strcpy(Channel_Text, "Ch 2");
		  break;
		case 2: //  
		  Channel_Color = Channel_3_Color;
		  strcpy(Channel_Text, "Ch 3");
		  break;
		 case 3: //  
		  Channel_Color = Channel_4_Color;
		  strcpy(Channel_Text, "Ch 4");
		  break;
		case 4: //  
		  Channel_Color = Channel_5_Color;
		  strcpy(Channel_Text, "Ch 5");
		  break;
		case 5: //  
		  Channel_Color = Channel_6_Color;
		  strcpy(Channel_Text, "Ch 6");
		  break;
		case 6: //  
		  Channel_Color = Channel_7_Color;
		  strcpy(Channel_Text, "Ch 7");
		  break;
		 case 7: //  
		  Channel_Color = Channel_8_Color;
		  strcpy(Channel_Text, "Ch 8");
		  break;
		 case 8: //  
		  Channel_Color = Channel_9_Color;
		  strcpy(Channel_Text, "Ch 9");
		  break;
		 case 9: //  
		  Channel_Color = Channel_10_Color;
		  strcpy(Channel_Text, "Ch 10");
		  break;
		 case 10: //  
		  Channel_Color = Channel_11_Color;
		  strcpy(Channel_Text, "Ch 11");
		  break;
		 case 11: //  
		  Channel_Color = Channel_12_Color;
		  strcpy(Channel_Text, "Ch 12");
		  break;
		 case 12: //  
		  Channel_Color = Channel_13_Color;
		  strcpy(Channel_Text, "Ch 13");
		  break;
		 case 13: //  
		  Channel_Color = Channel_14_Color;
		  strcpy(Channel_Text, "Ch 14");
		  break;
		 case 14: //  
		  Channel_Color = Channel_15_Color;
		  strcpy(Channel_Text, "Ch 15");
		  break;
		 case 15: //  
		  Channel_Color = Channel_16_Color;
		  strcpy(Channel_Text, "Ch 16");
		  break;
		 default: 
		  Channel_Color = Channel_1_Color;
		  strcpy(Channel_Text, "Ch x");
		  break;
	}

	this->c1->cd();
	draw_length = plot_length; //                ;
	if(draw_length > GRAPH_MAX_DRAW_LENGTH) {
		draw_length = GRAPH_MAX_DRAW_LENGTH ;
	}
	this->gr1->SetLineColor(Channel_Color);
	this->gr1_text->SetTextColor(Channel_Color);
	//this->gr1_text->DrawLatex(draw_length+5,(1000-(ch_no*100)),Channel_Text);
	this->gr1_text->DrawLatex(draw_length+5,(400+(ch_no*1000)),Channel_Text);

	iBin= 0;
	for(i=0;i<draw_length;i++) {
		this->float_gr_x[i] = float(i) + x_correction;
		//this->float_gr_y[i] = (int) (ushort_adc_buffer_ptr[i]) + 1000;
		this->float_gr_y[i] = (float) (ushort_adc_buffer_ptr[i]) + 1000;
	}
	//this->c1->Clear();
	this->gr1->DrawGraph(draw_length, this->float_gr_x,  this->float_gr_y, "L");   
	this->c1->Update();
	return  ;
}




/*********************************************************************************************************************************/
sis_root_graph_maw::sis_root_graph_maw(char* graph_text, int graph_x, int graph_y, int graph_x_size, int graph_y_size)
{

int i;
	CanvasBackgroundColor = 10 ; // white
	Channel_1_Color  = 2 ; //  
	Channel_2_Color  = 3 ; //  
	Channel_3_Color  = 4 ; //  
	Channel_4_Color  = 6 ; //  
	Channel_5_Color  = 7 ; //  
	Channel_5_Color  = 2 ; //  
	Channel_6_Color  = 8 ; //  
	Channel_7_Color  = 9 ; //  
	Channel_8_Color  = 28 ; //  
	Channel_9_Color  = 41 ; //  
	Channel_10_Color  = 42 ; //  
	Channel_11_Color  = 43 ; //  
	Channel_12_Color  = 44 ; //  
	Channel_13_Color  = 45 ; //  
	Channel_14_Color  = 46 ; //  
	Channel_15_Color  = 47 ; //  
	Channel_16_Color  = 48 ; //  

	for(i=0;i<GRAPH_MAX_MAW_LENGTH;i++) {
		this->gr_x[i] = i ;
		this->gr_y[i] = 0 ;
	}
	//c1 = new TCanvas("c1", graph_text, graph_x, graph_y, graph_x_size, graph_y_size);
	this->c1 = new TCanvas(graph_text, graph_text, graph_x, graph_y, graph_x_size, graph_y_size);
	this->c1->SetGrid();
	this->c1->SetFillColor(CanvasBackgroundColor);
	this->c1->Divide(1); 
	this->c1->cd();

	this->gr1 = new TGraph( GRAPH_MAX_MAW_LENGTH, this->gr_x, this->gr_y);  
	//this->gr1->SetTitle("1234");
	//this->gr1->SetTitle(graph_text);
	this->gr1->SetLineColor(Channel_1_Color);
	this->gr1_text = new TLatex(10,10,"Ch x");

	this->c1->Update();
}








void sis_root_graph_maw::sis3316_draw_mawN (unsigned int plot_length, unsigned int* maw_data_buffer, unsigned int ch_no )
{
	unsigned int i, iBin;
	unsigned int draw_length;
	int Channel_Color;
	char Channel_Text[80];

	switch (ch_no) {
		case 0: //  
		  Channel_Color = Channel_1_Color;
		  strcpy(Channel_Text, "Ch 1");
		  break;
		case 1: //  
		  Channel_Color = Channel_2_Color;
		  strcpy(Channel_Text, "Ch 2");
		  break;
		case 2: //  
		  Channel_Color = Channel_3_Color;
		  strcpy(Channel_Text, "Ch 3");
		  break;
		 case 3: //  
		  Channel_Color = Channel_4_Color;
		  strcpy(Channel_Text, "Ch 4");
		  break;
		case 4: //  
		  Channel_Color = Channel_5_Color;
		  strcpy(Channel_Text, "Ch 5");
		  break;
		case 5: //  
		  Channel_Color = Channel_6_Color;
		  strcpy(Channel_Text, "Ch 6");
		  break;
		case 6: //  
		  Channel_Color = Channel_7_Color;
		  strcpy(Channel_Text, "Ch 7");
		  break;
		 case 7: //  
		  Channel_Color = Channel_8_Color;
		  strcpy(Channel_Text, "Ch 8");
		  break;
		 case 8: //  
		  Channel_Color = Channel_9_Color;
		  strcpy(Channel_Text, "Ch 9");
		  break;
		 case 9: //  
		  Channel_Color = Channel_10_Color;
		  strcpy(Channel_Text, "Ch 10");
		  break;
		 case 10: //  
		  Channel_Color = Channel_11_Color;
		  strcpy(Channel_Text, "Ch 11");
		  break;
		 case 11: //  
		  Channel_Color = Channel_12_Color;
		  strcpy(Channel_Text, "Ch 12");
		  break;
		 case 12: //  
		  Channel_Color = Channel_13_Color;
		  strcpy(Channel_Text, "Ch 13");
		  break;
		 case 13: //  
		  Channel_Color = Channel_14_Color;
		  strcpy(Channel_Text, "Ch 14");
		  break;
		 case 14: //  
		  Channel_Color = Channel_15_Color;
		  strcpy(Channel_Text, "Ch 15");
		  break;
		 case 15: //  
		  Channel_Color = Channel_16_Color;
		  strcpy(Channel_Text, "Ch 16");
		  break;
		 default: 
		  Channel_Color = Channel_1_Color;
		  strcpy(Channel_Text, "Ch x");
		  break;
	}

	this->c1->cd();
	draw_length = plot_length; //                ;
	if(draw_length > GRAPH_MAX_DRAW_LENGTH) {
		draw_length = GRAPH_MAX_DRAW_LENGTH ;
	}
	this->gr1->SetLineColor(Channel_Color);
	this->gr1_text->SetTextColor(Channel_Color);
	//this->gr1_text->DrawLatex(draw_length+5,(1000-(ch_no*100)),Channel_Text);
	this->gr1_text->DrawLatex(draw_length+5,(400+(ch_no*1000)),Channel_Text);

	iBin= 0;
	for(i=0;i<draw_length;i++) {
		this->gr_x[i] = i ;
		this->gr_y[i] = (int) (maw_data_buffer[i] - 0x08000000) ;
	}
	//this->c1->Clear();
	this->c1->Clear();
	this->gr1->DrawGraph(draw_length, this->gr_x, this->gr_y, "AL*");   
	this->c1->Update();
	return  ;
}





void sis_root_graph_maw::sis3316_draw_trigger_and_energy_mawN (unsigned int plot_length, unsigned int* maw_data_buffer, unsigned int ch_no, unsigned int uint_trigger_maw_offset )
{
	unsigned int i, iBin;
	unsigned int draw_length;
	int Channel_Color;
	char Channel_Text[80];

	switch (ch_no) {
		case 0: //  
		  Channel_Color = Channel_1_Color;
		  strcpy(Channel_Text, "Ch 1");
		  break;
		case 1: //  
		  Channel_Color = Channel_2_Color;
		  strcpy(Channel_Text, "Ch 2");
		  break;
		case 2: //  
		  Channel_Color = Channel_3_Color;
		  strcpy(Channel_Text, "Ch 3");
		  break;
		 case 3: //  
		  Channel_Color = Channel_4_Color;
		  strcpy(Channel_Text, "Ch 4");
		  break;
		case 4: //  
		  Channel_Color = Channel_5_Color;
		  strcpy(Channel_Text, "Ch 5");
		  break;
		case 5: //  
		  Channel_Color = Channel_6_Color;
		  strcpy(Channel_Text, "Ch 6");
		  break;
		case 6: //  
		  Channel_Color = Channel_7_Color;
		  strcpy(Channel_Text, "Ch 7");
		  break;
		 case 7: //  
		  Channel_Color = Channel_8_Color;
		  strcpy(Channel_Text, "Ch 8");
		  break;
		 case 8: //  
		  Channel_Color = Channel_9_Color;
		  strcpy(Channel_Text, "Ch 9");
		  break;
		 case 9: //  
		  Channel_Color = Channel_10_Color;
		  strcpy(Channel_Text, "Ch 10");
		  break;
		 case 10: //  
		  Channel_Color = Channel_11_Color;
		  strcpy(Channel_Text, "Ch 11");
		  break;
		 case 11: //  
		  Channel_Color = Channel_12_Color;
		  strcpy(Channel_Text, "Ch 12");
		  break;
		 case 12: //  
		  Channel_Color = Channel_13_Color;
		  strcpy(Channel_Text, "Ch 13");
		  break;
		 case 13: //  
		  Channel_Color = Channel_14_Color;
		  strcpy(Channel_Text, "Ch 14");
		  break;
		 case 14: //  
		  Channel_Color = Channel_15_Color;
		  strcpy(Channel_Text, "Ch 15");
		  break;
		 case 15: //  
		  Channel_Color = Channel_16_Color;
		  strcpy(Channel_Text, "Ch 16");
		  break;
		 default: 
		  Channel_Color = Channel_1_Color;
		  strcpy(Channel_Text, "Ch x");
		  break;
	}

	this->c1->cd();
	draw_length = plot_length; //                ;
	if(draw_length > GRAPH_MAX_DRAW_LENGTH) {
		draw_length = GRAPH_MAX_DRAW_LENGTH ;
	}
	this->gr1->SetLineColor(Channel_Color);
	this->gr1_text->SetTextColor(Channel_Color);
	//this->gr1_text->DrawLatex(draw_length+5,(1000-(ch_no*100)),Channel_Text);
	this->gr1_text->DrawLatex(draw_length+5,(400+(ch_no*1000)),Channel_Text);

	iBin= 0;
	for(i=0;i<draw_length;i++) {
		this->gr_x[i] = i ;
		this->gr_y[i] = (int) (maw_data_buffer[i] - uint_trigger_maw_offset) ; //  0x08000000 or 0x0
	}
	//this->c1->Clear();
	this->c1->Clear();
	this->gr1->DrawGraph(draw_length, this->gr_x, this->gr_y, "AL");   
//	this->gr1->DrawGraph(draw_length, this->gr_x, this->gr_y, "AL*");   
	this->c1->Update();
	return  ;
}





/***************************************************************************************************************************************************************************************************************************************/

void sis_root_graph_maw::sis3316_draw_signedInt_buffer (unsigned int plot_length, signed int* signed_data_buffer, unsigned int ch_no )
{
	unsigned int i, iBin;
	unsigned int draw_length;
	int Channel_Color;
	char Channel_Text[80];

	switch (ch_no) {
		case 0: //  
		  Channel_Color = Channel_1_Color;
		  strcpy(Channel_Text, "Ch 1");
		  break;
		case 1: //  
		  Channel_Color = Channel_2_Color;
		  strcpy(Channel_Text, "Ch 2");
		  break;
		case 2: //  
		  Channel_Color = Channel_3_Color;
		  strcpy(Channel_Text, "Ch 3");
		  break;
		 case 3: //  
		  Channel_Color = Channel_4_Color;
		  strcpy(Channel_Text, "Ch 4");
		  break;
		case 4: //  
		  Channel_Color = Channel_5_Color;
		  strcpy(Channel_Text, "Ch 5");
		  break;
		case 5: //  
		  Channel_Color = Channel_6_Color;
		  strcpy(Channel_Text, "Ch 6");
		  break;
		case 6: //  
		  Channel_Color = Channel_7_Color;
		  strcpy(Channel_Text, "Ch 7");
		  break;
		 case 7: //  
		  Channel_Color = Channel_8_Color;
		  strcpy(Channel_Text, "Ch 8");
		  break;
		 case 8: //  
		  Channel_Color = Channel_9_Color;
		  strcpy(Channel_Text, "Ch 9");
		  break;
		 case 9: //  
		  Channel_Color = Channel_10_Color;
		  strcpy(Channel_Text, "Ch 10");
		  break;
		 case 10: //  
		  Channel_Color = Channel_11_Color;
		  strcpy(Channel_Text, "Ch 11");
		  break;
		 case 11: //  
		  Channel_Color = Channel_12_Color;
		  strcpy(Channel_Text, "Ch 12");
		  break;
		 case 12: //  
		  Channel_Color = Channel_13_Color;
		  strcpy(Channel_Text, "Ch 13");
		  break;
		 case 13: //  
		  Channel_Color = Channel_14_Color;
		  strcpy(Channel_Text, "Ch 14");
		  break;
		 case 14: //  
		  Channel_Color = Channel_15_Color;
		  strcpy(Channel_Text, "Ch 15");
		  break;
		 case 15: //  
		  Channel_Color = Channel_16_Color;
		  strcpy(Channel_Text, "Ch 16");
		  break;
		 default: 
		  Channel_Color = Channel_1_Color;
		  strcpy(Channel_Text, "Ch x");
		  break;
	}

	this->c1->cd();
	draw_length = plot_length; //                ;
	if(draw_length > GRAPH_MAX_DRAW_LENGTH) {
		draw_length = GRAPH_MAX_DRAW_LENGTH ;
	}
	this->gr1->SetLineColor(Channel_Color);
	this->gr1_text->SetTextColor(Channel_Color);
	//this->gr1_text->DrawLatex(draw_length+5,(1000-(ch_no*100)),Channel_Text);
	this->gr1_text->DrawLatex(draw_length+5,(400+(ch_no*1000)),Channel_Text);

	iBin= 0;
	for(i=0;i<draw_length;i++) {
		this->gr_x[i] = i ;
		this->gr_y[i] = signed_data_buffer[i] ;
	}
	//this->c1->Clear();
	this->c1->Clear();
	this->gr1->DrawGraph(draw_length, this->gr_x, this->gr_y, "AL*");   
	this->c1->Update();
	return  ;
}



/***************************************************************************************************************************************************************************************************************************************/
/***************************************************************************************************************************************************************************************************************************************/



 
sis_root_intensity_graph::sis_root_intensity_graph(char* graph_text, int graph_x, int graph_y, int graph_x_size, int graph_y_size,
													int nbinsx,Double_t xlow,Double_t xup,
													int nbinsy,Double_t ylow,Double_t yup)
{

//int i;
	CanvasBackgroundColor = 10 ; // white


	//c1 = new TCanvas("c1", graph_text, graph_x, graph_y, graph_x_size, graph_y_size);
	this->c1 = new TCanvas(graph_text, graph_text, graph_x, graph_y, graph_x_size, graph_y_size);
	this->c1->SetGrid();
	this->c1->SetFillColor(CanvasBackgroundColor);
	this->c1->Divide(1); 
	this->c1->cd();

	this->h_intensity_graph = new TH2F("h_intensity_graph","intensity plot", nbinsx, xlow, xup, nbinsy, ylow,yup); 
	gStyle->SetPalette(1);
	c1->cd(1);
	gPad->SetGrid();
	gPad->SetFillColor(CanvasBackgroundColor);

	h_intensity_graph->Reset(); //  
	h_intensity_graph->BufferEmpty(1); // action =  1 histogram is filled and buffer is deleted


	this->c1->Update();
}


void sis_root_intensity_graph::sis3316_add_intensity_with_float_x_correction (unsigned int plot_length, unsigned int* ch_rawdata_buffer, float x_correction )
{
	unsigned int i;
	unsigned int draw_length;
	unsigned short* ushort_adc_buffer_ptr;
	ushort_adc_buffer_ptr = (unsigned short*) ch_rawdata_buffer ;

	this->c1->cd();
	draw_length = plot_length; //                ;
	for(i=0;i<draw_length;i++) {
		h_intensity_graph->Fill((float(i) + x_correction), (float) ushort_adc_buffer_ptr[i] ); //  
	}
	return  ;
}


void sis_root_intensity_graph::sis3316_draw_intensity_with_float_x_correction (void)
{
	gStyle->SetPalette(1);
	this->c1->cd();
	//this->h_intensity_graph->Draw("CONTZ"); // 2-D colored
	//this->h_intensity_graph->Draw("SURF2Z"); // 2-D colored
	//this->h_intensity_graph->Draw("LEGO2"); // 2-D colored
	this->h_intensity_graph->Draw("CONT"); // 2-D colored
	//this->h_intensity_graph->Draw("COLZ"); // 2-D colored
	      //h2_ch12->Draw("CONT"); // 2-D colored
		  //h2_ch12->Draw("LEGO3 dm(1,2)  zs(0)");
	this->c1->Update();
	return  ;
}



/***************************************************************************************************************************************************************************************************************************************/
/***************************************************************************************************************************************************************************************************************************************/
/***************************************************************************************************************************************************************************************************************************************/


//const char *const sis_root_channel_delay_histograms::ChannelDelayHistogramLabel[3] = {
//   "Ch 1 to Ch2 delay",
//   "Ch 1 to Ch3 delay",
//   "Ch 1 to Ch4 delay"
//};

#define HISTOGRAM_MAX_NS_LENGTH				40    // 40ns
#define HISTOGRAM_MAX_NS_X_FACTOR			100   // steps in 10ps

sis_root_channel_delay_histograms::sis_root_channel_delay_histograms(char* histogram_text, int histogram_x, int histogram_y, int histogram_x_size, int histogram_y_size,
																	char* histogram1_text, char* histogram2_text, char* histogram3_text)
{
char *ChannelDelayHistogramLabel[3];

unsigned int i;
	CanvasBackgroundColor = 10 ; // white

	//c1 = new TCanvas("c1", graph_text, graph_x, graph_y, graph_x_size, graph_y_size);
	this->c1 = new TCanvas(histogram_text, histogram_text, histogram_x, histogram_y, histogram_x_size,histogram_y_size);
	this->c1->SetGrid();
	this->c1->SetFillColor(CanvasBackgroundColor);
	this->c1->Divide(3,1); 
	this->c1->cd();
	for (i=0;i<3;i++) {
		  this->c1->cd(1+i);
		  gPad->SetGrid();
		  gPad->SetFillColor(CanvasBackgroundColor);
	}
	ChannelDelayHistogramLabel[0] = histogram1_text ;
	ChannelDelayHistogramLabel[1] = histogram2_text ;
	ChannelDelayHistogramLabel[2] = histogram3_text ;

	for (i=0;i<3;i++) {
		this->c1->cd(1+i);
		this->h_ch_delay_histogram[i] = new TH1F("h_ch_delay_histogram[i]", ChannelDelayHistogramLabel[i], HISTOGRAM_MAX_NS_X_FACTOR * HISTOGRAM_MAX_NS_LENGTH, 0, HISTOGRAM_MAX_NS_LENGTH-1); // 0 to 40 ns
		gPad->SetGrid();
		gPad->SetFillColor(CanvasBackgroundColor);
  
		h_ch_delay_histogram[i]->Reset(); //  
		h_ch_delay_histogram[i]->BufferEmpty(1); // action =  1 histogram is filled and buffer is deleted
		h_ch_delay_histogram[i]->Draw();
		h_ch_delay_histogram[i]->SetFillColor(kRed);
	}
	this->c1->Modified();
	this->c1->Update();
}


void sis_root_channel_delay_histograms::sis3316_increment_delay_histogram (unsigned int delay_histogram_i, float ns_value )
{
	if(delay_histogram_i < 3) {
		h_ch_delay_histogram[delay_histogram_i]->Fill(ns_value ); //  
	}
	return  ;
}


void sis_root_channel_delay_histograms::sis3316_increment_delay_draw (void )
{
unsigned int i;

	for (i=0;i<3;i++) {
		this->c1->cd(1+i);
		h_ch_delay_histogram[i]->Draw();
	}
	this->c1->Modified();
	this->c1->Update();

	return  ;
}




/***************************************************************************************************************************************************************************************************************************************/
/***************************************************************************************************************************************************************************************************************************************/
/***************************************************************************************************************************************************************************************************************************************/

#define HISTOGRAM_NG_MAX_NS_LENGTH				40    // 40ns
#define HISTOGRAM_NG_MAX_NS_X_FACTOR			10   // steps in 10ps




sis_root_channel_nG_histogram::sis_root_channel_nG_histogram(char* histogram_text, int histogram_x, int histogram_y, int histogram_x_size, int histogram_y_size)
{


//char *ChannelnGHistogramLabel;


	CanvasBackgroundColor = 10 ; // white

	//c1 = new TCanvas("c1", graph_text, graph_x, graph_y, graph_x_size, graph_y_size);
	this->c1 = new TCanvas(histogram_text, histogram_text, histogram_x, histogram_y, histogram_x_size,histogram_y_size);
	this->c1->SetGrid();
	this->c1->SetFillColor(CanvasBackgroundColor);
	this->c1->Divide(1); 
	//this->c1->cd(1);
		  this->c1->cd(1);
		  gPad->SetGrid();
		  gPad->SetFillColor(CanvasBackgroundColor);

		  this->c1->cd(1);
		this->h_ch_nG_histogram = new TH1F("h_ch_nG_histogram", histogram_text, HISTOGRAM_MAX_NS_X_FACTOR * HISTOGRAM_MAX_NS_LENGTH, 0, HISTOGRAM_MAX_NS_LENGTH-1); // 0 to 40 ns
		gPad->SetGrid();
		gPad->SetFillColor(CanvasBackgroundColor);
  
		h_ch_nG_histogram->Reset(); //  
		h_ch_nG_histogram->BufferEmpty(1); // action =  1 histogram is filled and buffer is deleted
		h_ch_nG_histogram->Draw();
		h_ch_nG_histogram->SetFillColor(kRed);

	this->c1->Modified();
	this->c1->Update();
}


//#ifdef raus
void sis_root_channel_nG_histogram::sis3316_increment_nG_histogram (float ns_value )
{
		h_ch_nG_histogram->Fill(ns_value );  
	return  ;
}


void sis_root_channel_nG_histogram::sis3316_increment_nG_draw (void )
{

		this->c1->cd(1);
		h_ch_nG_histogram->Draw();
 
	this->c1->Modified();
	this->c1->Update();

	return  ;
}
//#endif

//#endif





/***************************************************************************************************************************************************************************************************************************************/
/***************************************************************************************************************************************************************************************************************************************/
/***************************************************************************************************************************************************************************************************************************************/

#define HISTOGRAM_TOF_MAX_NS_LENGTH				40    //  
#define HISTOGRAM_TOF_MAX_NS_X_FACTOR			100   // steps in 10ps




sis_root_channel_tof_histogram::sis_root_channel_tof_histogram(char* histogram_text, int histogram_x, int histogram_y, int histogram_x_size, int histogram_y_size)
{


//char *ChannelnGHistogramLabel;


	CanvasBackgroundColor = 10 ; // white

	//c1 = new TCanvas("c1", graph_text, graph_x, graph_y, graph_x_size, graph_y_size);
	this->c1 = new TCanvas(histogram_text, histogram_text, histogram_x, histogram_y, histogram_x_size,histogram_y_size);
	this->c1->SetGrid();
	this->c1->SetFillColor(CanvasBackgroundColor);
	this->c1->Divide(1); 
	//this->c1->cd(1);
		  this->c1->cd(1);
		  gPad->SetGrid();
		  gPad->SetFillColor(CanvasBackgroundColor);

		  this->c1->cd(1);
		this->h_ch_tof_histogram = new TH1F("h_ch_tof_histogram", histogram_text, HISTOGRAM_TOF_MAX_NS_X_FACTOR * HISTOGRAM_TOF_MAX_NS_LENGTH, -HISTOGRAM_TOF_MAX_NS_LENGTH, HISTOGRAM_TOF_MAX_NS_LENGTH-1); // 0 to 40 ns
		gPad->SetGrid();
		gPad->SetFillColor(CanvasBackgroundColor);
  
		h_ch_tof_histogram->Reset(); //  
		h_ch_tof_histogram->BufferEmpty(1); // action =  1 histogram is filled and buffer is deleted
		h_ch_tof_histogram->Draw();
		h_ch_tof_histogram->SetFillColor(kRed);

	this->c1->Modified();
	this->c1->Update();
}


//#ifdef raus
void sis_root_channel_tof_histogram::sis3316_increment_tof_histogram (float ns_value )
{
		h_ch_tof_histogram->Fill(ns_value );  
	return  ;
}


void sis_root_channel_tof_histogram::sis3316_increment_tof_draw (void )
{

		this->c1->cd(1);
		h_ch_tof_histogram->Draw();
 
	this->c1->Modified();
	this->c1->Update();

	return  ;
}

 


/***************************************************************************************************************************************************************************************************************************************/
/***************************************************************************************************************************************************************************************************************************************/
/***************************************************************************************************************************************************************************************************************************************/

#define HISTOGRAM_ENERGY_MAX_LENGTH				65536    //  
#define HISTOGRAM_ENERGY_MAX_X_FACTOR			1   //  




sis_root_channel_energy_histogram::sis_root_channel_energy_histogram(char* histogram_text, int histogram_x, int histogram_y, int histogram_x_size, int histogram_y_size)
{

	float_gr_x = (float*) malloc(sizeof(float)*HISTOGRAM_ENERGY_MAX_LENGTH);
	float_gr_y = (float*) malloc(sizeof(float)*HISTOGRAM_ENERGY_MAX_LENGTH);

//char *ChannelnGHistogramLabel;


	CanvasBackgroundColor = 10 ; // white

	//c1 = new TCanvas("c1", graph_text, graph_x, graph_y, graph_x_size, graph_y_size);
	this->c1 = new TCanvas(histogram_text, histogram_text, histogram_x, histogram_y, histogram_x_size,histogram_y_size);
	this->c1->SetGrid();
	this->c1->SetFillColor(CanvasBackgroundColor);
	this->c1->Divide(1); 
	//this->c1->cd(1);
		  this->c1->cd(1);
		  gPad->SetGrid();
		  gPad->SetFillColor(CanvasBackgroundColor);

		  this->c1->cd(1);
		this->h_ch_energy_histogram = new TH1F(histogram_text, histogram_text, HISTOGRAM_ENERGY_MAX_X_FACTOR * HISTOGRAM_ENERGY_MAX_LENGTH, 0, HISTOGRAM_ENERGY_MAX_LENGTH-1); // 
		gPad->SetGrid();
		gPad->SetFillColor(CanvasBackgroundColor);
  
		h_ch_energy_histogram->Reset(); //  
		h_ch_energy_histogram->BufferEmpty(1); // action =  1 histogram is filled and buffer is deleted
		h_ch_energy_histogram->Draw();
		h_ch_energy_histogram->SetFillColor(kRed);

	this->c1->Modified();
	this->c1->Update();
}


//#ifdef raus
void sis_root_channel_energy_histogram::sis3316_increment_energy_histogram (float energy_value )
{
		h_ch_energy_histogram->Fill(energy_value );  
	return  ;
}


void sis_root_channel_energy_histogram::sis3316_draw_energy_histogram (void )
{

		this->c1->cd(1);
		h_ch_energy_histogram->Draw();
 
	this->c1->Modified();
	this->c1->Update();

	return  ;
}

sis_root_channel_energy_histogram::~sis_root_channel_energy_histogram(void)
{
	free(float_gr_x);
	free(float_gr_y);
	delete this->c1 ;
} 

/***************************************************************************************************************************************************************************************************************************************/
/***************************************************************************************************************************************************************************************************************************************/
/***************************************************************************************************************************************************************************************************************************************/

#define HISTOGRAM_FPGA_ENERGY_MAX_LENGTH			65536    //  
#define HISTOGRAM_FPGA_ENERGY_MAX_X_FACTOR			1   //  




sis_root_channel_fpga_energy_histogram::sis_root_channel_fpga_energy_histogram(char* histogram_text, int histogram_x, int histogram_y, int histogram_x_size, int histogram_y_size)
{

	float_gr_x = (float*) malloc(sizeof(float)*HISTOGRAM_FPGA_ENERGY_MAX_LENGTH);
	float_gr_y = (float*) malloc(sizeof(float)*HISTOGRAM_FPGA_ENERGY_MAX_LENGTH);

//char *ChannelnGHistogramLabel;


	CanvasBackgroundColor = 10 ; // white

	//c1 = new TCanvas("c1", graph_text, graph_x, graph_y, graph_x_size, graph_y_size);
	this->c1 = new TCanvas(histogram_text, histogram_text, histogram_x, histogram_y, histogram_x_size,histogram_y_size);
	this->c1->SetGrid();
	this->c1->SetFillColor(CanvasBackgroundColor);
	this->c1->Divide(1); 
	//this->c1->cd(1);
		  this->c1->cd(1);
		  gPad->SetGrid();
		  gPad->SetFillColor(CanvasBackgroundColor);

		  this->c1->cd(1);
		//this->h_ch_energy_histogram = new TH1F("fpga_energy_histogram", histogram_text, HISTOGRAM_FPGA_ENERGY_MAX_X_FACTOR * HISTOGRAM_FPGA_ENERGY_MAX_LENGTH, 0, HISTOGRAM_FPGA_ENERGY_MAX_LENGTH-1); // 
		this->h_ch_energy_histogram = new TH1F(histogram_text, histogram_text, HISTOGRAM_FPGA_ENERGY_MAX_X_FACTOR * HISTOGRAM_FPGA_ENERGY_MAX_LENGTH, 0, HISTOGRAM_FPGA_ENERGY_MAX_LENGTH-1); // 
		gPad->SetGrid();
		gPad->SetFillColor(CanvasBackgroundColor);
  
		h_ch_energy_histogram->Reset(); //  
		h_ch_energy_histogram->BufferEmpty(1); // action =  1 histogram is filled and buffer is deleted
		h_ch_energy_histogram->Draw();
		h_ch_energy_histogram->SetFillColor(kRed);

	this->c1->Modified();
	this->c1->Update();
}


void sis_root_channel_fpga_energy_histogram::sis3316_write_histogram (unsigned int length, unsigned int* uint_histogram_array )
{
	int i ;
	int i_length ;
	i_length = length ;
	if (i_length > HISTOGRAM_FPGA_ENERGY_MAX_LENGTH) {
		i_length = HISTOGRAM_FPGA_ENERGY_MAX_LENGTH ;
	}
	h_ch_energy_histogram->Reset(); //  
	h_ch_energy_histogram->BufferEmpty(1); // action =  1 histogram is filled and buffer is deleted
	for (i=0; i<i_length; i++) {
		if (uint_histogram_array[i] != 0) {
			h_ch_energy_histogram->SetBinContent(i, uint_histogram_array[i] );  
		}
	}
	return  ;
}


void sis_root_channel_fpga_energy_histogram::sis3316_draw_energy_histogram (void )
{

		this->c1->cd(1);
		h_ch_energy_histogram->Draw();
 
	this->c1->Modified();
	this->c1->Update();

	return  ;
}

sis_root_channel_fpga_energy_histogram::~sis_root_channel_fpga_energy_histogram(void)
{
	free(float_gr_x);
	free(float_gr_y);
	delete this->c1 ;
} 

/***************************************************************************************************************************************************************************************************************************************/
/***************************************************************************************************************************************************************************************************************************************/



sis_root_canvas_with_TH1F_histogram::sis_root_canvas_with_TH1F_histogram(char* canvas_text, int canvas_x_pos, int canvas_y_pos, int canvas_x_size, int canvas_y_size,
																		int nof_histograms, char* histogram_text, int histogram_bins_x, double histogram_x_low, double histogram_x_up)
{
	int i;
	if (nof_histograms == 4) {
		this->nof_TH1F_histograms = 4 ;
	}
	else {
		this->nof_TH1F_histograms = 1 ;
	}

	CanvasBackgroundColor = 10 ; // white

	//c1 = new TCanvas("c1", graph_text, graph_x, graph_y, graph_x_size, graph_y_size);
	this->c1 = new TCanvas(canvas_text, canvas_text, canvas_x_pos, canvas_y_pos, canvas_x_size, canvas_y_size);
	this->c1->SetGrid();
	this->c1->SetFillColor(CanvasBackgroundColor);

	if (this->nof_TH1F_histograms == 4) {
		this->c1->Divide(2,2); 
	}
	else {
		this->c1->Divide(1); 
	}
	for (i=0;i<nof_TH1F_histograms;i++) {
		this->c1->cd(1+i);
		gPad->SetGrid();
		gPad->SetFillColor(CanvasBackgroundColor);
		//sprintf_s(temp_histogram_text,"%s channel %2d",histogram_text,1+i) ;
		snprintf(temp_histogram_text, 128 , "%s channel %2d",histogram_text,1+i) ;
		this->h_ch_histogram[i] = new TH1F(temp_histogram_text, temp_histogram_text, histogram_bins_x, histogram_x_low, histogram_x_up-1); // 
		h_ch_histogram[i]->Reset(); //  
		h_ch_histogram[i]->BufferEmpty(1); // action =  1 histogram is filled and buffer is deleted
		h_ch_histogram[i]->Draw();
		h_ch_histogram[i]->SetFillColor(kRed);
	}


	//this->c1->cd(1);
	//gPad->SetGrid();
	//gPad->SetFillColor(CanvasBackgroundColor);
  

	this->c1->Modified();
	this->c1->Update();
}


//#ifdef raus
void sis_root_canvas_with_TH1F_histogram::sis3316_increment_histogram (int histogram_index, float increment_index )
{
	if (histogram_index < this->nof_TH1F_histograms) {
		h_ch_histogram[histogram_index]->Fill(increment_index );  
	}
	return  ;
}


void sis_root_canvas_with_TH1F_histogram::sis3316_draw_histogram (void )
{
int i;
	for (i=0;i<nof_TH1F_histograms;i++) {
		this->c1->cd(1+i);
		h_ch_histogram[i]->Draw();
	}
	this->c1->Modified();
	this->c1->Update();

	return  ;
}

sis_root_canvas_with_TH1F_histogram::~sis_root_canvas_with_TH1F_histogram(void)
{
	delete this->c1 ;
} 

/***************************************************************************************************************************************************************************************************************************************/
/***************************************************************************************************************************************************************************************************************************************/
/***************************************************************************************************************************************************************************************************************************************/
/***************************************************************************************************************************************************************************************************************************************/









sis_root_extended_graph::sis_root_extended_graph(char* graph_text, int graph_x, int graph_y, int graph_x_size, int graph_y_size)
{

int i;
	CanvasBackgroundColor = 10 ; // white
	Channel_1_Color  = 2 ; //  
	Channel_2_Color  = 3 ; //  
	Channel_3_Color  = 4 ; //  
	Channel_4_Color  = 6 ; //  
	Channel_5_Color  = 7 ; //  
	Channel_5_Color  = 2 ; //  
	Channel_6_Color  = 8 ; //  
	Channel_7_Color  = 9 ; //  
	Channel_8_Color  = 28 ; //  
	Channel_9_Color  = 41 ; //  
	Channel_10_Color  = 42 ; //  
	Channel_11_Color  = 43 ; //  
	Channel_12_Color  = 44 ; //  
	Channel_13_Color  = 45 ; //  
	Channel_14_Color  = 46 ; //  
	Channel_15_Color  = 47 ; //  
	Channel_16_Color  = 1 ; //  

	max_y_axix_value  = 0xffff;// 16 bit;
	max_y_axix_value  = 0x3fff;// 14 bit;

	gr_x = (int*) malloc(sizeof(int)*GRAPH_MAX_DRAW_EXTENDED_LENGTH); //  
	gr_y = (int*) malloc(sizeof(int)*GRAPH_MAX_DRAW_EXTENDED_LENGTH); //  


	for(i=0;i<GRAPH_MAX_DRAW_EXTENDED_LENGTH;i++) {
		this->gr_x[i] = i ;
		this->gr_y[i] = 0 ;
	}

	//c1 = new TCanvas("c1", graph_text, graph_x, graph_y, graph_x_size, graph_y_size);
	this->c1 = new TCanvas(graph_text, graph_text, graph_x, graph_y, graph_x_size, graph_y_size);
	this->c1->SetGrid();
	this->c1->SetFillColor(CanvasBackgroundColor);
	this->c1->Divide(1); 
	this->c1->cd();

	this->gr1 = new TGraph( GRAPH_MAX_DRAW_EXTENDED_LENGTH, this->gr_x, this->gr_y);  
	//this->gr1->SetTitle(graph_text);
	this->gr1->SetLineColor(Channel_1_Color);
	this->gr1_text = new TLatex(10,10,"Ch x");


	this->c1->Update();
}
/**************************************************************************************/
sis_root_extended_graph::~sis_root_extended_graph(void)
{
	free(gr_x);
	free(gr_y);
	delete this->c1 ;
} 

/**************************************************************************************/
void sis_root_extended_graph::sis3316_set_14bit_Yaxis (void)
{
		this->max_y_axix_value  = 0x3fff ; // 14 bit;
}

void sis_root_extended_graph::sis3316_set_16bit_Yaxis (void)
{
		this->max_y_axix_value  = 0xffff ; // 16 bit
}

/**************************************************************************************/

void sis_root_extended_graph::sis3316_draw_XYaxis (unsigned int plot_length)
{
	unsigned int iBin ;
	unsigned int draw_length;

	this->c1->cd();
	draw_length = plot_length; // 
	if(draw_length > GRAPH_MAX_DRAW_EXTENDED_LENGTH) {
		draw_length = GRAPH_MAX_DRAW_EXTENDED_LENGTH ;
	}

	for(iBin=0; iBin<draw_length;iBin++) {
		this->gr_y[iBin] = max_y_axix_value; 
	}
	this->gr_y[draw_length/2] = 0; 

	this->c1->Clear();
	this->gr1->SetLineColor(CanvasBackgroundColor);
	this->gr1->DrawGraph(draw_length, this->gr_x, this->gr_y, "AL");   
	this->c1->Update();
	return  ;
}



void sis_root_extended_graph::sis3316_draw_chN (unsigned int plot_length, unsigned int* ch_rawdata_buffer, unsigned int ch_no )
{
	unsigned int i, iBin;
	unsigned int module_ch_no;
	unsigned int draw_length;
	int Channel_Color;
	char Channel_Text[80];
	unsigned short* ushort_adc_buffer_ptr;
	ushort_adc_buffer_ptr = (unsigned short*) ch_rawdata_buffer ;

	sprintf(Channel_Text,"Ch %d", ch_no+1);
	module_ch_no = ch_no & 0xf ;
	switch (module_ch_no) {
		case 0: //  
		  Channel_Color = Channel_1_Color;
		  //strcpy(Channel_Text, "Ch 1");
		  break;
		case 1: //  
		  Channel_Color = Channel_2_Color;
		  //strcpy(Channel_Text, "Ch 2");
		  break;
		case 2: //  
		  Channel_Color = Channel_3_Color;
		  //strcpy(Channel_Text, "Ch 3");
		  break;
		 case 3: //  
		  Channel_Color = Channel_4_Color;
		  //strcpy(Channel_Text, "Ch 4");
		  break;
		case 4: //  
		  Channel_Color = Channel_5_Color;
		  //strcpy(Channel_Text, "Ch 5");
		  break;
		case 5: //  
		  Channel_Color = Channel_6_Color;
		  //strcpy(Channel_Text, "Ch 6");
		  break;
		case 6: //  
		  Channel_Color = Channel_7_Color;
		  //strcpy(Channel_Text, "Ch 7");
		  break;
		 case 7: //  
		  Channel_Color = Channel_8_Color;
		  //strcpy(Channel_Text, "Ch 8");
		  break;
		 case 8: //  
		  Channel_Color = Channel_9_Color;
		  //strcpy(Channel_Text, "Ch 9");
		  break;
		 case 9: //  
		  Channel_Color = Channel_10_Color;
		  //strcpy(Channel_Text, "Ch 10");
		  break;
		 case 10: //  
		  Channel_Color = Channel_11_Color;
		  //strcpy(Channel_Text, "Ch 11");
		  break;
		 case 11: //  
		  Channel_Color = Channel_12_Color;
		  //strcpy(Channel_Text, "Ch 12");
		  break;
		 case 12: //  
		  Channel_Color = Channel_13_Color;
		  //strcpy(Channel_Text, "Ch 13");
		  break;
		 case 13: //  
		  Channel_Color = Channel_14_Color;
		  //strcpy(Channel_Text, "Ch 14");
		  break;
		 case 14: //  
		  Channel_Color = Channel_15_Color;
		  //strcpy(Channel_Text, "Ch 15");
		  break;
		 case 15: //  
		  Channel_Color = Channel_16_Color;
		  //strcpy(Channel_Text, "Ch 16");
		  break;
		 default: 
		  Channel_Color = Channel_1_Color;
		  //strcpy(Channel_Text, "Ch x");
		  break;
	}

	this->c1->cd();
	draw_length = plot_length; //                ;
	if(draw_length > GRAPH_MAX_DRAW_EXTENDED_LENGTH) {
		draw_length = GRAPH_MAX_DRAW_EXTENDED_LENGTH ;
	}
	this->gr1->SetLineColor(Channel_Color);
	this->gr1_text->SetTextColor(Channel_Color);
	//this->gr1_text->DrawLatex(draw_length+5,(1000-(ch_no*100)),Channel_Text);
	this->gr1_text->DrawLatex(draw_length+5,(400+(module_ch_no*1000)),Channel_Text);

	iBin= 0;
	for(i=0;i<draw_length;i++) {
		this->gr_y[i] = (int) (ushort_adc_buffer_ptr[i]) ;
	}
	//this->c1->Clear();
	this->gr1->DrawGraph(draw_length, this->gr_x, this->gr_y, "L");   
	this->c1->Update();
	return  ;
}

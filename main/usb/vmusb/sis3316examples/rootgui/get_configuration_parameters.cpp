/***************************************************************************/
/*                                                                         */
/*  Filename: get_configuration_parameters.cpp                             */
/*                                                                         */
/*  Funktion:                                                              */
/*                                                                         */
/*  Autor:                TH                                               */
/*  date:                 27.03.2015                                       */
/*  last modification:    08.11.2021                                       */
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


#include "project_system_define.h"		//define LINUX or WIN
#include "get_configuration_parameters.h"

#ifdef LINUX
	#include <iostream>
	#include <fstream>
	#include <stdio.h>
	#include <string.h>
	#include <stdlib.h>
	#include <unistd.h> 
	using namespace std;
#endif

#ifdef WIN
	#include <iostream>
	#include <fstream>

	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	//#include <getline.h>
	//#include <istream.h>
	//#include <unistd.h>
	//#include <cstdio>
	#include <vector>
	using namespace std;

/**************************************************************************************************************************/

	

void usleep(unsigned int uint_usec) ;

size_t getline(char **lineptr, size_t *n, FILE *stream) {
    char *bufptr = NULL;
    char *p = bufptr;
    size_t size;
    int c;

    if (lineptr == NULL) {
    	return -1;
    }
    if (stream == NULL) {
    	return -1;
    }
    if (n == NULL) {
    	return -1;
    }
    bufptr = *lineptr;
    size = *n;

    c = fgetc(stream);
    if (c == EOF) {
    	return -1;
    }
    if (bufptr == NULL) {
    	//bufptr = (char*)malloc(128);
    	bufptr = (char*)malloc(1024);
    	if (bufptr == NULL) {
    		return -1;
    	}
    	//size = 128;
    	size = 1024;
    }
    p = bufptr;
    while(c != EOF) {

		if ((p - bufptr) > ((signed int)(size - 1))) {
    		//size = size + 128;
    		size = size + 1024;
    		bufptr = (char*)realloc(bufptr, size);  // generates problems : heap error
    		if (bufptr == NULL) {
    			return -1;
    		}
    	}
    	*p++ = c;
    	if (c == '\n') {
    		break;
    	}
    	c = fgetc(stream);
    }

    *p++ = '\0';
    *lineptr = bufptr;
    *n = size;

    return p - bufptr - 1;
}
#endif



/**************************************************************************************************************************/


sis3316_get_configuration_parameters::sis3316_get_configuration_parameters(void)
{
	unsigned int i_ch;
// default values

	this->uint_stop_after_time_seconds_enable = 0;
	this->uint_stop_aftertime_seconds_value   = 60;

	this->uint_stop_after_nof_loops_enable = 0;
	this->uint_stop_after_nof_loops_value = 10;

	this->uint_sample_control_mode              = 0;
	this->uint_sample_control_use_max_enable    = 0;
	this->uint_sample_control_use_max_value     = 0;


	this->uint_pre_trigger_delay          = 20;
	this->uint_raw_sample_start_index     = 0;
	this->uint_raw_sample_length          = 1000;


	//this->uint_maw_test_buffer_enable_flag        = 0;
	this->uint_maw_test_buffer_select_energy_flag = 0;
	this->uint_maw_test_buffer_length             = 1000;
	this->uint_maw_test_buffer_delay              = 100;
	this->uint_maw_test_start_index				  = 0;

	
	this->uint_SuppressEventsIfAddrThres_enable_flag = 0;

	this->uint_format_accum_enable_flag = 0;
	this->uint_format_accum78_enable_flag = 0;
	this->uint_format_maw_enable_flag = 0;
	this->uint_format_energy_enable_flag = 0;

	this->uint_SaveRawDataFirstEventOnly_enable_flag = 0;

	//******

	this->uint_ext_trigger_function_as_extTrigger_enable_flag = 1;
	this->uint_trigger_cond_software_key_flag   = 1;
	this->uint_trigger_cond_vme_lemo_ti_flag    = 0;
	this->uint_extTrig_disable_with_Busy_flag   = 1;
	this->uint_feedback_intTrig_as_extTrig_flag = 0;
	this->uint_Coincidence1FeedbackTrigger_enable = 0;



	for (i_ch = 0; i_ch < 16; i_ch++) {
		this->uint_IntFeedbackTrigger_enable[i_ch] = 0; // 
	}

	this->uint_ext_trigger_function_as_Veto_enable_flag = 0;
	this->uint_local_veto_function_as_Veto_enable_flag = 0;
	this->uint_LemoInUI_as_local_veto_function_enable_flag = 0;


	this->uint_ext_Veto_delay = 0; // 

	for (i_ch = 0; i_ch < 16; i_ch++) {
		this->uint_int_Trigger_delay[i_ch] = 0; // 
	}


	for (i_ch = 0; i_ch < 16; i_ch++) {
		this->uint_ExtTrigger_enable[i_ch] = 1; // 
	}


	for (i_ch = 0; i_ch < 16; i_ch++) {
		this->uint_IntTrigger_enable[i_ch] = 0; // 
	}
	for (i_ch = 0; i_ch < 16; i_ch++) {
		this->uint_SumTrigger_enable[i_ch] = 0; // 
	}
	for (i_ch = 0; i_ch < 16; i_ch++) {
		this->uint_IntPileupTrigger_enable[i_ch] = 0; // 
	}


	for (i_ch = 0; i_ch < 16; i_ch++) {
		this->uint_ExtGate_enable[i_ch] = 0; //  
	}
	for (i_ch = 0; i_ch < 16; i_ch++) {
		this->uint_ExtVeto_enable[i_ch] = 0; //  
	}


	//**********************************************************************************

	for (i_ch = 0; i_ch < 16; i_ch++) {
		this->uint_DisplayChannel_enable[i_ch] = 1; //  
	}

	this->uint_DisplayStatisticCounter_enable = 0;
	this->uint_DisplayMAW_select			  = 1;
	this->uint_DisplayHistogram_select        = 1;
	this->uint_DisplayFFT_select              = 0;




//**********************************************************************************

	for(i_ch=0;i_ch<16;i_ch++) {
		//this->uint_channel_external_trigger_enable[i_ch]    = 1; // all enable
		this->uint_channel_polarity_invert[i_ch]            = 0; // not invert
	}

	for(i_ch=0;i_ch<16;i_ch++) {
		this->uint_channel_range_2V[i_ch]                   = 0; // 5V range
		this->uint_channel_50ohm_termination_disable[i_ch]  = 0; // 50 Ohm termaination
	}

	for(i_ch=0;i_ch<16;i_ch++) {
		this->uint_channel_adc_offset[i_ch]  = 0x8000; //32768 middle 
	}



	for(i_ch=0;i_ch<16;i_ch++) {
		this->uint_channel_trigger_enable[i_ch]        = 0 ; // disabled 
		this->uint_channel_trigger_cfd_enable[i_ch]    = 0 ; // disabled 
		this->uint_channel_trigger_threshold[i_ch]     = 200 ; //  
		this->uint_channel_trigger_peaking[i_ch]       = 10 ; //  
		this->uint_channel_trigger_gap[i_ch]           = 6 ; //  
		this->uint_channel_trigger_pulse_length[i_ch]  = 9; //  
	}

	for(i_ch=0;i_ch<16;i_ch++) {
		this->uint_channel_he_trigger_threshold[i_ch] = 0;
	}
	this->uint_channel_he_trigger_suppress = 0;
	this->uint_channel_he_trigger_generation_cfd_function_idx = 0;

	this->uint_sum_trigger_0_3 = 0;
	this->uint_sum_trigger_4_7 = 0;
	this->uint_sum_trigger_8_11 = 0;
	this->uint_sum_trigger_12_15 = 0;

	this->uint_pileup_window_length       = 1000;
	this->uint_re_pileup_window_length    = 1000;

	this->uint_vme_trigger_idx = 0;
	this->uint_vme_he_trigger_idx = 0;

// -----
	this->uint_energy_peaking_value = 0;
	this->uint_energy_gap_value = 0;
	this->uint_energy_decay_tau_table = 0;
	this->uint_energy_decay_tau_factor = 0;
	this->uint_energy_average_factor = 0;
	this->uint_energy_pickup_idx = 0;

	this->uint_gate1_start_index = 0;
	this->uint_gate1_length = 0;
	this->uint_gate2_start_index = 0;
	this->uint_gate2_length = 0;
	this->uint_gate3_start_index = 0;
	this->uint_gate3_length = 0;
	this->uint_gate4_start_index = 0;
	this->uint_gate4_length = 0;
	this->uint_gate5_start_index = 0;
	this->uint_gate5_length = 0;
	this->uint_gate6_start_index = 0;
	this->uint_gate6_length = 0;
	this->uint_gate7_start_index = 0;
	this->uint_gate7_length = 0;
	this->uint_gate8_start_index = 0;
	this->uint_gate8_length = 0;
	// -----
	this->uint_internal_sample_clock_idx = 0;
	this->uint_internal_sample_clock_fp_en = 0;
	this->uint_fp_clock_idx = 0;
	this->uint_sample_clock_idx = 0;
	this->uint_multiplier_idx = 0;
	this->uint_CoincidenceLookupMode_idx = 0;
	// -----

	this->uint_lemo_out_CO_select          = 0x0;
	this->uint_lemo_out_TO_select          = 0x0;
	this->uint_lemo_out_UO_select          = 0x0;

// -----


}





int sis3316_get_configuration_parameters::read_parameter_file(char *path){
	FILE *fp;

	if(path == NULL){
		return -100;
	}

#ifdef LINUX
	fp = fopen(path, "r");
	if(fp == NULL){
		//printf("open err\n");
		return -101;
	}
	rewind(fp);
#endif
#ifdef WIN
	fp = fopen(path, "r");
	if(fp == NULL){
		//printf("open err\n");
		return -101;
	}
	rewind(fp);
#endif

//printf("open OK\n");

size_t len = 0;      // this will be intialize the buffer
char *line = NULL ; // this will be intialize the buffer


int get_length = 0;
int key_word_length  ;
unsigned int parameter_value  ;
unsigned int channel_parameter_values[16]  ;
int nof_parameters  ;
int i  ;



int comp_err ;

   // fscanf(fp,"%s\n",line);	      // get KEY-STRING 	 
//while(0) {
//while ((get_length = getline(&line, &len, fp)) != EOF) {
while ((get_length = getline( &line, &len, fp)) != EOF) {
    //printf("%s",line);
	//printf("len = %d\n",len);
	//printf("get_length = %d\n",get_length);
    //printf("sizeof(line) = %d\n",sizeof(line));


    key_word_length = this->getNofKewWordLetters (line, get_length);
	//printf("key_word_length = %d\n",key_word_length);
	//printf("get_length = %d\n",get_length);

	if ((key_word_length != 0) && ((get_length - key_word_length) > 0)) {
		nof_parameters = getFollowingChannelParameters(&line[key_word_length], get_length - key_word_length, channel_parameter_values );
		parameter_value = channel_parameter_values[0];

		if (nof_parameters > 0) {

// ********************************************************************************************************


	// TEXT_PARAMETER_STOP_AFTER_TIME_SECONDS_ENABLE
			comp_err = strncmp(line, TEXT_PARAMETER_STOP_AFTER_TIME_SECONDS_ENABLE, key_word_length);
			if (comp_err == 0) {
				uint_stop_after_time_seconds_enable = parameter_value;
			}

		// TEXT_PARAMETER_STOP_AFTER_TIME_SECONDS_VALUE
			comp_err = strncmp(line, TEXT_PARAMETER_STOP_AFTER_TIME_SECONDS_VALUE, key_word_length);
			if (comp_err == 0) {
				uint_stop_aftertime_seconds_value = parameter_value;
			}





	// TEXT_PARAMETER_STOP_AFTER_NOF_LOOPS_ENABLE
			comp_err = strncmp( line, TEXT_PARAMETER_STOP_AFTER_NOF_LOOPS_ENABLE, key_word_length) ;
			if(comp_err == 0)  {
				uint_stop_after_nof_loops_enable = parameter_value ;
			}

	// TEXT_PARAMETER_STOP_AFTER_NOF_LOOPS_VALUE
			comp_err = strncmp( line, TEXT_PARAMETER_STOP_AFTER_NOF_LOOPS_VALUE, key_word_length) ;
			if(comp_err == 0)  {
				uint_stop_after_nof_loops_value = parameter_value ;
			}


	// TEXT_PARAMETER_SAMPLE_CONTROL_MODE
			comp_err = strncmp( line, TEXT_PARAMETER_SAMPLE_CONTROL_MODE, key_word_length) ;
			if(comp_err == 0)  {
				uint_sample_control_mode = parameter_value ;
			}


	// TEXT_PARAMETER_SAMPLE_CONTROL_USE_MAX_ENABLE
			comp_err = strncmp( line, TEXT_PARAMETER_SAMPLE_CONTROL_USE_MAX_ENABLE, key_word_length) ;
			if(comp_err == 0)  {
				uint_sample_control_use_max_enable = parameter_value ;
			}


	// TEXT_PARAMETER_SAMPLE_CONTROL_USE_MAX_VALUE
			comp_err = strncmp( line, TEXT_PARAMETER_SAMPLE_CONTROL_USE_MAX_VALUE, key_word_length) ;
			if(comp_err == 0)  {
				uint_sample_control_use_max_value = parameter_value ;
			}




	// PARAMETER_PRE_TRIGGER_DELAY
			comp_err = strncmp( line, TEXT_PARAMETER_PRE_TRIGGER_DELAY, key_word_length) ;
			if(comp_err == 0)  {
				uint_pre_trigger_delay = parameter_value ;
			}

	// PARAMETER_RAW_SAMPLE_START_INDEX
			comp_err = strncmp( line, TEXT_PARAMETER_RAW_SAMPLE_START_INDEX, key_word_length) ;
			if(comp_err == 0)  {
				uint_raw_sample_start_index = parameter_value ;
			}

	// PARAMETER_RAW_SAMPLE_LENGTH
			comp_err = strncmp( line, TEXT_PARAMETER_RAW_SAMPLE_LENGTH, key_word_length) ;
			if(comp_err == 0)  {
				uint_raw_sample_length = parameter_value ;
				//printf("strncmp OK\n");
			}

	
	// PARAMETER_MAW_TEST_BUFFER_SELECT_ENERGY_FLAG
			comp_err = strncmp( line, TEXT_PARAMETER_MAW_TEST_BUFFER_SELECT_ENERGY_FLAG, key_word_length) ;
			if(comp_err == 0)  {
				uint_maw_test_buffer_select_energy_flag = parameter_value ;
			}


	// PARAMETER_MAW_TEST_BUFFER_LENGTH
			comp_err = strncmp( line, TEXT_PARAMETER_MAW_TEST_BUFFER_LENGTH, key_word_length) ;
			if(comp_err == 0)  {
				uint_maw_test_buffer_length = parameter_value ;
			}

	// PARAMETER_MAW_TEST_BUFFER_DELAY
			comp_err = strncmp( line, TEXT_PARAMETER_MAW_TEST_BUFFER_DELAY, key_word_length) ;
			if(comp_err == 0)  {
				uint_maw_test_buffer_delay = parameter_value ;
			}

	// PARAMETER_MAW_TEST_START_INDEX
			comp_err = strncmp( line, TEXT_PARAMETER_MAW_TEST_START_INDEX, key_word_length) ;
			if(comp_err == 0)  {
				uint_maw_test_start_index = parameter_value ;
			}		




			
	// PARAMETER_SUPPRESS_EVENTS_IF_ADDR_THRES_ENABLE
			comp_err = strncmp(line, TEXT_PARAMETER_SUPPRESS_EVENTS_IF_ADDR_THRES_ENABLE, key_word_length);
			if (comp_err == 0) {
				uint_SuppressEventsIfAddrThres_enable_flag = parameter_value;
			}


	// PARAMETER_FORMAT_ACCUM_ENABLE
			comp_err = strncmp( line, TEXT_PARAMETER_FORMAT_ACCUM_ENABLE, key_word_length) ;
			if(comp_err == 0)  {
				uint_format_accum_enable_flag = parameter_value ;
			}		

	// PARAMETER_FORMAT_ACCUM78_ENABLE
			comp_err = strncmp(line, TEXT_PARAMETER_FORMAT_ACCUM78_ENABLE, key_word_length);
			if (comp_err == 0) {
				uint_format_accum78_enable_flag = parameter_value;
			}


	// PARAMETER_FORMAT_MAW_ENABLE
			comp_err = strncmp( line, TEXT_PARAMETER_FORMAT_MAW_ENABLE, key_word_length) ;
			if(comp_err == 0)  {
				uint_format_maw_enable_flag = parameter_value ;
			}		

	// PARAMETER_FORMAT_ENERGY_ENABLE
			comp_err = strncmp( line, TEXT_PARAMETER_FORMAT_ENERGY_ENABLE, key_word_length) ;
			if(comp_err == 0)  {
				uint_format_energy_enable_flag = parameter_value ;
			}


	// PARAMETER_SAVE_RAW_DATA_FIRST_EVENT_ENABLE
			comp_err = strncmp(line, TEXT_PARAMETER_SAVE_RAW_DATA_FIRST_EVENT_ENABLE, key_word_length);
			if (comp_err == 0) {
				uint_SaveRawDataFirstEventOnly_enable_flag = parameter_value;
			}


	// PARAMETER_EXT_TRIGGER_FUNC_AS_EXT_TRIGGER
			comp_err = strncmp(line, TEXT_PARAMETER_EXT_TRIGGER_FUNC_AS_EXT_TRIGGER, key_word_length);
			if (comp_err == 0) {
				uint_ext_trigger_function_as_extTrigger_enable_flag = parameter_value;
			}


	// PARAMETER_TRIGGER_COND_SOFTWARE_KEY
			comp_err = strncmp( line, TEXT_PARAMETER_TRIGGER_COND_SOFTWARE_KEY, key_word_length) ;
			if(comp_err == 0)  {
				uint_trigger_cond_software_key_flag = parameter_value ;
			}

	// PARAMETER_TRIGGER_COND_VME_LEMO_TI
			comp_err = strncmp( line, TEXT_PARAMETER_TRIGGER_COND_VME_LEMO_TI, key_word_length) ;
			if(comp_err == 0)  {
				uint_trigger_cond_vme_lemo_ti_flag = parameter_value ;
			}


			// PARAMETER_EXT_TRIGGER_DISABLE_WITH_BUSY
			comp_err = strncmp(line, TEXT_PARAMETER_EXT_TRIGGER_DISABLE_WITH_BUSY, key_word_length);
			if (comp_err == 0) {
				uint_extTrig_disable_with_Busy_flag = parameter_value;
			}

			// PARAMETER_FEEDBACK_INT_TRIGGER_AS_EXT_TRIGGER
			comp_err = strncmp(line, TEXT_PARAMETER_FEEDBACK_INT_TRIGGER_AS_EXT_TRIGGER, key_word_length);
			if (comp_err == 0) {
				uint_feedback_intTrig_as_extTrig_flag = parameter_value;
			}

			// PARAMETER_SELECT_FEEDBACK_COINCIDENCE1_TRIGGER
			comp_err = strncmp(line, TEXT_PARAMETER_SELECT_FEEDBACK_COINCIDENCE1_TRIGGER, key_word_length);
			if (comp_err == 0) {
				uint_Coincidence1FeedbackTrigger_enable = parameter_value;
			}


			// PARAMETER_CHANNEL_SELECT_FEEDBACK_INT_TRIGGER
			comp_err = strncmp(line, TEXT_PARAMETER_CHANNEL_SELECT_FEEDBACK_INT_TRIGGER, key_word_length);
			if (comp_err == 0) {
				for (i = 0; i < nof_parameters; i++) {
					uint_IntFeedbackTrigger_enable[i] = channel_parameter_values[i];
				}
			}
			//******

			// PARAMETER_EXT_TRIG_FUNC_AS_EXT_VETO_GATE
			comp_err = strncmp(line, TEXT_PARAMETER_EXT_TRIG_FUNC_AS_EXT_VETO_GATE, key_word_length);
			if (comp_err == 0) {
				uint_ext_trigger_function_as_Veto_enable_flag = parameter_value;
			}

			// PARAMETER_LOCAL_VETO_FUNC_AS_EXT_VETO_GATE
			comp_err = strncmp(line, TEXT_PARAMETER_LOCAL_VETO_FUNC_AS_EXT_VETO_GATE, key_word_length);
			if (comp_err == 0) {
				uint_local_veto_function_as_Veto_enable_flag = parameter_value;
			}


			// PARAMETER_UI_AS_LOCAL_VETO_FUNC
			comp_err = strncmp(line, TEXT_PARAMETER_UI_AS_LOCAL_VETO_FUNC, key_word_length);
			if (comp_err == 0) {
				uint_LemoInUI_as_local_veto_function_enable_flag = parameter_value;
			}


			// PARAMETER_EXT_VETO_DELAY
			comp_err = strncmp(line, TEXT_PARAMETER_EXT_VETO_DELAY, key_word_length);
			if (comp_err == 0) {
				uint_ext_Veto_delay = parameter_value;
			}

			// PARAMETER_CHANNEL_INT_TRIGGER_DELAY
			comp_err = strncmp(line, TEXT_PARAMETER_CHANNEL_INT_TRIGGER_DELAY, key_word_length);
			if (comp_err == 0) {
				if (nof_parameters == 1) {
					for (i = 0; i < 16; i++) {
						uint_int_Trigger_delay[i] = channel_parameter_values[0];
					}
				}
				else {
					for (i = 0; i < nof_parameters; i++) {
						uint_int_Trigger_delay[i] = channel_parameter_values[i];
					}
				}
			}



			//******

			// PARAMETER_CHANNEL_EXT_TRIGGER_ENBALE
			comp_err = strncmp(line, TEXT_PARAMETER_CHANNEL_EXT_TRIGGER_ENBALE, key_word_length);
			if (comp_err == 0) {
				for (i = 0; i < nof_parameters; i++) {
					uint_ExtTrigger_enable[i] = channel_parameter_values[i];
				}
			}

			// PARAMETER_CHANNEL_INT_TRIGGER_ENBALE
			comp_err = strncmp(line, TEXT_PARAMETER_CHANNEL_INT_TRIGGER_ENBALE, key_word_length);
			if (comp_err == 0) {
				for (i = 0; i < nof_parameters; i++) {
					uint_IntTrigger_enable[i] = channel_parameter_values[i];
				}
			}

			// PARAMETER_CHANNEL_INT_SUM_TRIGGER_ENBALE
			comp_err = strncmp(line, TEXT_PARAMETER_CHANNEL_INT_SUM_TRIGGER_ENBALE, key_word_length);
			if (comp_err == 0) {
				for (i = 0; i < nof_parameters; i++) {
					uint_SumTrigger_enable[i] = channel_parameter_values[i];
				}
			}

			// PARAMETER_CHANNEL_INT_PILEUP_TRIGGER_ENBALE
			comp_err = strncmp(line, TEXT_PARAMETER_CHANNEL_INT_PILEUP_TRIGGER_ENBALE, key_word_length);
			if (comp_err == 0) {
				for (i = 0; i < nof_parameters; i++) {
					uint_IntPileupTrigger_enable[i] = channel_parameter_values[i];
				}
			}


			// PARAMETER_CHANNEL_EXT_GATE_ENBALE
			comp_err = strncmp(line, TEXT_PARAMETER_CHANNEL_EXT_GATE_ENBALE, key_word_length);
			if (comp_err == 0) {
				for (i = 0; i < nof_parameters; i++) {
					uint_ExtGate_enable[i] = channel_parameter_values[i];
				}
			}

			// PARAMETER_CHANNEL_EXT_VETO_ENBALE
			comp_err = strncmp(line, TEXT_PARAMETER_CHANNEL_EXT_VETO_ENBALE, key_word_length);
			if (comp_err == 0) {
				for (i = 0; i < nof_parameters; i++) {
					uint_ExtVeto_enable[i] = channel_parameter_values[i];
				}
			}



		// *************

			// -------- Display
			// PARAMETER_DISPLAY_CHANNEL_ENABLE
			comp_err = strncmp(line, TEXT_PARAMETER_DISPLAY_CHANNEL_ENABLE, key_word_length);
			if (comp_err == 0) {
				for (i = 0; i < nof_parameters; i++) {
					uint_DisplayChannel_enable[i] = channel_parameter_values[i];
				}
			}

			// PARAMETER_DISPLAY_STATISTIC_COUNTER_ENABLE
			comp_err = strncmp(line, TEXT_PARAMETER_DISPLAY_STATISTIC_COUNTER_ENABLE, key_word_length);
			if (comp_err == 0) {
				uint_DisplayStatisticCounter_enable = parameter_value;
			}

			// PARAMETER_DISPLAY_MAW_SELECT
			comp_err = strncmp(line, TEXT_PARAMETER_DISPLAY_MAW_SELECT, key_word_length);
			if (comp_err == 0) {
				uint_DisplayMAW_select = parameter_value;
			}

			// PARAMETER_DISPLAY_HISTOGRAM_SELECT
			comp_err = strncmp(line, TEXT_PARAMETER_DISPLAY_HISTOGRAM_SELECT, key_word_length);
			if (comp_err == 0) {
				uint_DisplayHistogram_select = parameter_value;
			}

			// PARAMETER_DISPLAY_FFT_SELECT
			comp_err = strncmp(line, TEXT_PARAMETER_DISPLAY_FFT_SELECT, key_word_length);
			if (comp_err == 0) {
				uint_DisplayFFT_select = parameter_value;
			}


// *************


	// TEXT_PARAMETER_CHANNEL_POLARITY_INVERT
			comp_err = strncmp( line, TEXT_PARAMETER_CHANNEL_POLARITY_INVERT, key_word_length) ;
			if(comp_err == 0)  {
				for (i=0;i<nof_parameters;i++) {
					uint_channel_polarity_invert[i] = channel_parameter_values[i]  ;
				}
			}

	// TEXT_PARAMETER_CHANNEL_RANGE_2V
			comp_err = strncmp( line, TEXT_PARAMETER_CHANNEL_RANGE_2V, key_word_length) ;
			if(comp_err == 0)  {
				for (i=0;i<nof_parameters;i++) {
					uint_channel_range_2V[i] = channel_parameter_values[i]  ;
				}
			}

	// TEXT_PARAMETER_CHANNEL_50OHM_TERMINATION_DISABLE
			comp_err = strncmp( line, TEXT_PARAMETER_CHANNEL_50OHM_TERMINATION_DISABLE, key_word_length) ;
			if(comp_err == 0)  {
				for (i=0;i<nof_parameters;i++) {
					uint_channel_50ohm_termination_disable[i] = channel_parameter_values[i]  ;
				}
			}

	// TEXT_PARAMETER_CHANNEL_ADC_OFFSET
			comp_err = strncmp( line, TEXT_PARAMETER_CHANNEL_ADC_OFFSET, key_word_length) ;
			if(comp_err == 0)  {
				for (i=0;i<nof_parameters;i++) {
					uint_channel_adc_offset[i] = channel_parameter_values[i]  ;
				}
			}

// *******************************

	// TEXT_PARAMETER_CHANNEL_TRIGGER_GENERATION_PULSE_LENGTH
			comp_err = strncmp( line, TEXT_PARAMETER_CHANNEL_TRIGGER_GENERATION_PULSE_LENGTH, key_word_length) ;
			if(comp_err == 0)  {
				if(nof_parameters == 1) {
					for (i=0;i<16;i++) {
						uint_channel_trigger_pulse_length[i] = channel_parameter_values[0]  ;
					}
				}
				else {
					for (i=0;i<nof_parameters;i++) {
						uint_channel_trigger_pulse_length[i] = channel_parameter_values[i]  ;
					}
				}
			}


	// TEXT_PARAMETER_CHANNEL_TRIGGER_GENERATION_GAP
			comp_err = strncmp( line, TEXT_PARAMETER_CHANNEL_TRIGGER_GENERATION_GAP, key_word_length) ;
			if(comp_err == 0)  {
				if(nof_parameters == 1) {
					for (i=0;i<16;i++) {
						uint_channel_trigger_gap[i] = channel_parameter_values[0]  ;
					}
				}
				else {
					for (i=0;i<nof_parameters;i++) {
						uint_channel_trigger_gap[i] = channel_parameter_values[i]  ;
					}
				}
			}


	// TEXT_PARAMETER_CHANNEL_TRIGGER_GENERATION_PEAKING
			comp_err = strncmp( line, TEXT_PARAMETER_CHANNEL_TRIGGER_GENERATION_PEAKING, key_word_length) ;
			if(comp_err == 0)  {
				if(nof_parameters == 1) {
					for (i=0;i<16;i++) {
						uint_channel_trigger_peaking[i] = channel_parameter_values[0]  ;
					}
				}
				else {
					for (i=0;i<nof_parameters;i++) {
						uint_channel_trigger_peaking[i] = channel_parameter_values[i]  ;
					}
				}
			}


	// TEXT_PARAMETER_CHANNEL_TRIGGER_GENERATION_THRESHOLD
			comp_err = strncmp( line, TEXT_PARAMETER_CHANNEL_TRIGGER_GENERATION_THRESHOLD, key_word_length) ;
			if(comp_err == 0)  {
				if(nof_parameters == 1) {
					for (i=0;i<16;i++) {
						uint_channel_trigger_threshold[i] = channel_parameter_values[0]  ;
					}
				}
				else {
					for (i=0;i<nof_parameters;i++) {
						uint_channel_trigger_threshold[i] = channel_parameter_values[i]  ;
					}
				}
			}



	// TEXT_PARAMETER_CHANNEL_HE_TRIGGER_GENERATION_THRESHOLD
			comp_err = strncmp( line, TEXT_PARAMETER_CHANNEL_HE_TRIGGER_GENERATION_THRESHOLD, key_word_length) ;
			if(comp_err == 0)  {
				if(nof_parameters == 1) {
					for (i=0;i<16;i++) {
						uint_channel_he_trigger_threshold[i] = channel_parameter_values[0]  ;
					}
				}
				else {
					for (i=0;i<nof_parameters;i++) {
						uint_channel_he_trigger_threshold[i] = channel_parameter_values[i]  ;
					}
				}
			}


			
	// TEXT_PARAMETER_CHANNEL_HE_TRIGGER_GENERATION_SUPPRESS
			comp_err = strncmp( line, TEXT_PARAMETER_CHANNEL_HE_TRIGGER_GENERATION_SUPPRESS, key_word_length) ;
			if(comp_err == 0)  {
				uint_channel_he_trigger_suppress = parameter_value ;
			}

	// TEXT_PARAMETER_CHANNEL_HE_TRIGGER_GENERATION_CFD_FUNCTION_IDX
			comp_err = strncmp( line, TEXT_PARAMETER_CHANNEL_HE_TRIGGER_GENERATION_CFD_FUNCTION_IDX, key_word_length) ;
			if(comp_err == 0)  {
				uint_channel_he_trigger_generation_cfd_function_idx = parameter_value ;
			}
			


// ****** 



	// TEXT_PARAMETER_CHANNEL_TRIGGER_GENERATION_ENABLE
			comp_err = strncmp( line, TEXT_PARAMETER_CHANNEL_TRIGGER_GENERATION_ENABLE, key_word_length) ;
			if(comp_err == 0)  {
				for (i=0;i<nof_parameters;i++) {
					uint_channel_trigger_enable[i] = channel_parameter_values[i]  ;
				}
			}



	// TEXT_PARAMETER_SUM_TRIGGER_0_4
			comp_err = strncmp( line, TEXT_PARAMETER_SUM_TRIGGER_0_3, key_word_length) ;
			if(comp_err == 0)  {
				uint_sum_trigger_0_3 = parameter_value ;
			}

	// TEXT_PARAMETER_SUM_TRIGGER_5_7
			comp_err = strncmp( line, TEXT_PARAMETER_SUM_TRIGGER_4_7, key_word_length) ;
			if(comp_err == 0)  {
				uint_sum_trigger_4_7 = parameter_value ;
			}

	// TEXT_PARAMETER_SUM_TRIGGER_8_11
			comp_err = strncmp( line, TEXT_PARAMETER_SUM_TRIGGER_8_11, key_word_length) ;
			if(comp_err == 0)  {
				uint_sum_trigger_8_11 = parameter_value ;
			}

	// TEXT_PARAMETER_SUM_TRIGGER_12_15
			comp_err = strncmp( line, TEXT_PARAMETER_SUM_TRIGGER_12_15, key_word_length) ;
			if(comp_err == 0)  {
				uint_sum_trigger_12_15 = parameter_value ;
			}



// ****** 
	// PARAMETER_PILEUP_WINDOW_LENGTH
			comp_err = strncmp( line, TEXT_PARAMETER_PILEUP_WINDOW_LENGTH, key_word_length) ;
			if(comp_err == 0)  {
				uint_pileup_window_length = parameter_value ;
			}

	// PARAMETER_REPILEUP_WINDOW_LENGTH
			comp_err = strncmp( line, TEXT_PARAMETER_REPILEUP_WINDOW_LENGTH, key_word_length) ;
			if(comp_err == 0)  {
				uint_re_pileup_window_length = parameter_value ;
			}

			
			
	// TEXT_PARAMETER_VME_TRIGGER_IDX
			comp_err = strncmp( line, TEXT_PARAMETER_VME_TRIGGER_IDX, key_word_length) ;
			if(comp_err == 0)  {
				uint_vme_trigger_idx = parameter_value ;
			}

	// TEXT_PARAMETER_VME_HE_TRIGGER_IDX
			comp_err = strncmp( line, TEXT_PARAMETER_VME_HE_TRIGGER_IDX, key_word_length) ;
			if(comp_err == 0)  {
				uint_vme_he_trigger_idx = parameter_value ;
			}




// *************************



	// TEXT_PARAMETER_ENERGY_PEAKING
			comp_err = strncmp( line, TEXT_PARAMETER_ENERGY_PEAKING, key_word_length) ;
			if(comp_err == 0)  {
				uint_energy_peaking_value = parameter_value ;
			}

	// TEXT_PARAMETER_ENERGY_GAP
			comp_err = strncmp( line, TEXT_PARAMETER_ENERGY_GAP, key_word_length) ;
			if(comp_err == 0)  {
				uint_energy_gap_value = parameter_value ;
			}

	// TEXT_PARAMETER_ENERGY_DECAY_TAU_TABLE
			comp_err = strncmp( line, TEXT_PARAMETER_ENERGY_DECAY_TAU_TABLE, key_word_length) ;
			if(comp_err == 0)  {
				uint_energy_decay_tau_table = parameter_value ;
			}

	// TEXT_PARAMETER_ENERGY_DECAY_TAU_FACTOR
			comp_err = strncmp( line, TEXT_PARAMETER_ENERGY_DECAY_TAU_FACTOR, key_word_length) ;
			if(comp_err == 0)  {
				uint_energy_decay_tau_factor = parameter_value ;
			}

	// TEXT_PARAMETER_ENERGY_AVG_FACTOR
			comp_err = strncmp( line, TEXT_PARAMETER_ENERGY_AVG_FACTOR, key_word_length) ;
			if(comp_err == 0)  {
				uint_energy_average_factor = parameter_value ;
			}

	// TEXT_PARAMETER_ENERGY_PICKUP_IDX
			comp_err = strncmp( line, TEXT_PARAMETER_ENERGY_PICKUP_IDX, key_word_length) ;
			if(comp_err == 0)  {
				uint_energy_pickup_idx = parameter_value ;
			}
	// ------

	// TEXT_PARAMETER_GATE1_START_INDEX
			comp_err = strncmp( line, TEXT_PARAMETER_GATE1_START_INDEX, key_word_length) ;
			if(comp_err == 0)  {
				uint_gate1_start_index = parameter_value ;
			}

	// TEXT_PARAMETER_GATE1_LENGTH
			comp_err = strncmp( line, TEXT_PARAMETER_GATE1_LENGTH, key_word_length) ;
			if(comp_err == 0)  {
				uint_gate1_length = parameter_value ;
			}

	// TEXT_PARAMETER_GATE2_START_INDEX
			comp_err = strncmp( line, TEXT_PARAMETER_GATE2_START_INDEX, key_word_length) ;
			if(comp_err == 0)  {
				uint_gate2_start_index = parameter_value ;
			}

	// TEXT_PARAMETER_GATE2_LENGTH
			comp_err = strncmp( line, TEXT_PARAMETER_GATE2_LENGTH, key_word_length) ;
			if(comp_err == 0)  {
				uint_gate2_length = parameter_value ;
			}

	// TEXT_PARAMETER_GATE3_START_INDEX
			comp_err = strncmp( line, TEXT_PARAMETER_GATE3_START_INDEX, key_word_length) ;
			if(comp_err == 0)  {
				uint_gate3_start_index = parameter_value ;
			}

	// TEXT_PARAMETER_GATE3_LENGTH
			comp_err = strncmp( line, TEXT_PARAMETER_GATE3_LENGTH, key_word_length) ;
			if(comp_err == 0)  {
				uint_gate3_length = parameter_value ;
			}

	// TEXT_PARAMETER_GATE4_START_INDEX
			comp_err = strncmp( line, TEXT_PARAMETER_GATE4_START_INDEX, key_word_length) ;
			if(comp_err == 0)  {
				uint_gate4_start_index = parameter_value ;
			}

	// TEXT_PARAMETER_GATE4_LENGTH
			comp_err = strncmp( line, TEXT_PARAMETER_GATE4_LENGTH, key_word_length) ;
			if(comp_err == 0)  {
				uint_gate4_length = parameter_value ;
			}

	// TEXT_PARAMETER_GATE5_START_INDEX
			comp_err = strncmp( line, TEXT_PARAMETER_GATE5_START_INDEX, key_word_length) ;
			if(comp_err == 0)  {
				uint_gate5_start_index = parameter_value ;
			}

	// TEXT_PARAMETER_GATE5_LENGTH
			comp_err = strncmp( line, TEXT_PARAMETER_GATE5_LENGTH, key_word_length) ;
			if(comp_err == 0)  {
				uint_gate5_length = parameter_value ;
			}

	// TEXT_PARAMETER_GATE6_START_INDEX
			comp_err = strncmp( line, TEXT_PARAMETER_GATE6_START_INDEX, key_word_length) ;
			if(comp_err == 0)  {
				uint_gate6_start_index = parameter_value ;
			}

	// TEXT_PARAMETER_GATE6_LENGTH
			comp_err = strncmp( line, TEXT_PARAMETER_GATE6_LENGTH, key_word_length) ;
			if(comp_err == 0)  {
				uint_gate6_length = parameter_value ;
			}


	// TEXT_PARAMETER_GATE7_START_INDEX
			comp_err = strncmp(line, TEXT_PARAMETER_GATE7_START_INDEX, key_word_length);
			if (comp_err == 0) {
				uint_gate7_start_index = parameter_value;
			}

	// TEXT_PARAMETER_GATE7_LENGTH
			comp_err = strncmp(line, TEXT_PARAMETER_GATE7_LENGTH, key_word_length);
			if (comp_err == 0) {
				uint_gate7_length = parameter_value;
			}

	// TEXT_PARAMETER_GATE8_START_INDEX
			comp_err = strncmp(line, TEXT_PARAMETER_GATE8_START_INDEX, key_word_length);
			if (comp_err == 0) {
				uint_gate8_start_index = parameter_value;
			}

	// TEXT_PARAMETER_GATE8_LENGTH
			comp_err = strncmp(line, TEXT_PARAMETER_GATE8_LENGTH, key_word_length);
			if (comp_err == 0) {
				uint_gate8_length = parameter_value;
			}


	// ************************************

	// TEXT_PARAMETER_INTERNAL_SAMPLE_CLOCK_IDX
			comp_err = strncmp( line, TEXT_PARAMETER_INTERNAL_SAMPLE_CLOCK_IDX, key_word_length) ;
			if(comp_err == 0)  {
				uint_internal_sample_clock_idx = parameter_value ;
			}

	// TEXT_PARAMETER_INTERNAL_SAMPLE_CLOCK_FP_EN
			comp_err = strncmp( line, TEXT_PARAMETER_INTERNAL_SAMPLE_CLOCK_FP_EN, key_word_length) ;
			if(comp_err == 0)  {
				uint_internal_sample_clock_fp_en = parameter_value ;
			}

	// TEXT_PARAMETER_FP_CLOCK_IDX
			comp_err = strncmp( line, TEXT_PARAMETER_FP_CLOCK_IDX, key_word_length) ;
			if(comp_err == 0)  {
				uint_fp_clock_idx = parameter_value ;
			}

	// TEXT_PARAMETER_SAMPLE_CLOCK_IDX
			comp_err = strncmp( line, TEXT_PARAMETER_SAMPLE_CLOCK_IDX, key_word_length) ;
			if(comp_err == 0)  {
				uint_sample_clock_idx = parameter_value ;
			}

	// TEXT_PARAMETER_MULTIPLIER_IDX
			comp_err = strncmp( line, TEXT_PARAMETER_MULTIPLIER_IDX, key_word_length) ;
			if(comp_err == 0)  {
				uint_multiplier_idx = parameter_value ;
			}


	// TEXT_PARAMETER_COINCIDENCE_LOOKUP_TABLE_IDX
			comp_err = strncmp(line, TEXT_PARAMETER_COINCIDENCE_LOOKUP_TABLE_IDX, key_word_length);
			if (comp_err == 0) {
				uint_CoincidenceLookupMode_idx = parameter_value;
			}




// -------------------------------------------------------------
			 		

 


	// TEXT_PARAMETER_LEMO_OUT_CO_SELECT
			comp_err = strncmp( line, TEXT_PARAMETER_LEMO_OUT_CO_SELECT, key_word_length) ;
			if(comp_err == 0)  {
				uint_lemo_out_CO_select = parameter_value ;
				//printf("strncmp OK\n");
			}

	// TEXT_PARAMETER_LEMO_OUT_TO_SELECT
			comp_err = strncmp( line, TEXT_PARAMETER_LEMO_OUT_TO_SELECT, key_word_length) ;
			if(comp_err == 0)  {
				uint_lemo_out_TO_select = parameter_value ;
				//printf("strncmp OK\n");
			}

	// TEXT_PARAMETER_LEMO_OUT_UO_SELECT
			comp_err = strncmp( line, TEXT_PARAMETER_LEMO_OUT_UO_SELECT, key_word_length) ;
			if(comp_err == 0)  {
				uint_lemo_out_UO_select = parameter_value ;
				//printf("strncmp OK\n");
			}





		}
	}



}
fclose(fp);

	return 0;
}

/*******************************************************************************/

int sis3316_get_configuration_parameters::write_parameter_file(char *path) {

	int i_ch;
	std::ofstream file;
	//FILE *fp;
	file.open(path);

	file << TEXT_PARAMETER_STOP_AFTER_TIME_SECONDS_ENABLE << " " << uint_stop_after_time_seconds_enable << endl;
	file << TEXT_PARAMETER_STOP_AFTER_TIME_SECONDS_VALUE << " " << uint_stop_aftertime_seconds_value << endl;
	file << endl;

	file << TEXT_PARAMETER_STOP_AFTER_NOF_LOOPS_ENABLE << " " << uint_stop_after_nof_loops_enable << endl;
	file << TEXT_PARAMETER_STOP_AFTER_NOF_LOOPS_VALUE << " " << uint_stop_after_nof_loops_value << endl;
	file << endl;

	file << TEXT_PARAMETER_SAMPLE_CONTROL_MODE << " " << uint_sample_control_mode << endl;
	file << TEXT_PARAMETER_SAMPLE_CONTROL_USE_MAX_ENABLE << " " << uint_sample_control_use_max_enable << endl;
	file << TEXT_PARAMETER_SAMPLE_CONTROL_USE_MAX_VALUE << " " << uint_sample_control_use_max_value << endl;
	file << endl;





	file << TEXT_PARAMETER_PRE_TRIGGER_DELAY << " " << uint_pre_trigger_delay << endl;
	file << TEXT_PARAMETER_RAW_SAMPLE_START_INDEX << " " << uint_raw_sample_start_index << endl;
	file << TEXT_PARAMETER_RAW_SAMPLE_LENGTH << " " << uint_raw_sample_length << endl;

	file << endl;
	file << TEXT_PARAMETER_MAW_TEST_BUFFER_SELECT_ENERGY_FLAG << " " << uint_maw_test_buffer_select_energy_flag << endl;
	file << TEXT_PARAMETER_MAW_TEST_BUFFER_LENGTH << " " << uint_maw_test_buffer_length << endl;
	file << TEXT_PARAMETER_MAW_TEST_BUFFER_DELAY << " " << uint_maw_test_buffer_delay << endl;
	file << TEXT_PARAMETER_MAW_TEST_START_INDEX << " " << uint_maw_test_start_index << endl;

	
	file << endl;
	file << TEXT_PARAMETER_SUPPRESS_EVENTS_IF_ADDR_THRES_ENABLE << " " << uint_SuppressEventsIfAddrThres_enable_flag << endl;
	file << TEXT_PARAMETER_FORMAT_ACCUM_ENABLE << " " << uint_format_accum_enable_flag << endl;
	file << TEXT_PARAMETER_FORMAT_ACCUM78_ENABLE << " " << uint_format_accum78_enable_flag << endl;
	file << TEXT_PARAMETER_FORMAT_MAW_ENABLE << " " << uint_format_maw_enable_flag << endl;
	file << TEXT_PARAMETER_FORMAT_ENERGY_ENABLE << " " << uint_format_energy_enable_flag << endl;
	file << TEXT_PARAMETER_SAVE_RAW_DATA_FIRST_EVENT_ENABLE << " " << uint_SaveRawDataFirstEventOnly_enable_flag << endl;

	file << endl;
	file << endl;

	file << TEXT_PARAMETER_EXT_TRIGGER_FUNC_AS_EXT_TRIGGER << " " << uint_ext_trigger_function_as_extTrigger_enable_flag << endl;
	file << TEXT_PARAMETER_TRIGGER_COND_SOFTWARE_KEY << " " << uint_trigger_cond_software_key_flag << endl;
	file << TEXT_PARAMETER_TRIGGER_COND_VME_LEMO_TI << " " << uint_trigger_cond_vme_lemo_ti_flag << endl;

	file << TEXT_PARAMETER_EXT_TRIGGER_DISABLE_WITH_BUSY << " " << uint_extTrig_disable_with_Busy_flag << endl;
	file << TEXT_PARAMETER_FEEDBACK_INT_TRIGGER_AS_EXT_TRIGGER << " " << uint_feedback_intTrig_as_extTrig_flag << endl;

	file << TEXT_PARAMETER_SELECT_FEEDBACK_COINCIDENCE1_TRIGGER << " " << uint_Coincidence1FeedbackTrigger_enable << endl;



	file << TEXT_PARAMETER_CHANNEL_SELECT_FEEDBACK_INT_TRIGGER;
	for (i_ch = 0; i_ch < 16; i_ch++) {
		file << " " << uint_IntFeedbackTrigger_enable[i_ch];
	}
	file << endl;

	file << TEXT_PARAMETER_EXT_TRIG_FUNC_AS_EXT_VETO_GATE << " " << uint_ext_trigger_function_as_Veto_enable_flag << endl;
	file << TEXT_PARAMETER_LOCAL_VETO_FUNC_AS_EXT_VETO_GATE << " " << uint_local_veto_function_as_Veto_enable_flag << endl;
	file << TEXT_PARAMETER_UI_AS_LOCAL_VETO_FUNC << " " << uint_LemoInUI_as_local_veto_function_enable_flag << endl;
	file << TEXT_PARAMETER_EXT_VETO_DELAY << " " << uint_ext_Veto_delay << endl;

	file << TEXT_PARAMETER_CHANNEL_INT_TRIGGER_DELAY;
	i_ch = 0;
	file << " " << uint_int_Trigger_delay[i_ch] << endl;




	file << TEXT_PARAMETER_CHANNEL_EXT_TRIGGER_ENBALE;
	for (i_ch = 0; i_ch < 16; i_ch++) {
		file << " " << uint_ExtTrigger_enable[i_ch];
	}
	file << endl;

	file << TEXT_PARAMETER_CHANNEL_INT_TRIGGER_ENBALE;
	for (i_ch = 0; i_ch < 16; i_ch++) {
		file << " " << uint_IntTrigger_enable[i_ch];
	}
	file << endl;

	file << TEXT_PARAMETER_CHANNEL_INT_SUM_TRIGGER_ENBALE;
	for (i_ch = 0; i_ch < 16; i_ch++) {
		file << " " << uint_SumTrigger_enable[i_ch];
}
	file << endl;

	file << TEXT_PARAMETER_CHANNEL_INT_PILEUP_TRIGGER_ENBALE;
	for (i_ch = 0; i_ch < 16; i_ch++) {
		file << " " << uint_IntPileupTrigger_enable[i_ch];
	}
	file << endl;

	file << TEXT_PARAMETER_CHANNEL_EXT_GATE_ENBALE;
	for (i_ch = 0; i_ch < 16; i_ch++) {
		file << " " << uint_ExtGate_enable[i_ch];
	}
	file << endl;

	file << TEXT_PARAMETER_CHANNEL_EXT_VETO_ENBALE;
	for (i_ch = 0; i_ch < 16; i_ch++) {
		file << " " << uint_ExtVeto_enable[i_ch];
	}
	file << endl;
	file << endl;



	// -------- Display

	file << TEXT_PARAMETER_DISPLAY_CHANNEL_ENABLE;
	for (i_ch = 0; i_ch < 16; i_ch++) {
		file << " " << uint_DisplayChannel_enable[i_ch];
	}
	file << endl;


	file << TEXT_PARAMETER_DISPLAY_STATISTIC_COUNTER_ENABLE << " " << uint_DisplayStatisticCounter_enable << endl;
	file << TEXT_PARAMETER_DISPLAY_MAW_SELECT << " " << uint_DisplayMAW_select << endl;
	file << TEXT_PARAMETER_DISPLAY_HISTOGRAM_SELECT << " " << uint_DisplayHistogram_select << endl;
	file << TEXT_PARAMETER_DISPLAY_FFT_SELECT << " " << uint_DisplayFFT_select << endl;


 	file << endl;
	file << endl;
	



	file << TEXT_PARAMETER_CHANNEL_POLARITY_INVERT;
	for(i_ch=0;i_ch<16;i_ch++) {
		file << " " << uint_channel_polarity_invert[i_ch];
	}
	file << endl;

	file << TEXT_PARAMETER_CHANNEL_RANGE_2V;
	for(i_ch=0;i_ch<16;i_ch++) {
		file << " " << uint_channel_range_2V[i_ch];
	}
	file << endl;

	file << TEXT_PARAMETER_CHANNEL_50OHM_TERMINATION_DISABLE;
	for(i_ch=0;i_ch<16;i_ch++) {
		file << " " << uint_channel_50ohm_termination_disable[i_ch];
	}
	file << endl;

	file << TEXT_PARAMETER_CHANNEL_ADC_OFFSET;
	for(i_ch=0;i_ch<16;i_ch++) {
		file << " " << uint_channel_adc_offset[i_ch];
	}
	file << endl;
	file << endl;






	file << TEXT_PARAMETER_CHANNEL_TRIGGER_GENERATION_PULSE_LENGTH;
	i_ch = 0 ;
	file << " " << uint_channel_trigger_pulse_length[i_ch] << endl;
//
	file << TEXT_PARAMETER_CHANNEL_TRIGGER_GENERATION_GAP;
	i_ch = 0 ;
	file << " " << uint_channel_trigger_gap[i_ch] << endl;
//
	file << TEXT_PARAMETER_CHANNEL_TRIGGER_GENERATION_PEAKING;
	i_ch = 0 ;
	file << " " << uint_channel_trigger_peaking[i_ch] << endl;
//
	file << TEXT_PARAMETER_CHANNEL_TRIGGER_GENERATION_THRESHOLD;
	i_ch = 0 ;
	file << " " << uint_channel_trigger_threshold[i_ch] << endl;
//
	file << TEXT_PARAMETER_CHANNEL_HE_TRIGGER_GENERATION_THRESHOLD;
	i_ch = 0 ;
	file << " " << uint_channel_he_trigger_threshold[i_ch] << endl;
//
	file << TEXT_PARAMETER_CHANNEL_HE_TRIGGER_GENERATION_SUPPRESS << " " << uint_channel_he_trigger_suppress << endl;
	file << TEXT_PARAMETER_CHANNEL_HE_TRIGGER_GENERATION_CFD_FUNCTION_IDX << " " << uint_channel_he_trigger_generation_cfd_function_idx << endl;
//
	file << endl;


	file << TEXT_PARAMETER_CHANNEL_TRIGGER_GENERATION_ENABLE;
	for(i_ch=0;i_ch<16;i_ch++) {
		file << " " << uint_channel_trigger_enable[i_ch];
	}
	file << endl;
	file << TEXT_PARAMETER_SUM_TRIGGER_0_3 << " " << uint_sum_trigger_0_3 << endl;
	file << TEXT_PARAMETER_SUM_TRIGGER_4_7 << " " << uint_sum_trigger_4_7 << endl;
	file << TEXT_PARAMETER_SUM_TRIGGER_8_11 << " " << uint_sum_trigger_8_11 << endl;
	file << TEXT_PARAMETER_SUM_TRIGGER_12_15 << " " << uint_sum_trigger_12_15 << endl;
	file << endl;

	file << TEXT_PARAMETER_PILEUP_WINDOW_LENGTH << " " << uint_pileup_window_length << endl;
	file << TEXT_PARAMETER_REPILEUP_WINDOW_LENGTH << " " << uint_re_pileup_window_length << endl;
	file << endl;

	file << TEXT_PARAMETER_VME_TRIGGER_IDX << " " << uint_vme_trigger_idx << endl;
	file << TEXT_PARAMETER_VME_HE_TRIGGER_IDX << " " << uint_vme_he_trigger_idx << endl;
	file << endl;
	file << endl;


//---------------

	
	file << TEXT_PARAMETER_ENERGY_PEAKING << " " << uint_energy_peaking_value << endl;
	file << TEXT_PARAMETER_ENERGY_GAP << " " << uint_energy_gap_value << endl;
	file << TEXT_PARAMETER_ENERGY_DECAY_TAU_TABLE << " " << uint_energy_decay_tau_table << endl;
	file << TEXT_PARAMETER_ENERGY_DECAY_TAU_FACTOR << " " << uint_energy_decay_tau_factor << endl;
	file << TEXT_PARAMETER_ENERGY_AVG_FACTOR << " " << uint_energy_average_factor << endl;
	file << TEXT_PARAMETER_ENERGY_PICKUP_IDX << " " << uint_energy_pickup_idx << endl;
	file << endl;
	

	file << TEXT_PARAMETER_GATE1_START_INDEX << " " << uint_gate1_start_index << endl;
	file << TEXT_PARAMETER_GATE1_LENGTH << " " << uint_gate1_length << endl;
	file << TEXT_PARAMETER_GATE2_START_INDEX << " " << uint_gate2_start_index << endl;
	file << TEXT_PARAMETER_GATE2_LENGTH << " " << uint_gate2_length << endl;
	file << TEXT_PARAMETER_GATE3_START_INDEX << " " << uint_gate3_start_index << endl;
	file << TEXT_PARAMETER_GATE3_LENGTH << " " << uint_gate3_length << endl;
	file << TEXT_PARAMETER_GATE4_START_INDEX << " " << uint_gate4_start_index << endl;
	file << TEXT_PARAMETER_GATE4_LENGTH << " " << uint_gate4_length << endl;
	file << TEXT_PARAMETER_GATE5_START_INDEX << " " << uint_gate5_start_index << endl;
	file << TEXT_PARAMETER_GATE5_LENGTH << " " << uint_gate5_length << endl;
	file << TEXT_PARAMETER_GATE6_START_INDEX << " " << uint_gate6_start_index << endl;
	file << TEXT_PARAMETER_GATE6_LENGTH << " " << uint_gate6_length << endl;
	file << TEXT_PARAMETER_GATE7_START_INDEX << " " << uint_gate7_start_index << endl;
	file << TEXT_PARAMETER_GATE7_LENGTH << " " << uint_gate7_length << endl;
	file << TEXT_PARAMETER_GATE8_START_INDEX << " " << uint_gate8_start_index << endl;
	file << TEXT_PARAMETER_GATE8_LENGTH << " " << uint_gate8_length << endl;
	file << endl;
	file << endl;

//---------------
	file << TEXT_PARAMETER_INTERNAL_SAMPLE_CLOCK_IDX << " " << uint_internal_sample_clock_idx << endl;
	file << TEXT_PARAMETER_INTERNAL_SAMPLE_CLOCK_FP_EN << " " << uint_internal_sample_clock_fp_en << endl;
	file << TEXT_PARAMETER_FP_CLOCK_IDX << " " << uint_fp_clock_idx << endl;
	file << TEXT_PARAMETER_SAMPLE_CLOCK_IDX << " " << uint_sample_clock_idx << endl;
	file << TEXT_PARAMETER_MULTIPLIER_IDX << " " << uint_multiplier_idx << endl;
	file << TEXT_PARAMETER_COINCIDENCE_LOOKUP_TABLE_IDX << " " << uint_CoincidenceLookupMode_idx << endl;
	file << endl;
	file << endl;
//---------------



	file << TEXT_PARAMETER_LEMO_OUT_CO_SELECT << " " << uint_lemo_out_CO_select << endl;
	file << TEXT_PARAMETER_LEMO_OUT_TO_SELECT << " " << uint_lemo_out_TO_select << endl;
	file << TEXT_PARAMETER_LEMO_OUT_UO_SELECT << " " << uint_lemo_out_UO_select << endl;
	file << endl;
	file << endl;


	file << endl;



	file.close();
	return 0;
}



int sis3316_get_configuration_parameters::getNofKewWordLetters (char* line_buf, int max_length)
{
	int i;
	for (i=0;i<max_length;i++) {
		if (this->isKeyWord(line_buf[i])) {
			//printf("%c\n",line_buf[i]);
		}
		else {
			//printf("end i = %d\n",i);
			return i ;
		}
	}
	return 0 ;
}


/*******************************************************************************/

int sis3316_get_configuration_parameters::getFollowingParameter(char* line_buf, int max_length)
{
	int i;
	int temp_data=0;
	//printf("max_length = %d\n",max_length);
	for (i=0;i<max_length;i++) {
		if (this->isDigit(line_buf[i])) {
			//printf("%c\n",line_buf[i]);
			sscanf(&line_buf[i],"%d\t", &temp_data) ;
			//printf("return temp_data = %d\n",temp_data);
			return temp_data ;
		}
	}
	return -1 ;
}

int sis3316_get_configuration_parameters::getFollowingChannelParameters(char* line_buf, int max_length, unsigned int* channel_parameters )
{
	int i;
	int rc;
	int temp_data=0;
	int nof_parameters=0;
	for (i=0;i<max_length;i++) {
		if (this->isDigit(line_buf[i])) {
			//printf("%c\n",line_buf[i]);
			if (this->isHexDigit(line_buf[i+1])) {
				rc=sscanf(&line_buf[i],"%x\t", &temp_data) ;
				//printf("hex  temp_data = %x\n", temp_data);
			} 
			else {
				rc=sscanf(&line_buf[i],"%d\t", &temp_data) ;
				//printf("dec  temp_data = %x\n", temp_data);
			} 
			//printf("rc = %d  temp_data = %d\n",rc, temp_data);
			channel_parameters[nof_parameters] = temp_data ;
			nof_parameters++;
			if(nof_parameters >= 16) {return nof_parameters;}
			
			do {
				i++;
				if(i >= max_length) {
					return nof_parameters ;
				}
			} while (!this->isControl(line_buf[i])) ;

		}
	}
	//printf("maxnof_parameters_length = %d\n\n",nof_parameters);
	return nof_parameters ;
}

/*******************************************************************************/

bool sis3316_get_configuration_parameters::isKeyWord (char c )
{
	return (('A' <= c && c <= 'Z') || (c == '_') || ('0' <= c && c <= '9') ) ;
}

bool sis3316_get_configuration_parameters::isUpperLetter (char c )
{
	return (('A' <= c && c <= 'Z') || (c == '_') ) ;
}

bool sis3316_get_configuration_parameters::isDigit (char c )
{
	return ('0' <= c && c <= '9') ;
}

bool sis3316_get_configuration_parameters::isHexDigit (char c )
{
	return (('X' == c ) || (c == 'x') ) ;
}

bool sis3316_get_configuration_parameters::isControl (char c )
{
	return (c <= 0x20) ;
}


/*******************************************************************************/

  


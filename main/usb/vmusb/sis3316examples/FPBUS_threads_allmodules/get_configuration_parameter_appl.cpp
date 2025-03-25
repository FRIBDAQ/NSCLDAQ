/***************************************************************************/
/*                                                                         */
/*  Filename: get_configuration_parameter_appl.cpp                         */
/*                                                                         */
/*  Funktion:                                                              */
/*                                                                         */
/*  Autor:                TH                                               */
/*  date:                 17.09.2019                                       */
/*  last modification:    04.01.2022                                       */
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
/*  � 2022                                                                 */
/*                                                                         */
/***************************************************************************/
#include "get_configuration_parameter_appl.h"
#include "project_system_define.h"		//define LINUX or WIN



#ifdef LINUX
	#include <stdio.h>
	#include <string.h>
	#include <stdlib.h>
	#include <unistd.h>

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

		if ((p - bufptr) > (size - 1)) {
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
	unsigned int temp;
// default values


	this->uint_udp_jumbo_mode            = 0;
	this->uint_udp_nofPacketsPerRequest  = 1 ;

	this->uint_channel_polarity_invert             = 0 ;
	this->uint_channel_range_2V                    = 0 ;
	this->uint_channel_50ohm_termination_disable   = 0 ;
	this->uint_channel_adc_offset                  = 0x8000 ;

	this->uint_nof_events                         = 1 ;
	this->uint_pre_trigger_delay                  = 100 ;
	this->uint_raw_sample_start_index             = 0 ;
	this->uint_raw_sample_length                  = 600 ;
	this->uint_raw_sample_first_event_only_mode   = 0 ;

	this->uint_trigger_gate_window_length         = 620 ;
	this->uint_pileup_window_length               = 620 ;
	this->uint_re_pileup_window_length            = 620 ;


	this->uint_trigger_function_deadtime_block_with_addrThres_enable   = 0 ;
	this->uint_trigger_function_deadtime_logic_enable  = 1 ;
	this->uint_trigger_function_deadtime_length   = 800 ;
	this->uint_veto_gate_delay                    = 0 ;

	this->uint_channel_trigger_threshold          = 100 ;
	this->uint_channel_trigger_peaking            = 10 ;
	this->uint_channel_trigger_gap                = 8 ;
	this->uint_channel_trigger_pulse_length       = 10 ;



	this->uint_gate1_start_index       = 0 ;
	this->uint_gate1_length            = 10 ;

	this->uint_gate2_start_index       = 10 ;
	this->uint_gate2_length            = 10 ;

	this->uint_gate3_start_index       = 20 ;
	this->uint_gate3_length            = 10 ;

	this->uint_gate4_start_index       = 30 ;
	this->uint_gate4_length            = 10 ;

	this->uint_gate5_start_index       = 40 ;
	this->uint_gate5_length            = 10 ;

	this->uint_gate6_start_index       = 50 ;
	this->uint_gate6_length            = 10 ;

	this->uint_lemo_out_CO_select            = 0 ;
	this->uint_lemo_out_TO_select            = 0 ;
	this->uint_lemo_out_UO_select            = 0 ;

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
		//parameter_value = getFollowingParameter(&line[key_word_length], get_length - key_word_length);
		//printf("parameter_value = %d\n",parameter_value);
		nof_parameters = getFollowingChannelParameters(&line[key_word_length], get_length - key_word_length, channel_parameter_values );
		parameter_value = channel_parameter_values[0];

		if (nof_parameters > 0) {
			//printf("nof_parameters = %d\n",nof_parameters);
			//for (i=0;i<nof_parameters;i++) {
			//	printf("%d  \n",channel_parameter_values[i]);
			//}
			//printf("\n");

// ********************************************************************************************************


			// TEXT_PARAMETER_UDP_JUMBO_MODE
				comp_err = strncmp( line, TEXT_PARAMETER_UDP_JUMBO_MODE, key_word_length) ;
				if(comp_err == 0)  {
					uint_udp_jumbo_mode = parameter_value ;
				}

			// TEXT_PARAMETER_UDP_NOF_PACKET_PER_REQUEST
				comp_err = strncmp( line, TEXT_PARAMETER_UDP_NOF_PACKET_PER_REQUEST, key_word_length) ;
				if(comp_err == 0)  {
					uint_udp_nofPacketsPerRequest = parameter_value ;
				}


			// TEXT_PARAMETER_ALL_CHANNEL_POLARITY_INVERT
				comp_err = strncmp( line, TEXT_PARAMETER_ALL_CHANNEL_POLARITY_INVERT, key_word_length) ;
				if(comp_err == 0)  {
					uint_channel_polarity_invert = parameter_value ;
				}

			// TEXT_PARAMETER_ALL_CHANNEL_RANGE_2V
				comp_err = strncmp( line, TEXT_PARAMETER_ALL_CHANNEL_RANGE_2V, key_word_length) ;
				if(comp_err == 0)  {
					uint_channel_range_2V = parameter_value ;
				}

			// TEXT_PARAMETER_ALL_CHANNEL_50OHM_TERMINATION_DISABLE
				comp_err = strncmp( line, TEXT_PARAMETER_ALL_CHANNEL_50OHM_TERMINATION_DISABLE, key_word_length) ;
				if(comp_err == 0)  {
					uint_channel_50ohm_termination_disable = parameter_value ;
				}


			// TEXT_PARAMETER_ALL_CHANNEL_ADC_OFFSET
				comp_err = strncmp( line, TEXT_PARAMETER_ALL_CHANNEL_ADC_OFFSET, key_word_length) ;
				if(comp_err == 0)  {
					uint_channel_adc_offset = parameter_value ;
				}


			// TEXT_PARAMETER_NOF_EVENTS_PER_BANK
				comp_err = strncmp( line, TEXT_PARAMETER_NOF_EVENTS_PER_BANK, key_word_length) ;
				if(comp_err == 0)  {
					uint_nof_events = parameter_value ;
				}

			// TEXT_PARAMETER_PRE_TRIGGER_DELAY
				comp_err = strncmp( line, TEXT_PARAMETER_PRE_TRIGGER_DELAY, key_word_length) ;
				if(comp_err == 0)  {
					uint_pre_trigger_delay = parameter_value ;
				}


			// TEXT_PARAMETER_RAW_SAMPLE_START_INDEX
				comp_err = strncmp( line, TEXT_PARAMETER_RAW_SAMPLE_START_INDEX, key_word_length) ;
				if(comp_err == 0)  {
					uint_raw_sample_start_index = parameter_value ;
				}

			// TEXT_PARAMETER_RAW_SAMPLE_LENGTH
				comp_err = strncmp( line, TEXT_PARAMETER_RAW_SAMPLE_LENGTH, key_word_length) ;
				if(comp_err == 0)  {
					uint_raw_sample_length = parameter_value ;
				}

			// TEXT_PARAMETER_SAVE_RAW_SAMPLE_FIRST_EVENT_ONLY_MODE
				comp_err = strncmp( line, TEXT_PARAMETER_SAVE_RAW_SAMPLE_FIRST_EVENT_ONLY_MODE, key_word_length) ;
				if(comp_err == 0)  {
					uint_raw_sample_first_event_only_mode = parameter_value ;
				}


			// TEXT_PARAMETER_TRIGGER_GATE_WINDOW_LENGTH
				comp_err = strncmp( line, TEXT_PARAMETER_TRIGGER_GATE_WINDOW_LENGTH, key_word_length) ;
				if(comp_err == 0)  {
					uint_trigger_gate_window_length = parameter_value ;
				}

			// TEXT_PARAMETER_PILEUP_WINDOW_LENGTH
				comp_err = strncmp( line, TEXT_PARAMETER_PILEUP_WINDOW_LENGTH, key_word_length) ;
				if(comp_err == 0)  {
					uint_pileup_window_length = parameter_value ;
				}

			// TEXT_PARAMETER_REPILEUP_WINDOW_LENGTH
				comp_err = strncmp( line, TEXT_PARAMETER_REPILEUP_WINDOW_LENGTH, key_word_length) ;
				if(comp_err == 0)  {
					uint_re_pileup_window_length = parameter_value ;
				}


			// TEXT_PARAMETER_ACQUISITIN_TRIGGER_MODE
				comp_err = strncmp( line, TEXT_PARAMETER_ACQUISITIN_TRIGGER_MODE, key_word_length) ;
				if(comp_err == 0)  {
					uint_acquisition_trigger_mode = parameter_value ;
				}


			// TEXT_PARAMETER_TRIGGER_FUNCTION_BLOCK_WITH_ADDR_THRES_ENABLE
				comp_err = strncmp( line, TEXT_PARAMETER_TRIGGER_FUNCTION_BLOCK_WITH_ADDR_THRES_ENABLE, key_word_length) ;
				if(comp_err == 0)  {
					uint_trigger_function_deadtime_block_with_addrThres_enable = parameter_value ;
					//printf("strncmp OK\n");
				}

			// TEXT_PARAMETER_TRIGGER_FUNCTION_DEADTIME_ENABLE
				comp_err = strncmp( line, TEXT_PARAMETER_TRIGGER_FUNCTION_DEADTIME_ENABLE, key_word_length) ;
				if(comp_err == 0)  {
					uint_trigger_function_deadtime_logic_enable = parameter_value ;
					//printf("strncmp OK\n");
				}

			// TEXT_PARAMETER_TRIGGER_FUNCTION_DEADTIME_LENGTH
				comp_err = strncmp( line, TEXT_PARAMETER_TRIGGER_FUNCTION_DEADTIME_LENGTH, key_word_length) ;
				if(comp_err == 0)  {
					uint_trigger_function_deadtime_length = parameter_value ;
					//printf("strncmp OK\n");
				}



			// TEXT_PARAMETER_VETO_GATE_DELAY
				comp_err = strncmp( line, TEXT_PARAMETER_VETO_GATE_DELAY, key_word_length) ;
				if(comp_err == 0)  {
					uint_veto_gate_delay = parameter_value ;
					//printf("strncmp OK\n");
				}



			// TEXT_PARAMETER_ALL_CHANNEL_TRIGGER_GENERATION_THRESHOLD
				comp_err = strncmp( line, TEXT_PARAMETER_ALL_CHANNEL_TRIGGER_GENERATION_THRESHOLD, key_word_length) ;
				if(comp_err == 0)  {
					uint_channel_trigger_threshold = parameter_value ;
				}


			// TEXT_PARAMETER_ALL_CHANNEL_TRIGGER_GENERATION_PEAKING
				comp_err = strncmp( line, TEXT_PARAMETER_ALL_CHANNEL_TRIGGER_GENERATION_PEAKING, key_word_length) ;
				if(comp_err == 0)  {
					uint_channel_trigger_peaking = parameter_value ;
				}

			// TEXT_PARAMETER_ALL_CHANNEL_TRIGGER_GENERATION_GAP
				comp_err = strncmp( line, TEXT_PARAMETER_ALL_CHANNEL_TRIGGER_GENERATION_GAP, key_word_length) ;
				if(comp_err == 0)  {
					uint_channel_trigger_gap = parameter_value ;
				}

			// TEXT_PARAMETER_ALL_CHANNEL_TRIGGER_GENERATIONR_PULSE_LENGTH
				comp_err = strncmp( line, TEXT_PARAMETER_ALL_CHANNEL_TRIGGER_GENERATIONR_PULSE_LENGTH, key_word_length) ;
				if(comp_err == 0)  {
					uint_channel_trigger_pulse_length = parameter_value ;
				}




			// TEXT_PARAMETER_ALL_CHANNEL_GATE1_START_INDEX
				comp_err = strncmp( line, TEXT_PARAMETER_ALL_CHANNEL_GATE1_START_INDEX, key_word_length) ;
				if(comp_err == 0)  {
					uint_gate1_start_index = parameter_value ;
					//printf("strncmp OK\n");
				}

			// TEXT_PARAMETER_ALL_CHANNEL_GATE1_LENGTH
				comp_err = strncmp( line, TEXT_PARAMETER_ALL_CHANNEL_GATE1_LENGTH, key_word_length) ;
				if(comp_err == 0)  {
					uint_gate1_length = parameter_value ;
				}


			// TEXT_PARAMETER_ALL_CHANNEL_GATE2_START_INDEX
				comp_err = strncmp( line, TEXT_PARAMETER_ALL_CHANNEL_GATE2_START_INDEX, key_word_length) ;
				if(comp_err == 0)  {
					uint_gate2_start_index = parameter_value ;
					//printf("strncmp OK\n");
				}

			// TEXT_PARAMETER_ALL_CHANNEL_GATE2_LENGTH
				comp_err = strncmp( line, TEXT_PARAMETER_ALL_CHANNEL_GATE2_LENGTH, key_word_length) ;
				if(comp_err == 0)  {
					uint_gate2_length = parameter_value ;
				}

			// TEXT_PARAMETER_ALL_CHANNEL_GATE3_START_INDEX
				comp_err = strncmp( line, TEXT_PARAMETER_ALL_CHANNEL_GATE3_START_INDEX, key_word_length) ;
				if(comp_err == 0)  {
					uint_gate3_start_index = parameter_value ;
					//printf("strncmp OK\n");
				}

			// TEXT_PARAMETER_GATE3_LENGTH
				comp_err = strncmp( line, TEXT_PARAMETER_ALL_CHANNEL_GATE3_LENGTH, key_word_length) ;
				if(comp_err == 0)  {
					uint_gate3_length = parameter_value ;
				}


			// TEXT_PARAMETER_ALL_CHANNEL_GATE4_START_INDEX
				comp_err = strncmp( line, TEXT_PARAMETER_ALL_CHANNEL_GATE4_START_INDEX, key_word_length) ;
				if(comp_err == 0)  {
					uint_gate4_start_index = parameter_value ;
					//printf("strncmp OK\n");
				}

			// TEXT_PARAMETER_GATE4_LENGTH
				comp_err = strncmp( line, TEXT_PARAMETER_ALL_CHANNEL_GATE4_LENGTH, key_word_length) ;
				if(comp_err == 0)  {
					uint_gate4_length = parameter_value ;
				}

			// TEXT_PARAMETER_ALL_CHANNEL_GATE5_START_INDEX
				comp_err = strncmp( line, TEXT_PARAMETER_ALL_CHANNEL_GATE5_START_INDEX, key_word_length) ;
				if(comp_err == 0)  {
					uint_gate5_start_index = parameter_value ;
					//printf("strncmp OK\n");
				}

			// TEXT_PARAMETER_GATE5_LENGTH
				comp_err = strncmp( line, TEXT_PARAMETER_ALL_CHANNEL_GATE5_LENGTH, key_word_length) ;
				if(comp_err == 0)  {
					uint_gate5_length = parameter_value ;
				}


			// TEXT_PARAMETER_ALL_CHANNEL_GATE6_START_INDEX
				comp_err = strncmp( line, TEXT_PARAMETER_ALL_CHANNEL_GATE6_START_INDEX, key_word_length) ;
				if(comp_err == 0)  {
					uint_gate6_start_index = parameter_value ;
					//printf("strncmp OK\n");
				}

			// TEXT_PARAMETER_GATE6_LENGTH
				comp_err = strncmp( line, TEXT_PARAMETER_ALL_CHANNEL_GATE6_LENGTH, key_word_length) ;
				if(comp_err == 0)  {
					uint_gate6_length = parameter_value ;
				}





			// TEXT_PARAMETER_ALL_MODULES_LEMO_OUT_CO_SELECT
				comp_err = strncmp( line, TEXT_PARAMETER_ALL_MODULES_LEMO_OUT_CO_SELECT, key_word_length) ;
				if(comp_err == 0)  {
					uint_lemo_out_CO_select = parameter_value ;
				}

			// TEXT_PARAMETER_ALL_MODULES_LEMO_OUT_TO_SELECT
				comp_err = strncmp( line, TEXT_PARAMETER_ALL_MODULES_LEMO_OUT_TO_SELECT, key_word_length) ;
				if(comp_err == 0)  {
					uint_lemo_out_TO_select = parameter_value ;
				}


			// TEXT_PARAMETER_ALL_MODULES_LEMO_OUT_UO_SELECT
				comp_err = strncmp( line, TEXT_PARAMETER_ALL_MODULES_LEMO_OUT_UO_SELECT, key_word_length) ;
				if(comp_err == 0)  {
					uint_lemo_out_UO_select = parameter_value ;
				}



		}

	}



} // while

	return 0;
}

/*******************************************************************************/

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
void sis3316_get_configuration_parameters::calculateTriggerGateWindowLength (void)
{
unsigned int temp ;
	// calculation of the Trigger_Gate_Window Length
		this->uint_trigger_gate_window_length      = 8 ;

		if (this->uint_gate1_length  != 0) {
			temp = this->uint_gate1_start_index + this->uint_gate1_length ;
			if (temp > this->uint_trigger_gate_window_length) {
				this->uint_trigger_gate_window_length = temp ;
			}
		}

		if (this->uint_gate2_length  != 0) {
			temp = this->uint_gate2_start_index + this->uint_gate2_length ;
			if (temp > this->uint_trigger_gate_window_length) {
				this->uint_trigger_gate_window_length = temp ;
			}
		}

		if (this->uint_gate3_length  != 0) {
			temp = this->uint_gate3_start_index + this->uint_gate3_length ;
			if (temp > this->uint_trigger_gate_window_length) {
				this->uint_trigger_gate_window_length = temp ;
			}
		}

		this->uint_trigger_gate_window_length      = this->uint_trigger_gate_window_length + 3 ; // must be 3 clocks longer than the accumulation
	return   ;
}


/*******************************************************************************/

#ifdef not_used
void sis3316_get_configuration_parameters::calculatePileupWindowLength (void)
{
	// calculation of the Pileup_Window Length
		this->uint_pileup_window_length  = this->uint_trigger_gate_window_length    ;


		this->uint_re_pileup_window_length  = 8    ;
		  switch (uint_baseline_average_mode) {

		    case 0:
				this->uint_re_pileup_window_length  = this->uint_re_pileup_window_length + 8    ;
				break;

		    case 1:
				this->uint_re_pileup_window_length  = this->uint_re_pileup_window_length + 16    ;
				break;

		    case 2:
				this->uint_re_pileup_window_length  = this->uint_re_pileup_window_length + 32    ;
				break;

		    case 3:
				this->uint_re_pileup_window_length  = this->uint_re_pileup_window_length + 64    ;
				break;

		    case 4:
				this->uint_re_pileup_window_length  = this->uint_re_pileup_window_length + 128    ;
				break;

		    case 5:
				this->uint_re_pileup_window_length  = this->uint_re_pileup_window_length + 256    ;
				break;

		    case 6:
				this->uint_re_pileup_window_length  = this->uint_re_pileup_window_length + 512    ;
				break;

		    default:
				this->uint_re_pileup_window_length  = this->uint_re_pileup_window_length + 512    ;
				break;
		    }


	return   ;
}
#endif


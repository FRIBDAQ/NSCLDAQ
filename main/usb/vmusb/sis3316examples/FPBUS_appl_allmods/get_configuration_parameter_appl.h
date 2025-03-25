/***************************************************************************/
/*                                                                         */
/*  Filename: get_configuration_parameter_appl.h                           */
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
/*  Fax  +49 (0)40 60 87 305 20                                            */
/*                                                                         */
/*  http://www.struck.de                                                   */
/*                                                                         */
/*  � 2021                                                                 */
/*                                                                         */
/***************************************************************************/

#ifndef GET_CONFIGURATION_PARAMETERS_H_
#define GET_CONFIGURATION_PARAMETERS_H_

#define TEXT_PARAMETER_UDP_JUMBO_MODE 									"PARAMETER_UDP_JUMBO_MODE"
#define TEXT_PARAMETER_UDP_NOF_PACKET_PER_REQUEST 						"PARAMETER_UDP_NOF_PACKET_PER_REQUEST"


#define TEXT_PARAMETER_ALL_CHANNEL_POLARITY_INVERT 						"PARAMETER_ALL_CHANNEL_POLARITY_INVERT"
#define TEXT_PARAMETER_ALL_CHANNEL_RANGE_2V   							"PARAMETER_ALL_CHANNEL_RANGE_2V"
#define TEXT_PARAMETER_ALL_CHANNEL_50OHM_TERMINATION_DISABLE   			"PARAMETER_ALL_CHANNEL_50OHM_TERMINATION_DISABLE"
#define TEXT_PARAMETER_ALL_CHANNEL_ADC_OFFSET  							"PARAMETER_ALL_CHANNEL_ADC_OFFSET"


#define TEXT_PARAMETER_NOF_EVENTS_PER_BANK 								"PARAMETER_NOF_EVENTS_PER_BANK"

#define TEXT_PARAMETER_PRE_TRIGGER_DELAY   								"PARAMETER_PRE_TRIGGER_DELAY"
#define TEXT_PARAMETER_RAW_SAMPLE_START_INDEX							"PARAMETER_RAW_SAMPLE_START_INDEX"
#define TEXT_PARAMETER_RAW_SAMPLE_LENGTH   								"PARAMETER_RAW_SAMPLE_LENGTH"
#define TEXT_PARAMETER_SAVE_RAW_SAMPLE_FIRST_EVENT_ONLY_MODE   			"PARAMETER_SAVE_RAW_SAMPLE_FIRST_EVENT_ONLY_MODE"

#define TEXT_PARAMETER_TRIGGER_GATE_WINDOW_LENGTH  						"PARAMETER_TRIGGER_GATE_WINDOW_LENGTH"
#define TEXT_PARAMETER_PILEUP_WINDOW_LENGTH   							"PARAMETER_PILEUP_WINDOW_LENGTH"
#define TEXT_PARAMETER_REPILEUP_WINDOW_LENGTH   						"PARAMETER_REPILEUP_WINDOW_LENGTH"

#define TEXT_PARAMETER_ACQUISITIN_TRIGGER_MODE   						"PARAMETER_ACQUISITIN_TRIGGER_MODE"



#define TEXT_PARAMETER_TRIGGER_FUNCTION_BLOCK_WITH_ADDR_THRES_ENABLE	"PARAMETER_TRIGGER_FUNCTION_BLOCK_WITH_ADDR_THRES_ENABLE"
#define TEXT_PARAMETER_TRIGGER_FUNCTION_DEADTIME_ENABLE					"PARAMETER_TRIGGER_FUNCTION_DEADTIME_ENABLE"
#define TEXT_PARAMETER_TRIGGER_FUNCTION_DEADTIME_LENGTH					"PARAMETER_TRIGGER_FUNCTION_DEADTIME_LENGTH"
#define TEXT_PARAMETER_VETO_GATE_DELAY   								"PARAMETER_VETO_GATE_DELAY"


#define TEXT_PARAMETER_ALL_CHANNEL_TRIGGER_GENERATION_THRESHOLD    		"PARAMETER_ALL_CHANNEL_TRIGGER_GENERATION_THRESHOLD "
#define TEXT_PARAMETER_ALL_CHANNEL_TRIGGER_GENERATION_PEAKING   		"PARAMETER_ALL_CHANNEL_TRIGGER_GENERATION_PEAKING"
#define TEXT_PARAMETER_ALL_CHANNEL_TRIGGER_GENERATION_GAP				"PARAMETER_ALL_CHANNEL_TRIGGER_GENERATION_GAP"
#define TEXT_PARAMETER_ALL_CHANNEL_TRIGGER_GENERATIONR_PULSE_LENGTH		"PARAMETER_ALL_CHANNEL_TRIGGER_GENERATIONR_PULSE_LENGTH"



#define TEXT_PARAMETER_ALL_CHANNEL_GATE1_START_INDEX   					"PARAMETER_ALL_CHANNEL_GATE1_START_INDEX"
#define TEXT_PARAMETER_ALL_CHANNEL_GATE1_LENGTH   						"PARAMETER_ALL_CHANNEL_GATE1_LENGTH"

#define TEXT_PARAMETER_ALL_CHANNEL_GATE2_START_INDEX   					"PARAMETER_ALL_CHANNEL_GATE2_START_INDEX"
#define TEXT_PARAMETER_ALL_CHANNEL_GATE2_LENGTH   						"PARAMETER_ALL_CHANNEL_GATE2_LENGTH"

#define TEXT_PARAMETER_ALL_CHANNEL_GATE3_START_INDEX   					"PARAMETER_ALL_CHANNEL_GATE3_START_INDEX"
#define TEXT_PARAMETER_ALL_CHANNEL_GATE3_LENGTH   						"PARAMETER_ALL_CHANNEL_GATE3_LENGTH"

#define TEXT_PARAMETER_ALL_CHANNEL_GATE4_START_INDEX   					"PARAMETER_ALL_CHANNEL_GATE4_START_INDEX"
#define TEXT_PARAMETER_ALL_CHANNEL_GATE4_LENGTH   						"PARAMETER_ALL_CHANNEL_GATE4_LENGTH"

#define TEXT_PARAMETER_ALL_CHANNEL_GATE5_START_INDEX   					"PARAMETER_ALL_CHANNEL_GATE5_START_INDEX"
#define TEXT_PARAMETER_ALL_CHANNEL_GATE5_LENGTH   						"PARAMETER_ALL_CHANNEL_GATE5_LENGTH"

#define TEXT_PARAMETER_ALL_CHANNEL_GATE6_START_INDEX   					"PARAMETER_ALL_CHANNEL_GATE6_START_INDEX"
#define TEXT_PARAMETER_ALL_CHANNEL_GATE6_LENGTH   						"PARAMETER_ALL_CHANNEL_GATE6_LENGTH"




#define TEXT_PARAMETER_ALL_MODULES_LEMO_OUT_CO_SELECT  					"PARAMETER_ALL_MODULES_LEMO_OUT_CO_SELECT"
#define TEXT_PARAMETER_ALL_MODULES_LEMO_OUT_TO_SELECT  					"PARAMETER_ALL_MODULES_LEMO_OUT_TO_SELECT"
#define TEXT_PARAMETER_ALL_MODULES_LEMO_OUT_UO_SELECT  					"PARAMETER_ALL_MODULES_LEMO_OUT_UO_SELECT"






class sis3316_get_configuration_parameters
{

private:
	int getNofKewWordLetters (char* line_buf, int max_length);
	int getFollowingParameter(char* line_buf, int max_length);
	int getFollowingChannelParameters(char* line_buf, int max_length, unsigned int* channel_parameters );


	bool isKeyWord (char c ) ;

	bool isUpperLetter (char c );
	bool isDigit (char c );
	bool isControl (char c );
	bool isHexDigit (char c );

	void calculateTriggerGateWindowLength (void);
	//void calculatePileupWindowLength (void);


public:

	unsigned int uint_udp_jumbo_mode ;
	unsigned int uint_udp_nofPacketsPerRequest ;

	unsigned int uint_channel_polarity_invert ;
	unsigned int uint_channel_range_2V ;
	unsigned int uint_channel_50ohm_termination_disable ;
	unsigned int uint_channel_adc_offset ;

	unsigned int uint_nof_events ;

	unsigned int uint_pre_trigger_delay ;
	unsigned int uint_raw_sample_start_index ;
	unsigned int uint_raw_sample_length ;
	unsigned int uint_raw_sample_first_event_only_mode ;


	unsigned int uint_trigger_gate_window_length;
	unsigned int uint_pileup_window_length;
	unsigned int uint_re_pileup_window_length;

	unsigned int uint_acquisition_trigger_mode ;

	unsigned int uint_trigger_function_deadtime_block_with_addrThres_enable;
	unsigned int uint_trigger_function_deadtime_logic_enable;
	unsigned int uint_trigger_function_deadtime_length;
	unsigned int uint_veto_gate_delay;

	unsigned int uint_channel_trigger_threshold ;
	unsigned int uint_channel_trigger_peaking ;
	unsigned int uint_channel_trigger_gap ;
	unsigned int uint_channel_trigger_pulse_length ;


	unsigned int uint_gate1_start_index ;
	unsigned int uint_gate1_length ;
	unsigned int uint_gate2_start_index ;
	unsigned int uint_gate2_length ;
	unsigned int uint_gate3_start_index ;
	unsigned int uint_gate3_length ;
	unsigned int uint_gate4_start_index ;
	unsigned int uint_gate4_length ;
	unsigned int uint_gate5_start_index ;
	unsigned int uint_gate5_length ;
	unsigned int uint_gate6_start_index ;
	unsigned int uint_gate6_length ;

	unsigned int uint_lemo_out_CO_select ;
	unsigned int uint_lemo_out_TO_select ;
	unsigned int uint_lemo_out_UO_select ;





public:
	sis3316_get_configuration_parameters(void);

	int read_parameter_file(char *path);

	~sis3316_get_configuration_parameters(void);
};






#endif /* GET_CONFIGURATION_PARAMETERS_H_ */

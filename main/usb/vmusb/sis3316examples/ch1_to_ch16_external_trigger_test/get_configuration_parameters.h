/*
 * get_configuration_parameters.h
 *
 *  Created on: 10.02.2015
 *      Author: th
 */

#ifndef GET_CONFIGURATION_PARAMETERS_H_
#define GET_CONFIGURATION_PARAMETERS_H_


#define TEXT_PARAMETER_NOF_EVENTS_EACH_BANK 				"PARAMETER_NOF_EVENTS_EACH_BANK"


#define TEXT_PARAMETER_CHANNEL_EXTERNAL_TRIGGER_ENABLE   	"PARAMETER_CHANNEL_EXTERNAL_TRIGGER_ENABLE"
#define TEXT_PARAMETER_CHANNEL_POLARITY_INVERT   			"PARAMETER_CHANNEL_POLARITY_INVERT"

#define TEXT_PARAMETER_CHANNEL_RANGE_2V   					"PARAMETER_CHANNEL_RANGE_2V"
#define TEXT_PARAMETER_CHANNEL_50OHM_TERMINATION_DISABLE   	"PARAMETER_CHANNEL_50OHM_TERMINATION_DISABLE"
#define TEXT_PARAMETER_CHANNEL_ADC_OFFSET  					"PARAMETER_CHANNEL_ADC_OFFSET"

#define TEXT_PARAMETER_LEMO_OUT_CO_SELECT  					"PARAMETER_LEMO_OUT_CO_SELECT"
#define TEXT_PARAMETER_LEMO_OUT_TO_SELECT  					"PARAMETER_LEMO_OUT_TO_SELECT"
#define TEXT_PARAMETER_LEMO_OUT_UO_SELECT  					"PARAMETER_LEMO_OUT_UO_SELECT"


#define TEXT_PARAMETER_PRE_TRIGGER_DELAY   					"PARAMETER_PRE_TRIGGER_DELAY"
#define TEXT_PARAMETER_RAW_SAMPLE_LENGTH   					"PARAMETER_RAW_SAMPLE_LENGTH"

#define TEXT_PARAMETER_TRIGGER_GATE_WINDOW_LENGTH  			"PARAMETER_TRIGGER_GATE_WINDOW_LENGTH"
#define TEXT_PARAMETER_PILEUP_WINDOW_LENGTH   				"PARAMETER_PILEUP_WINDOW_LENGTH"
#define TEXT_PARAMETER_REPILEUP_WINDOW_LENGTH   			"PARAMETER_REPILEUP_WINDOW_LENGTH"


#define TEXT_PARAMETER_CHANNEL_TRIGGER_GENERATION_ENABLE   		"PARAMETER_CHANNEL_TRIGGER_GENERATION_ENABLE"
#define TEXT_PARAMETER_CHANNEL_TRIGGER_GENERATION_CFD_ENABLE    "PARAMETER_CHANNEL_TRIGGER_GENERATION_CFD_ENABLE "
#define TEXT_PARAMETER_CHANNEL_TRIGGER_GENERATION_THRESHOLD    	"PARAMETER_CHANNEL_TRIGGER_GENERATION_THRESHOLD "
#define TEXT_PARAMETER_CHANNEL_TRIGGER_GENERATION_PEAKING   	"PARAMETER_CHANNEL_TRIGGER_GENERATION_PEAKING"
#define TEXT_PARAMETER_CHANNEL_TRIGGER_GENERATION_GAP			"PARAMETER_CHANNEL_TRIGGER_GENERATION_GAP"
#define TEXT_PARAMETER_CHANNEL_TRIGGER_GENERATION_PULSE_LENGTH	"PARAMETER_CHANNEL_TRIGGER_GENERATION_G_VALUE"




#define TEXT_PARAMETER_ENERGY_P_VALUE   					"PARAMETER_ENERGY_P_VALUE"
#define TEXT_PARAMETER_ENERGY_GAP_VALUE   					"PARAMETER_ENERGY_GAP_VALUE"
#define TEXT_PARAMETER_ENERGY_TAU_VALUE   					"PARAMETER_ENERGY_TAU_VALUE"
#define TEXT_PARAMETER_ENERGY_AVERAGE_VALUE   				"PARAMETER_ENERGY_AVERAGE_VALUE"


#define TEXT_PARAMETER_MAW_TEST_BUFFER_ENABLE_FLAG 			"PARAMETER_MAW_TEST_BUFFER_ENABLE_FLAG"
#define TEXT_PARAMETER_MAW_TEST_BUFFER_SELECT_ENERGY_FLAG 	"PARAMETER_MAW_TEST_BUFFER_SELECT_ENERGY_FLAG"
#define TEXT_PARAMETER_MAW_TEST_BUFFER_LENGTH   			"PARAMETER_MAW_TEST_BUFFER_LENGTH"
#define TEXT_PARAMETER_MAW_TEST_BUFFER_DELAY   				"PARAMETER_MAW_TEST_BUFFER_DELAY"


#define TEXT_PARAMETER_ENERGY_HISTOGRAM_ENABLE_FLAG 		"PARAMETER_ENERGY_HISTOGRAM_ENABLE_FLAG"
#define TEXT_PARAMETER_ENERGY_HISTOGRAM_ONLY_FLAG 			"PARAMETER_ENERGY_HISTOGRAM_ONLY_FLAG"
#define TEXT_PARAMETER_ENERGY_HISTOGRAM_DIVIDER_VALUE 		"PARAMETER_ENERGY_HISTOGRAM_DIVIDER_VALUE"
#define TEXT_PARAMETER_ENERGY_HISTOGRAM_OFFSET_VALUE 		"PARAMETER_ENERGY_HISTOGRAM_OFFSET_VALUE"






#define TEXT_PARAMETER_GATE1_START_INDEX   					"PARAMETER_GATE1_START_INDEX"
#define TEXT_PARAMETER_GATE1_LENGTH   						"PARAMETER_GATE1_LENGTH"

#define TEXT_PARAMETER_GATE2_START_INDEX   					"PARAMETER_GATE2_START_INDEX"
#define TEXT_PARAMETER_GATE2_LENGTH   						"PARAMETER_GATE2_LENGTH"

#define TEXT_PARAMETER_GATE3_START_INDEX   					"PARAMETER_GATE3_START_INDEX"
#define TEXT_PARAMETER_GATE3_LENGTH   						"PARAMETER_GATE3_LENGTH"

#define TEXT_PARAMETER_BASELINE_AVERAGE_MODE   				"PARAMETER_BASELINE_AVERAGE_MODE"



#define TEXT_PARAMETER_PC_SHAPE_HISTOGRAM_DIVIDER_X_VAL			"PARAMETER_PC_SHAPE_HISTOGRAM_DIVIDER_X_VAL"
#define TEXT_PARAMETER_PC_SHAPE_HISTOGRAM_DIVIDER_Y_VAL			"PARAMETER_PC_SHAPE_HISTOGRAM_DIVIDER_Y_VAL"

#define TEXT_PARAMETER_PC_TOF_SHAPE_HISTOGRAM_DIVIDER_X_VAL		"PARAMETER_PC_TOF_SHAPE_HISTOGRAM_DIVIDER_X_VAL"
#define TEXT_PARAMETER_PC_TOF_SHAPE_HISTOGRAM_DIVIDER_Y_VAL		"PARAMETER_PC_TOF_SHAPE_HISTOGRAM_DIVIDER_Y_VAL"

#define TEXT_PARAMETER_PC_TOF_HISTOGRAM_DIVIDER_VAL				"PARAMETER_PC_TOF_HISTOGRAM_DIVIDER_VAL"

#define TEXT_PARAMETER_PC_CHARGE_HISTOGRAM_DIVIDER_VAL			"PARAMETER_PC_CHARGE_HISTOGRAM_DIVIDER_VAL"
#define TEXT_PARAMETER_PC_CHARGE_HISTOGRAM_CHARGE_SELECT_FLAG	"PARAMETER_PC_CHARGE_HISTOGRAM_CHARGE_SELECT_FLAG"


#define TEXT_PARAMETER_PC_HISTOGRAMMING_DISABLE_FOR_N_USEC_AFTER_RESTART_VAL	"PARAMETER_PC_HISTOGRAMMING_DISABLE_FOR_N_USEC_AFTER_RESTART_VAL"








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
	void calculatePileupWindowLength (void);


public:

	unsigned int uint_nof_events ;

	unsigned int uint_channel_external_trigger_enable[16] ;
	unsigned int uint_channel_polarity_invert[16] ;

	unsigned int uint_channel_range_2V[16] ;
	unsigned int uint_channel_50ohm_termination_disable[16] ;
	unsigned int uint_channel_adc_offset[16] ;
	
	unsigned int uint_lemo_out_CO_select ;
	unsigned int uint_lemo_out_TO_select ;
	unsigned int uint_lemo_out_UO_select ;



	unsigned int uint_pre_trigger_delay ;
	unsigned int uint_raw_sample_length ;
	unsigned int uint_trigger_gate_window_length;
	unsigned int uint_pileup_window_length;
	unsigned int uint_re_pileup_window_length;


	unsigned int uint_channel_trigger_enable[16] ;
	unsigned int uint_channel_trigger_cfd_enable[16] ;
	unsigned int uint_channel_trigger_threshold[16] ;
	unsigned int uint_channel_trigger_peaking[16] ;
	unsigned int uint_channel_trigger_gap[16] ;
	unsigned int uint_channel_trigger_pulse_length[16] ;

	unsigned int uint_energy_p_value ;
	unsigned int uint_energy_gap_value ;
	unsigned int uint_energy_tau_value ;
	unsigned int uint_energy_average_value ;


	unsigned int uint_maw_test_buffer_enable_flag ;
	unsigned int uint_maw_test_buffer_select_energy_flag ;
	unsigned int uint_maw_test_buffer_length ;
	unsigned int uint_maw_test_buffer_delay ;

	unsigned int uint_energy_histogram_enable_flag ;
	unsigned int uint_energy_histogram_only_flag ;
	unsigned int uint_energy_histogram_divider_value ;
	unsigned int uint_energy_histogram_offset_value ;





	unsigned int uint_gate1_start_index ;
	unsigned int uint_gate1_length ;
	unsigned int uint_gate2_start_index ;
	unsigned int uint_gate2_length ;
	unsigned int uint_gate3_start_index ;
	unsigned int uint_gate3_length ;

	unsigned int uint_baseline_average_mode ;


	unsigned int uint_PC_shapeHistogram_divider_x_val ;
	unsigned int uint_PC_shapeHistogram_divider_y_val ;

	unsigned int uint_PC_tof_shapeHistogram_divider_x_val ;
	unsigned int uint_PC_tof_shapeHistogram_divider_y_val ;

	unsigned int uint_PC_tofHistogram_divider_val ;

	unsigned int uint_PC_chargeHistogram_divider_val ;
	unsigned int uint_PC_chargeHistogram_charge_select_flag;

	unsigned int uint_PC_histogramming_disable_for_N_usec_after_restart;




public:
	sis3316_get_configuration_parameters(void);

	int read_parameter_file(char *path);

	~sis3316_get_configuration_parameters(void);
};






#endif /* GET_CONFIGURATION_PARAMETERS_H_ */
